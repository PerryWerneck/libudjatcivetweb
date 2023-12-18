/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 /**
  * @brief Implements simple oauth2 authenticator.
  *
  */

 // References:

 //	https://www.tutorialspoint.com/oauth2.0/oauth2.0_obtaining_an_access_token.htm
 // https://www.freebsd.org/doc/en/articles/pam/pam-essentials.html

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/keypair.h>
 #include <private/request.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/timestamp.h>

 #include <sys/socket.h>
 #include <netinet/in.h>

 #if defined(HAVE_PAM)
	#include <security/pam_appl.h>
 #endif // HAVE_PAM

 #ifdef _WIN32
	#include <windows.h>
 #else
	#include <sys/types.h>
	#include <pwd.h>
 #endif // _WIN32

 using namespace std;

 #ifdef HAVE_LIBSSL

 /// @brief Authentication token
 #pragma pack(1)
 struct AuthenticationToken {

	unsigned char type = 0;

#ifdef _WIN32

#else

	uid_t user = ((uid_t) -1);

#endif // _WIN32

	/// @brief The authenticated user id.
	unsigned int id = (unsigned int) -1;

	/// @brief Login expiration time.
	time_t expiration_time = 0;

	/// @brief Client IP Address
	uint8_t iptype = 0;

	union {
		struct in_addr v4;
		struct in6_addr v6;
	} ip;

 };
 #pragma pack()

#ifdef HAVE_PAM
	///
	/// @brief Run PAM conversation request.
	///
	/// Reference: <https://www.freebsd.org/doc/en/articles/pam/pam-sample-conv.html>
	///
	///
	static int pam_conversation(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *pwd) {

		debug(__FUNCTION__);

		if (num_msg <= 0 || num_msg > PAM_MAX_NUM_MSG)
			return (PAM_CONV_ERR);

		*resp = (struct pam_response *) calloc(num_msg, sizeof *resp);

		if(!*resp) {
			return (PAM_BUF_ERR);
		}

		for(int f = 0; f < num_msg;f++) {

			debug("style[",f,"=%",msg[f]->msg_style);

			switch(msg[f]->msg_style) {
			case PAM_PROMPT_ECHO_OFF:
				resp[f]->resp = strdup((const char *) pwd);
				break;

			case PAM_PROMPT_ECHO_ON:
				debug(msg[f]->msg);
				break;

			case PAM_ERROR_MSG:
				Logger::String{msg[f]->msg}.error("pam");
				break;

			case PAM_TEXT_INFO:
				Logger::String{msg[f]->msg}.info("pam");
				break;

			default:
				Logger::String{"Invalid or unexpected conversation message style: ",msg[f]->msg_style}.warning("pam");

			}


		}

		return (PAM_SUCCESS);

	}

	static int pam_exec(const char *username, const char *pwd, string &message){

		int state = EINVAL;

	    struct pam_conv local_conversation = {
	    	pam_conversation,
	    	(void *) pwd
		};

		int 			  retval		= 0;
		pam_handle_t 	* local_auth_handle = NULL; // this gets set by pam_start

#ifdef DEBUG
		retval = pam_start("common-auth", username, &local_conversation, &local_auth_handle);
#else
		retval = pam_start(
						Config::Value{"oauth2","service-name",Application::Name().c_str()}.c_str(),
						username,
						&local_conversation,
						&local_auth_handle
					);
#endif // DEBUG

		if(retval != PAM_SUCCESS) {
			throw runtime_error(Logger::String{pam_strerror(local_auth_handle, retval), " (rc=", retval, ")"});
		}

		retval = pam_authenticate(local_auth_handle, 0);

		switch(retval) {
		case PAM_SUCCESS:
			Logger::String{username," was authenticated"}.info("pam");
			state = 0;

			retval = pam_setcred(local_auth_handle,PAM_ESTABLISH_CRED);
			if(retval != PAM_SUCCESS) {
				Logger::String{"setcred: ", pam_strerror(local_auth_handle, retval), "(rc=",retval,")"}.error("pam");
			}
			break;

		case PAM_AUTH_ERR:
			Logger::String{username," was not authenticated"}.error("pam");
			state = EPERM;
			break;

		case PAM_USER_UNKNOWN:
			Logger::String{username, ": ", pam_strerror(local_auth_handle, retval), " (rc=",retval,")"}.error("pam");
			state = ENOENT;
			message = _("Invalid username");
			break;

		default:
			Logger::String{username, ": ", pam_strerror(local_auth_handle, retval), " (rc=",retval,")"}.error("pam");
			message = pam_strerror(local_auth_handle,retval);
			state = EINVAL;
		}

		retval = pam_end(local_auth_handle, retval);

		if (retval != PAM_SUCCESS) {
			Logger::String{"pam_end(",username,") returned ",retval}.error("pam");
		}

		return state;

	}

#endif // HAVE_PAM

 static void header_send(struct mg_connection *conn, AuthenticationToken &token) {

	// Reset timer.
	token.expiration_time = time(0) + Config::Value<time_t>("oauth","max-age",86400);

	mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
	mg_response_header_add(conn, "Expires", "0", -1);

	// Setup cookie
	string cookie{"session-data="};
	cookie += Udjat::HTTP::KeyPair::getInstance().encrypt(&token,sizeof(token));
	cookie += "; path=/; Expires=";
	cookie += HTTP::TimeStamp::to_string(token.expiration_time).c_str();

	debug("Cookie='",cookie,"'");
	mg_response_header_add(conn, "Set-Cookie", cookie.c_str(),-1);

	mg_response_header_send(conn);

 }

 static int login_page(struct mg_connection *conn, CivetWeb::Request &request, AuthenticationToken token, const char *login_message) {

#ifdef DEBUG
	String text{Application::DataFile{"./templates/login.html"}.load()};
#else
	String text{Application::DataFile{"templates/www/login.html"}.load()};
#endif // DEBUG

	// TODO: Expand common values

	// Last, expand request arguments.
	text.expand([request,login_message](const char *key, std::string &value) {

		if(!strcasecmp(key,"login_message")) {
			value = login_message;
		} else {
			value = request.getArgument(key);
		}
		return true;
	});

	mg_response_header_start(conn, 200);
	mg_response_header_add(conn, "Content-Type",std::to_string(MimeType::html),-1);
	mg_response_header_add(conn, "Content-Length", std::to_string(text.size()).c_str(), -1);
	header_send(conn,token);

	mg_write(conn, text.c_str(), text.size());

	return 200;

 }

 int oauthWebHandler(struct mg_connection *conn, void *) {


	/// @brief Validate login arguments.
	struct LoginValidator {
		String client_id;
		String redirect_uri;
		String response_type;

		LoginValidator(CivetWeb::Request &request)
			: client_id{request.getArgument("client_id")},
				redirect_uri{request.getArgument("redirect_uri")},
				response_type{request.getArgument("response_type")} {

			debug("client_id='",client_id,"'");
			debug("redirect_uri='",redirect_uri,"'");
			debug("response_type='",response_type,"'");
		}

		operator bool() {
			return !(client_id.empty() || redirect_uri.empty() || response_type.empty());
		}

	};

	CivetWeb::Request request{conn};

	try {

		debug("--------------------------------------------");

		debug("Authentication path: '",request.path(),"'");

		// Check cookie for authorization
		AuthenticationToken token;

		// Check for operation.
		switch(request.select("authorize","login","signin",nullptr)) {
		case 0:	// Authorize
			{
				if(!LoginValidator{request}) {
					mg_send_http_error(conn, 400, "Invalid request");
					return 400;
				}

				// TODO: Check if already authenticated.

				// TODO: Check client ID.

				// TODO: Create session.

				// Create token
				memset(&token,0,sizeof(token));

				// Redirect to login page.
				String target{"login?",request.query()};
				mg_response_header_start(conn, 303);
				mg_response_header_add(conn, "Location",target.c_str(),target.size());
 				mg_response_header_add(conn, "Content-Length", "0", -1);
				header_send(conn,token);

				return 303;
			}
			break;

		case 1:	// Login
			if(!LoginValidator{request}) {
				mg_send_http_error(conn, 400, "Invalid request");
				return 400;
			}
			memset(&token,0,sizeof(token));
			return login_page(conn,request,token,"");

		case 2:	// signin
			debug("Received signin response");

			if(!LoginValidator{request}) {
				mg_send_http_error(conn, 400, "Invalid request");
				return 400;
			}

			// Reset token.
			memset(&token,0,sizeof(token));

			// Authenticate
			if(request == MimeType::html) {

				string message{_("Access Denied")};

				String username{request["username"]};
				if(username.empty()) {
					return login_page(conn,request,token,_("Please, inform username"));
				}

				String password{request["password"]};
				if(password.empty()) {
					return login_page(conn,request,token,_("Please, inform password"));
				}

				//
				// Do the authentication
				//
#if defined(HAVE_PAM)

				// Use PAM
				// https://www.freebsd.org/doc/en/articles/pam/pam-essentials.html

				if(!pam_exec(username.c_str(),password.c_str(),message)) {

					// User was authenticated
					struct passwd pwd;
					struct passwd *ppwd = NULL;
					char buf[1024];

					if (getpwnam_r(username.c_str(), &pwd, buf, sizeof buf, &ppwd)) {

						message = strerror(errno);

					} else {

						// Got user info
						token.user = pwd.pw_uid;

						String target{request["redirect_uri"]};

						mg_response_header_start(conn, 303);
						mg_response_header_add(conn, "Location",target.c_str(),target.size());
						mg_response_header_add(conn, "Content-Length", "0", -1);
						header_send(conn,token);

						return 303;

					}
				}

#else
				// No authentication module
				message = strerror(ENOTSUP);
#endif

				// Authentication failed, send login again
				return login_page(conn,request,token,message.c_str());
			}
			break;

		default:
			mg_send_http_error(conn, 400, "Invalid request: '%s'", request.path());
			return 400;
		}

	} catch(const exception &e) {

		if(request == MimeType::html) {
			return login_page(conn,request,AuthenticationToken{},e.what());
		}
		mg_send_http_error(conn, 500, "%s", e.what());
		return 500;

	} catch(...) {

		if(request == MimeType::html) {
			return login_page(conn,request,AuthenticationToken{},_("Unexpected error"));
		}
		mg_send_http_error(conn, 500, "%s", _("Unexpected error"));
		return 500;

	}

	mg_send_http_error(conn, 404, _("Not available"));
	return 404;

 }

 #endif // HAVE_LIBSSL

