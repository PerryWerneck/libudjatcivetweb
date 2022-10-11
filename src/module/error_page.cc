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

 #include "private.h"
 #include <udjat/defs.h>
 #include <udjat/module.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/expander.h>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/http/handler.h>
 #include <udjat/tools/logger.h>
 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 int http_error( struct mg_connection *conn, int status, const char *message ) {

	Udjat::MimeType mimetype = (Udjat::MimeType) 0;

	const struct mg_request_info *request_info = mg_get_request_info(conn);

	/*
	//
	// Search request headers for mime-type.
	//
	for(int ix=0;ix<request_info->num_headers;ix++) {
		debug(request_info->http_headers[ix].name,"=",request_info->http_headers[ix].value);

		if(!strcasecmp(request_info->http_headers[ix].name,"Content-Type")) {
			mimetype = MimeTypeFactory(request_info->http_headers[ix].value,false);
		}


	}
	*/

	if(!mimetype && request_info->local_uri_raw && *request_info->local_uri_raw) {
		//
		// Not found on headers, try by the path
		//
		const char *ptr = strrchr(request_info->local_uri_raw,'.');
		if(ptr) {
			mimetype = MimeTypeFactory(ptr+1);
		}
	}

	clog << "civetweb\t" << request_info->remote_addr << " " << status << " " << message << " (" << mimetype << ")" << endl;

	if(Config::Value<bool>("httpd","error-templates",true)) {

		try {

			string response;

			Application::DataDir page;
			page += "templates/www/error.";
			page += to_string(mimetype,true);

			trace("Searching for error page in '",page.c_str(),"'");

			if(!access(page.c_str(),R_OK)) {
				response = File::Text(page.c_str()).c_str();
			} else if(mimetype == Udjat::json) {
				response = "{\"error\":{\"application\":\"${application}\",\"code\":${code},\"message\":\"${message}\"}}";
			} else {
				cout << "civetweb\tNo access to '" << page << "', using default response" << endl;
			}

			if(!response.empty()) {

				Udjat::expand(response,[&status,&message](const char *key, std::string &value){

					if(!strcasecmp(key,"code")) {
						value = to_string(status);
					} else if(!strcasecmp(key,"message")) {
						value = message;
					} else if(!strcasecmp(key,"application")) {
						value = Application::Name();
					} else {
						value.clear();
					}

					return true;
				});

				mg_printf(
					conn,
					"HTTP/1.1 %d %s\r\n"
					"Content-Type: %s\r\n"
					"Content-Length: %u\r\n"
					"\r\n"
					"%s",
					status,message,
					std::to_string(mimetype),
					(unsigned int) response.size(),
					response.c_str()
				);
				return 0;

			}

		} catch(const std::exception &e) {
			clog << "civetweb\tError '" << e.what() << "' processing error page, using default" << endl;
		} catch(...) {
			clog << "civetweb\tUnexpected error processing error page, using default" << endl;
		}

	}

	return 1;

 }

