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

 #include <config.h>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/http/keypair.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>

 #include <sys/socket.h>
 #include <netinet/in.h>

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

	String OAuth::User::encript() {
		return HTTP::KeyPair::getInstance().encrypt(&data,sizeof(data));
	}

	void OAuth::User::get(OAuth::Token &token) {
		token.cookie = encript();
		token.expiration_time = data.expiration_time;
	}

	bool OAuth::User::decript(const char *str) {

		User::Token data;

		if(HTTP::KeyPair::getInstance().decrypt(str,&data,sizeof(data))) {

			if(data.type != this->data.type) {
				Logger::String{"Rejecting user token by type mismatch"}.trace("oauth2");
				return false;
			}

			if(data.type == 0x14 && data.ip.v4 != this->data.ip.v4) {
				Logger::String{"Rejecting user token by IPV4 mismatch"}.trace("oauth2");
				return false;
			}

			if(data.type == 0x16 && memcmp(&data.ip.v6,&this->data.ip.v6,sizeof(this->data.ip.v6))) {
				Logger::String{"Rejecting user token by IPV6 mismatch"}.trace("oauth2");
				return false;
			}

			Logger::String{"Accepting valid user token"}.trace("oauth2");
			this->data = data;
			return true;
		}

		return false;
	}


 }
