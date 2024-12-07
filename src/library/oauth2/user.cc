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
  * @brief Implements common OAuth2 user methods.
  */

 #include <config.h>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/http/keypair.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/intl.h>
 #include <cstring>

 using namespace std;
 using namespace Udjat;

 namespace Udjat {

	OAuth::User::User() {
		data.clear();
		data.expiration_time = time(0) + Config::Value<time_t>("oauth2","expiration-time",86400);
		data.type = 0x20;
		data.scope = 0x0f;	// Default scope.
#ifndef _WIN32
		data.uid = (unsigned int) -1;
#endif // _WIN32
	}

	OAuth::User::User(HTTP::Request &request) : User() {
		set(request);
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
#ifndef _WIN32
		token.uid = data.uid;
#endif // _WIN32

		memset(token.username,0,TOKEN_USERNAME_LEN);
		strncpy(token.username,data.username,TOKEN_USERNAME_LEN);

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

#ifdef _WIN32
			if(data.type == 0x14 && memcmp(&data.ip.v4,&this->data.ip.v4,sizeof(data.ip.v4))) {
				Logger::String{"Rejecting user token by IPV4 mismatch"}.error("oauth2");
				return false;
			}
#else
			if(data.type == 0x14 && data.ip.v4 != this->data.ip.v4) {
				Logger::String{"Rejecting user token by IPV4 mismatch"}.error("oauth2");
				return false;
			}
#endif // _WIN32

			if(data.type == 0x16 && memcmp(&data.ip.v6,&this->data.ip.v6,sizeof(this->data.ip.v6))) {
				Logger::String{"Rejecting user token by IPV6 mismatch"}.error("oauth2");
				return false;
			}

			if(data.expiration_time < time(0)) {
				Logger::String{"Rejecting expired user token"}.error("oauth2");
				return false;
			}

			this->data = data;
#ifdef _WIN32
			Logger::String{"Accepting valid user token"}.trace("oauth2");
#else
			Logger::String{"Accepting valid user token for uid ",this->data.uid}.trace("oauth2");
#endif // _WIN32

			return true;
		}

		return false;
	}

 }
