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
  * @brief Implements OAuth2 user.
  */

 // References:
 //
 // https://www.freebsd.org/doc/en/articles/pam/pam-essentials.html

 #include <config.h>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/http/keypair.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>
 #include <cstring>

 #include <sys/socket.h>
 #include <netinet/in.h>

 #if defined(HAVE_PAM)
	#include <security/pam_appl.h>
 #endif // HAVE_PAM

 using namespace std;

 namespace Udjat {

	OAuth::User::User() {
		memset(&data,0,sizeof(data));
		data.expiration_time = time(0) + Config::Value<time_t>("oauth2","expiration-time",86400);
		data.type = 0x20;
		data.uid = (unsigned int) -1;
	}

	OAuth::User::User(HTTP::Request &request) : User() {

		// TODO: Validate client id and secret.
//		string id{request["client-id"]};
//		string secret{request["client-secret"]};

		set(request);

	}

	void OAuth::User::set(HTTP::Request &request) {

		// Setup token from request.
		String req_addr{request.address()};
		sockaddr_storage addr;

		if(inet_pton(AF_INET,req_addr.c_str(),&((struct sockaddr_in *) &addr)->sin_addr) == 1) {
			data.type |= 0x04;
			data.ip.v4 = ((struct sockaddr_in *) &addr)->sin_addr.s_addr;
		} else if(inet_pton(AF_INET6,req_addr.c_str(),&((struct sockaddr_in6 *) &addr)->sin6_addr) == 1) {
			data.type |= 0x06;
			data.ip.v6 = ((struct sockaddr_in6 *) &addr)->sin6_addr;
		} else {
			memset(&data.ip,0,sizeof(data.ip));
			Logger::String{"Cant identify address '",req_addr.c_str(),"'"}.warning("oauth");
		}

	}

	OAuth::User::~User() {
	}

	String OAuth::User::encrypt() {
		return HTTP::KeyPair::getInstance().encrypt(&data,sizeof(data));
	}

	String OAuth::User::code() {
		return HTTP::KeyPair::getInstance().encrypt(&data,sizeof(data));
	}

	bool OAuth::User::code(const char *str) {
		return decrypt(str);
	}

	void OAuth::User::get(OAuth::Context &context) {
		context.token = encrypt();
		context.expiration_time = data.expiration_time;
	}

	bool OAuth::User::decrypt(const char *str) {

		User::Token data;

		if(HTTP::KeyPair::getInstance().decrypt(str,&data,sizeof(data))) {

			if(data.type != this->data.type) {
				Logger::String{"Rejecting user token by type mismatch"}.error("oauth2");
				return false;
			}

			if(data.type == 0x14 && data.ip.v4 != this->data.ip.v4) {
				Logger::String{"Rejecting user token by IPV4 mismatch"}.error("oauth2");
				return false;
			}

			if(data.type == 0x16 && memcmp(&data.ip.v6,&this->data.ip.v6,sizeof(this->data.ip.v6))) {
				Logger::String{"Rejecting user token by IPV6 mismatch"}.error("oauth2");
				return false;
			}

			if(data.expiration_time < time(0)) {
				Logger::String{"Rejecting expired user token"}.error("oauth2");
				return false;
			}

			Logger::String{"Accepting valid user token"}.trace("oauth2");
			this->data = data;
			return true;
		}

		return false;
	}


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

	static int pam_exec(const char *username, const char *pwd, std::string &message){

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

	bool OAuth::User::authenticate(HTTP::Request &request, std::string &message) {

		message = _("Access Denied");
		data.uid = (unsigned int) -1;

		String username{request["username"]};
		if(username.empty()) {
			message = _("Please, inform username");
			return false;
		}

		String password{request["password"]};
		if(password.empty()) {
			message = _("Please, inform password");
			return false;
		}

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
				return false;
			}

			data.uid = pwd.pw_uid;
			message = String{"Username '",username.c_str(),"' (",data.uid,")"};

			return true;

		}

#else
		// No authentication module
		message = strerror(ENOTSUP);
#endif

		return false;
	}

 }
