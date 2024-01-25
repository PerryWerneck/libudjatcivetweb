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
 #include <udjat/tools/application.h>
 #include <udjat/tools/intl.h>
 #include <cstring>

 #include <sys/socket.h>
 #include <netinet/in.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifdef _WIN32
	#include <windows.h>
 #else
	#include <pwd.h>
 #endif // _WIN32

 #if defined(HAVE_PAM)
	#include <security/pam_appl.h>
 #endif // HAVE_PAM

 using namespace std;
 using namespace Udjat;

 namespace Udjat {

	static const char * scope_names[] = {
		"username",		// 0x0001
		"userinfo", 	// 0x0002
	};

	OAuth::User::User() {
		memset(&data,0,sizeof(data));
		data.expiration_time = time(0) + Config::Value<time_t>("oauth2","expiration-time",86400);
		data.type = 0x20;
		data.scope = 0x0f;	// Default scope.
		data.uid = (unsigned int) -1;
	}

	OAuth::User::User(HTTP::Request &request) : User() {
		set(request);
	}

	void OAuth::User::set(HTTP::Request &request) {

		// Setup token from request.
		String req_addr{request.address()};
		sockaddr_storage addr;

		debug("Building user info from ",req_addr.c_str()," request");

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

		// Check for authorization
		{
			String token = request.Abstract::Object::getProperty("Authorization","");
			if(!token.empty() && token.has_prefix("Bearer ",true) && decrypt(token.c_str()+7)) {
				debug("Got user fron 'Authorization' header");
				return;
			}
		}

		// Check for cookie
		{
			String token = request.cookie("oauth2-session");
			if(!token.empty() && decrypt(token.c_str())) {
				debug("Got user from 'oauth2-session' cookie");
				return;
			}
		}

	}

	OAuth::User::~User() {
	}

	String OAuth::User::encrypt() {
		return HTTP::KeyPair::getInstance().encrypt(&data,sizeof(data));
	}

	String OAuth::User::encrypt(Udjat::HTTP::Request::Token &token) {

		if(!*this) {
			throw runtime_error("User is not authenticated in this context");
		}

		token.expiration_time = data.expiration_time;
		token.scope = data.scope;
		token.uid = data.uid;

		memset(token.username,0,sizeof(token.username));
		strncpy(token.username,data.username,sizeof(token.username)-1);

		memset(&token.ip,0,sizeof(token.ip));
		if((data.type & 0x0f) == 0x04) {
			// IPV4
			token.type |= 0x04;
			token.ip.v4 = data.ip.v4;
		} else if( (data.type & 0x0f) == 0x06) {
			// IPV6
			token.type |= 0x06;
			token.ip.v6 = data.ip.v6;
		}

		return HTTP::KeyPair::getInstance().encrypt(&token,sizeof(token));
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

		while(*str && isspace(*str)) {
			str++;
		}

		if(!*str) {
			Logger::String{"Rejecting empty user token"}.error("oauth2");
			return false;
		}

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

			this->data = data;
			Logger::String{"Accepting valid user token for uid ",this->data.uid}.trace("oauth2");

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
		retval = pam_start(
						Config::Value<std::string>{"oauth2","service-name","common-auth"}.c_str(),
						username,
						&local_conversation,
						&local_auth_handle
					);
#else
		retval = pam_start(
						Config::Value<std::string>{"oauth2","service-name",Application::Name().c_str()}.c_str(),
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

		memset(data.username,0,sizeof(data.username));
		strncpy(data.username,username.c_str(),sizeof(data.username)-1);

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

	bool OAuth::User::get(Udjat::Value &value) {

		if(!*this) {
			return false;
		}

		return get(data.uid,data.scope,value);

	}

	bool OAuth::User::get(uint64_t uid, uint16_t scope, Udjat::Value &value) {

#ifdef _WIN32

		// TODO

#else

		// Get Linux passwd
		long buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
		if(buflen < 1) {
			buflen = 4096;
		}

		char buffer[buflen];
		struct passwd pwbuf;
		struct passwd *pw = NULL;

		if(getpwuid_r((uid_t) uid, &pwbuf, buffer, buflen, &pw) == 0 && pw != NULL) {

			debug("Got user '",(const char *) pw->pw_name,"'");

			if(scope & 0x00000001) {
				debug("addding '",scope_names[0],"'");
				value[scope_names[0]] = (const char *) pw->pw_name;
			}

			if(scope & 0x00000002) {
				debug("addding '",scope_names[1],"'");
				value[scope_names[1]] = (const char *) pw->pw_gecos;
			}

		} else {

			Logger::String{"Unable to get info for UID ",(uid_t) uid}.error("oauth2");

		}


#endif // _WIN32

		return true;

	}


 }
