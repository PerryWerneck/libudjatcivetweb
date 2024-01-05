/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>

 #include <udjat/civetweb.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/http/keypair.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>

 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	HTTP::Request::Request(const char *path, HTTP::Method method) : Udjat::Request{path, method} {

		if(reqpath && *reqpath) {

			if(strncasecmp(reqpath,"/api/",5) && Config::Value<bool>("httpd","allow-legacy-path",true)) {

				// Parse as legacy request
				const char *next = strchr(reqpath+1,'/');
				if(!next) {
					throw system_error(
								ENOENT,system_category(),
								Logger::Message{
									_("Request path should be /api/{}/[REQUEST]"),
									Config::Value<string>{"http","apiver","[APIVER]"}.c_str()
								}
							);
				}

				reqpath = next;

				// TODO: Check if the first path element is the same as mimetype

			} else {

				// Is an standard API request, extract version.

				reqpath += 5;

				while(*reqpath && *reqpath != '/') {
					if(isdigit(*reqpath)) {
						apiver *= 10;
						apiver += ('0' - *reqpath);
					}
					reqpath++;
				}

			}

		}

	}

	const char * HTTP::Request::c_str() const noexcept {
		if(reqpath && *reqpath) {
			return reqpath;
		}
		return Udjat::Request::path();
	}

	static inline bool decrypt(const HTTP::Request &request, HTTP::Request::Token &token) {

		// Check for authorization header.
		{
			String b64 = request.getProperty("Authorization");
			if(!b64.empty() && b64.has_prefix("Bearer ",true) && HTTP::KeyPair::getInstance().decrypt(b64.c_str()+7,&token,sizeof(token))) {
				debug("Got authentication from header");
				return true;
			}
		}

		// Check for cookie.
		{
			String b64 = request.cookie((Application::Name() + "-session").c_str());
			if(!b64.empty() && HTTP::KeyPair::getInstance().decrypt(b64.c_str(),&token,sizeof(token))) {
				debug("Got authentication from cookie");
				return true;
			}
		}

		return false;
	}

	bool HTTP::Request::authenticated() const noexcept {
		Token token;
		return get(token);
	}

	bool HTTP::Request::get(Request::Token &token) const noexcept {

		try {

			if(!decrypt(*this,token)) {
				return false;
			}

			if(token.expiration_time < time(0)) {
				Logger::String{"Rejecting expired authentication token"}.error("civetweb");
				return false;
			}

			String req_addr{address()};
			sockaddr_storage addr;

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

			return true;

		} catch(const std::exception &e) {

			Logger::String{"Error checking authentication: ",e.what()}.error("civetweb");

		} catch(...) {

			Logger::String{"Unexpected error checking authentication"}.error("civetweb");

		}

		return false;

	}

	bool HTTP::Request::cached(const Udjat::TimeStamp &timestamp) const {

		HTTP::TimeStamp	reqtime{getProperty("If-Modified-Since","").c_str()};

		if(reqtime && ((time_t) reqtime) >= ((time_t) timestamp)) {
			return true;
		}

		return Udjat::Request::cached(timestamp);
	}

 }
