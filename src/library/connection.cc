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
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/template.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/application.h>
 #include <sstream>
 #include <udjat/authentication.h>
 #include <udjat/tools/http/authentication.h>

 using namespace std;

 namespace Udjat {

	HTTP::Connection::Connection() {
	}

	HTTP::Connection::~Connection() {
	}

	bool HTTP::Connection::get_property(const char *key, Udjat::Value &value) const {
		return false;
	}

	bool HTTP::Connection::process_template(HTTP::Status &status, const char *key, std::ostream &stream) const noexcept {

		try {

			if(!strcasecmp(key,"page-summary")) {

				// TODO: Get root agent, format summary, update status.last_modified and status.expires

				return true;
			}

	
			Udjat::Value value;
			if(get_property(key,value)) {
				stream << value.serialize(status.mimetype);
				return true;
			}

			if(!(strcasecmp(key,"page-title") && strcasecmp(key,"title"))) {
				Config::Value<String> title{
					"theme",
					"title",
#ifdef _WIN32
					Application::Description().c_str()
#else
					STRINGIZE_VALUE_OF(PRODUCT_NAME)
#endif
				};
				title.expand(true,true);
				stream << title.c_str();
				return true;
			}

			if(!strcasecmp(key,"user-link")) {

				if(auth.available() && mimetype(MimeType::html) == MimeType::html) {

					if(auth >= Authentication::Guest) {
						stream
							<< "<a id=\"user-info\" href=\""
							<< Config::Value<string>("authentication","userinfo","/user").c_str()
							<< "\">"
							<< auth.name()
							<< "</a>";
					} else {
						stream 
							<< "<a id=\"login-button\" href=\""
							<< Config::Value<string>("authentication","begin","/oauth2/authenticate").c_str()
							<< "\">"
							<< _( "Log in" )
							<< "</a>";
					}

				}

				return true;

			}

			if(!strcasecmp(key,"avatar")) {

				if(auth.available()) {
					stream << auth.avatar();
				}
				return true;

			}

			if(!strcasecmp(key,"navbar")) {

				// TODO: Implement navbar

				return true;
			}

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error();
			status.assign(e);

		}

		return false;

	}

	HTTP::StatusCode HTTP::Connection::send(const HTTP::StatusCode code, const Request &request) noexcept {
		return send(
			HTTP::Status{
				code,
				request.mimetype()
			},
			request.apicall()
		);
	}

	HTTP::StatusCode HTTP::Connection::send(const HTTP::Response &response) noexcept {
		
		debug("Sending response");

		try {
	
			stringstream out;
			response.serialize(out);
			return send(response.status(),out.str().c_str());

		} catch(const std::exception &e) {

			return send(HTTP::Status{e},true);

		}

		return response.status_code();
	}

	HTTP::StatusCode HTTP::Connection::send(const HTTP::Status &status, bool apicall) noexcept {

		stringstream out;

		if(apicall) {

			// Api call, just serialize the response.
			status.serialize(out);

		} else if(status.code != HTTP::NoContent && status.code != HTTP::NotModified) {

			// Load template
			Template tmplt{
				(status.success() ? "success" : "failed"),
				status.mimetype
			};

			if(!tmplt) {

				// Cant find template, just serialize.
				status.serialize(out);

			} else {

				// Found template, use it.
				tmplt.apply(out,status);

			}

		}

		return send(status,out.str().c_str());

	}

	HTTP::StatusCode HTTP::Connection::logger(HTTP::StatusCode code, const char *message, Logger::Level level) const {

		Logger::String{
			code," ",message
		}.write(level,"httpd");

		return code;

	}


 }

