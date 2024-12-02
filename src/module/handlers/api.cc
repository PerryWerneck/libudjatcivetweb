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
 #include <udjat/tools/civetweb/service.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>

 #include <civetweb.h>

 #include <private/module.h>
 #include <private/request.h>

 using namespace std;

 namespace Udjat {

	int CivetWeb::Service::api_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept {

		HTTP::Response response{MimeTypeFactory(conn)};
		
		try {

			unsigned int apiver = srvc->apiver;
			const mg_request_info *info = mg_get_request_info(conn); 
			const char *path = info->local_uri;

			if(path && *path && !strncasecmp(path,"/api/",5)) {
				path += 4;
				if(isdigit(path[1])) {
					path++;
					apiver = 0;
					while(*path && *path != '/') {
						if(isdigit(*path)) {
							apiver *= 10;
							apiver += (*path - '0');
						}
						path++;
					}
				}
			}

			CivetWeb::Request request{conn,path,apiver};

			int rc = srvc->HTTP::Server::call(info->request_method,request,response);
			if(rc) {
				response.failed(_("Unexpected error"));
			}

		} catch(const exception &e) {
			HTTP::Response response{MimeTypeFactory(conn)};
			response.failed(e);
		} catch(...) {
			HTTP::Response response{MimeTypeFactory(conn)};
			response.failed(_("Unexpected error"));
		}

		return send(conn,response);
	
	}


 }

