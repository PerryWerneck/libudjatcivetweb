/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements Win32 OAuth2 user.
  */

 // References:
 //
 // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-logonusera

 #include <config.h>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/http/keypair.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/intl.h>
 #include <cstring>

 #include <windows.h>

 using namespace std;
 using namespace Udjat;

 namespace Udjat {

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
				debug("Got user from 'Authorization' header");
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

		// No authentication module
		message = strerror(ENOTSUP);

		return false;
	}

	bool OAuth::User::get(uint64_t uid, uint16_t scope, Udjat::Value &value) {

		// TODO


		return false;

	}


 }
