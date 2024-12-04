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

 #include <config.h>

 #include <udjat/defs.h>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/interface.h>

 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 
 #include <civetweb.h>

 
 using namespace std;

 namespace Udjat {

	int HTTP::Server::call(const char *name, HTTP::Request &request, HTTP::Response &response) {

		try {

			//
			// Check for interfaces
			//
			for(auto &interface : interfaces) {

				if(strcasecmp(name,interface.c_str())) {
					debug("Ignoring '",interface.c_str(),"'");
					continue;
				}

				debug("Calling interface '",name,"'");
				interface.call(request,response);

				return 0;

			}

			response.failed(ENOENT);
			return 0;

		} catch(const exception &e) {
			response.failed(e);
		} catch(...) {
			response.failed(_("Unexpected error"));
		}

		return 0;

	}

 }

