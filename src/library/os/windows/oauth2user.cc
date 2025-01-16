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
 #include <udjat/win32/exception.h>
 #include <udjat/tools/string.h>

 #include <windows.h>

 using namespace std;
 using namespace Udjat;

 namespace Udjat {

	bool HTTP::Request::get(Request::Token &token) const noexcept {

		try {

			if(!decrypt(token)) {
				return false;
			}

			if(token.expiration_time < time(0)) {
				Logger::String{"Rejecting expired authentication token"}.error("civetweb");
				return false;
			}

			Udjat::String req_addr{address()};
			sockaddr_storage addr;

			// TODO: Decode win32 adresses.

			/*
			if(inet_pton(AF_INET,req_addr.c_str(),&((struct sockaddr_in *) &addr)->sin_addr) == 1) {

				if( (token.type & 0x0F) != 4) {
					Logger::String{"Rejecting authentication token by network type"}.error("civetweb");
					return false;
				}

				if(token.ip.v4 != ((struct sockaddr_in *) &addr)->sin_addr.s_addr) {
					Logger::String{"Rejecting authentication token by IPV4 network address"}.error("civetweb");
					return false;
				}

			} else if(inet_pton(AF_INET6,req_addr.c_str(),&((struct sockaddr_in6 *) &addr)->sin6_addr) == 1) {

				if( (token.type & 0x0F) != 6) {
					Logger::String{"Rejecting authentication token by network type"}.error("civetweb");
					return false;
				}

				if(memcmp(&token.ip.v6,&((struct sockaddr_in6 *) &addr)->sin6_addr,sizeof(token.ip.v6))) {
					Logger::String{"Rejecting authentication token by IPV6 network address"}.error("civetweb");
					return false;
				}

			} else {

				if( (token.type & 0x0F) != 0) {
					Logger::String{"Cant identify address '",req_addr.c_str(),"', rejecting authentication token"}.error("civetweb");
					return false;
				};

			}
			*/

			return true;

		} catch(const std::exception &e) {

			Logger::String{"Error checking authentication: ",e.what()}.error("civetweb");

		} catch(...) {

			Logger::String{"Unexpected error checking authentication"}.error("civetweb");

		}

		return false;

	}

	void OAuth::User::set(HTTP::Request &request) {

		// Setup token from request.
		String req_addr{request.address()};
		sockaddr_storage addr;

		debug("Building user info from ",req_addr.c_str()," request");

		// TODO: Decode win32 addresses

		/*
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
		*/

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

		Config::Value<std::string> domain{"oauth2","domain",""};

		DWORD dwLogonType;

		{
			static DWORD types[] = {
				LOGON32_LOGON_BATCH,
				LOGON32_LOGON_INTERACTIVE,
				LOGON32_LOGON_NETWORK,
				LOGON32_LOGON_SERVICE
			};

			int lt = Config::Value<string>{"oauth2","logon-type","network"}.select("batch","interactive","network","service",nullptr);
			if(lt < 0 || lt > N_ELEMENTS(types)) {
				throw runtime_error("Invalid oauth2 logon type, check configuration");
			}

			dwLogonType = types[lt];
		}

		HANDLE hToken = 0;

		if(LogonUser(
			username.c_str(),
			(domain.empty() ? NULL : TEXT(domain.c_str())),
			password.c_str(),
			dwLogonType,
			LOGON32_PROVIDER_DEFAULT,
			&hToken
		)) {

			CloseHandle(hToken);

			return true;
		}

		Logger::String{Win32::Exception::format("LogonUser has failed")}.error("oauth2");

		return false;
	}

	bool OAuth::User::get(uint64_t uid, uint16_t scope, Udjat::Value &value) {

		// TODO


		return false;

	}

	bool OAuth::User::get(Udjat::Value &value) {

		if(!*this) {
			return false;
		}

		// TODO: Get win32 user info

		return false;
	}

 }
