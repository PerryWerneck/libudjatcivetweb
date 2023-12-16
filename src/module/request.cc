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
  * @brief Implement CivetWeb request.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/request.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <ctype.h>

 #include <civetweb.h>

 using namespace std;

 namespace Udjat {

	namespace CivetWeb {

		Request::Request(struct mg_connection *c)
			: HTTP::Request{mg_get_request_info(c)->local_uri,mg_get_request_info(c)->request_method}, conn{c}, info{mg_get_request_info(c)} {

			debug("Request path set to '",path(),"'");

			// https://github.com/civetweb/civetweb/blob/master/examples/embedded_c/embedded_c.c


		}

  		const char * Request::c_str() const noexcept {
			return info->local_uri;
  		}

		const char * Request::query(const char *) const {
			return info->query_string;
		}

 		String Request::getProperty(const char *name, const char *def) const {

			for(int header = 0; header < info->num_headers; header++) {
				if(!strcasecmp(info->http_headers[header].name,name)) {
					return info->http_headers[header].value;
				}
			}

			return Udjat::Request::getProperty(name,def);
 		}

		String Request::getArgument(const char *name, const char *def) const {

			if( ((HTTP::Method) *this) == HTTP::Get ) {
				return HTTP::Request::getArgument(name,def);
			}

			throw runtime_error("Incomplete");

		}

	}

 }
