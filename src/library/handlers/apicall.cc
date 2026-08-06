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
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/interface.h>

 using namespace std;

 namespace Udjat {

	/// @brief Run method, handle exceptions.
	HTTP::StatusCode HTTP::Connection::apicall(HTTP::Request &request, HTTP::Response &response) noexcept {

		debug("Running API Call for '",request.path(),"'");

		try {

			auto intf = Interface::find(request);
			if(!intf) {
				response.assign(
					HTTP::NotFound,
					String{"Cant find interface for '",request.path(),"'"}.c_str()
				);
				return send(response);
			}

			Schema::Output out;
			if(!intf->schema(out)) {
				response.assign(
					HTTP::SystemError,
					String{"Interface '",intf->name(),"' doesnt provide an output schema"}.c_str()
				);
				return send(response);
			}
 
			if(response.mimetype() == MimeType::svg) {

				// TODO: It's an svg, search for an icon.
				for(const auto &item : out) {

					if(item.type() == Schema::Icon) {

						debug("Searching for icon at '",item.name(),"'");

						Variant prop;
						if(intf->get_property(request.path(),item.name(),prop)) {
							return icon(prop.to_string().c_str());
						}
#ifdef DEBUG
						else {
							Logger::String{"Cant find property '",item.name(),"'"}.info();
						}
#endif

					}

				}

				HTTP::Status status{
					HTTP::NotFound,
					MimeType::text
				};

				return send(status,false);

			} else if(request.root()) {

				if( (out.options & out.Enumerable) != 0) {

					// Run enumeration.
					response.Variant::clear(Variant::Array);

					intf->for_each([&response,&out](const Udjat::Variant &value){
						auto &item = response.append(Variant::Object);
						for(const auto &s : out) {
							item[s.name()] = value[s.name()];
						}
						return false;
					});

					return send(response);
				}

				Schema::Input in;
				intf->schema(in);

				if( (in.options & in.AllowRoot) == 0) {
					response.assign(
						HTTP::BadRequest,
						_("An object path is required")
					);
					return send(response);
				}

			}

			if(!intf->process(request,response)) {
				response.assign(
					HTTP::NotFound,
					_("Request rejected by backend")
				);
			}

		} catch(const std::exception &e) {

			debug("Exception: ",e.what());
			response.assign(e);

		} catch(...) {

			response.assign(
				HTTP::SystemError,
				"Unexpected exception"
			);

		}

		return send(response);

	}

 }
