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
 #include <udjat/tools/http/response.h>
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

		// Build status.
		HTTP::Status status{HTTP::Ok,request.mimetype()};

		// Scan for interface.
		Interface *intf = nullptr;
		
		if(!request.root()) {

			// It's not the root path, search for interface
			intf = Interface::find(request);

			if(!intf) {
				status = HTTP::NotFound;
				error(
					status.code,
					String{"Cant find interface for '",request.path(),"'"}.c_str()
				);
				return send(status,request.apicall());
			}

		}

		try {

			stringstream response;

			Template tmplt{
				Config::Value<string>{"theme","index","main"},
				status.mimetype
			};

			if(tmplt) {

				debug("Got index template");

				tmplt.apply(response,[this,&status,&request,intf](const char *key, std::ostream &stream){

					if(process_template(status,key,stream)) {
						return true;
					}

					if(!strcasecmp(key,"page-contents")) {
						if(intf) {
							intf->process(request,status,stream);
						}
						return true;
					}

					return false;

				});

			} else {
				status = HTTP::NotFound;
			}

			return send(status,response.str().c_str());

		} catch(const std::exception &e) {

			status.assign(e);

		} catch(...) {

			status.assign(HTTP::SystemError);
			Logger::String{"Unexpected error processing request"}.error();

		}

		return send(status,request.apicall());

	}

 }
