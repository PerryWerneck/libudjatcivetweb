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

		if(reqpath && *reqpath && !strncasecmp(reqpath,"/api/",5)) {

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

	const char * HTTP::Request::c_str() const noexcept {
		if(reqpath && *reqpath) {
			return reqpath;
		}
		return Udjat::Request::path();
	}

	const char * HTTP::Request::header(const char *) const noexcept {
		return "";
	}

	bool HTTP::Request::getProperty(const char *key, std::string &value) const {

		if(!strcasecmp(key,"client-address")) {
			value = address();
			return true;
		}

		return Udjat::Request::getProperty(key,value);
	}

	bool HTTP::Request::for_each(const std::function<bool(const char *name, const char *value)> &call) const {

		if(call("client-address",address().c_str())) {
			return true;
		}

		return Udjat::Request::for_each(call);
	}

	bool HTTP::Request::decrypt(HTTP::Request::Token &token) const {

		memset(&token,0,sizeof(token));

		// Check for authorization header.
		{
			String b64 = header("Authorization");
			if(!b64.empty() && b64.has_prefix("Bearer ",true) && HTTP::KeyPair::getInstance().decrypt(b64.c_str()+7,&token,sizeof(token))) {
				debug("Got authentication from header");
				return true;
			}
		}

		// Check for cookie.
		{
			String b64 = cookie((Application::Name() + "-session").c_str());
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

	bool HTTP::Request::cached(const Udjat::TimeStamp &timestamp) const {

		HTTP::TimeStamp	reqtime{header("If-Modified-Since")};

		if(reqtime && ((time_t) reqtime) >= ((time_t) timestamp)) {
			return true;
		}

		return Udjat::Request::cached(timestamp);
	}

 }
