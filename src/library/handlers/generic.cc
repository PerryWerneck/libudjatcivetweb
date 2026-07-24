/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/template.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/file/path.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/http/status.h>
 #include <stdexcept>
 #include <sstream>

 using namespace std;

 namespace Udjat {

	/// @brief Run method, handle exceptions.
	HTTP::StatusCode HTTP::Connection::handle(HTTP::Request &request) noexcept {

		debug("\n\n------------ Request::path = '",request.path(),"'");

		const char *path = request.path();
		if(path[0] == '/' && !path[1] && !request.apicall()) {

			// The path is '/' and it's not an apicall, send index.
			HTTP::Status status{request.mimetype()};

			try {

				// empty request, send index
				stringstream response;

				Template tmplt{
					Config::Value<string>{"theme","index","main"},
					status.mimetype
				};

				if(tmplt) {

					debug("Got index template");

					status.last_modified = tmplt.last_modified();
					tmplt.apply(response,[this,&status](const char *key, std::ostream &stream){
						return process_template(status,key,stream);
					});

				} else {

					Logger::String{"Cant find template for 'main' on current theme"}.error();
					status.clear();
					status.assign(HTTP::NotFound,request);

				}

				if(request.cached(TimeStamp{status.last_modified})) {
					status.assign(HTTP::NotModified);
					return send(status,"");
				}

				// Send template
				status.assign(HTTP::Ok);
				return send(status,response.str().c_str());

			} catch(const std::exception &e) {

				status.assign(e);

			} catch(...) {

				status.assign(HTTP::SystemError);

			}

			return send(status,request.apicall());

		}

		// Search for interfaces


		// Send 404 response
		return send(HTTP::NotFound,request);

	}

 }
