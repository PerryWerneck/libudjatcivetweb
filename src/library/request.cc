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
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/timestamp.h>
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
					throw runtime_error(_("Request path should be /api/[APIVER]/[REQUEST]"));
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

	bool HTTP::Request::cached(const Udjat::TimeStamp &timestamp) const {

		HTTP::TimeStamp	reqtime{getProperty("If-Modified-Since","").c_str()};

		if(reqtime && ((time_t) reqtime) >= ((time_t) timestamp)) {
			return true;
		}

		return Udjat::Request::cached(timestamp);
	}

 }
