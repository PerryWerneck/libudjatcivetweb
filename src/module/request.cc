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
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/authentication.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/string.h>
 #include <private/oauth.h>
 #include <ctype.h>
 #include <stdexcept>

 #include <civetweb.h>

 using namespace std;

 namespace Udjat {

	namespace CivetWeb {

		Request::Request(CivetWeb::Connection &c) 
			: HTTP::Request{c.local_uri(),c.method()}, conn{c} {

			auto mg_conn = c.connection();
			const struct mg_request_info *ri = mg_get_request_info(mg_conn);

			// https://github.com/civetweb/civetweb/blob/master/examples/embedded_c/embedded_c.c
			if(!strcasecmp(header("Content-Type",""),"application/x-www-form-urlencoded")) {
				
				//
				// It's a form, get values
				//

				// https://github.com/civetweb/civetweb/blob/master/examples/embedded_c/embedded_c.c#L466
				struct InputParser {

					string name;
					Udjat::Value &values;

					static int field_found(const char *key,const char *,char *,size_t ,void *user_data) {
						debug("Field name : '", key , "'");
						((InputParser *) user_data)->name = key;
						return MG_FORM_FIELD_STORAGE_GET;
					}

					static int field_get(const char *, const char *value, size_t valuelen, void *user_data) {
						if(valuelen) {
							debug("   [",string{value,valuelen},"]");
							((InputParser *) user_data)->values[((InputParser *) user_data)->name.c_str()] = Udjat::String{value,valuelen}.unescape().c_str();
						}
						return MG_FORM_FIELD_HANDLE_GET;
					}

					static int field_stored(const char *, long long, void *) {
						return 0;
					}

					struct mg_form_data_handler fdh;

					InputParser(Udjat::Value &v) : values{v}, fdh{field_found, field_get, field_stored, this} {
					}

				};

				InputParser input{*this};

				mg_handle_form_request(mg_conn, &input.fdh);

			} else if (strcmp(ri->request_method, "POST") == 0) {

				// It's a post request, parse contents.

				// TODO: Load payload, check mime-type and parse values.

				Logger::String{"The parsing for 'post' data is incomplete"}.error("civetweb");

			} else if (ri->query_string && *ri->query_string && strcmp(ri->request_method, "GET") == 0) {

				// It's a 'GET' request, parse values from query string.

				for(const auto &query : Udjat::String{ri->query_string}.split("&")) {
					auto vals = query.split("=",2);
					char decoded_val[4096];
					mg_url_decode(
						vals[1].c_str(),
						vals[1].size(), 
						decoded_val,
						4095, 
						1
					);
					(*this)[vals[0].c_str()] = decoded_val;
				}
			}
		}

		Udjat::String Request::uri() const {
			return conn.local_uri();
		}

		bool Request::get_property(const char *key, Udjat::Value &value) const {

			if(HTTP::Request::get_property(key,value)) {
				return true;
			}

			if(conn.get_property(key,value)) {
				return true;
			}

			// if(!strcasecmp(key,"redirect-uri")) {
			// 	Udjat::String uri{
			// 		Config::Value<string>{"authentication","redirect-uri",""}.c_str(),	
			// 	};
			// 	value = uri.escape();
			// 	return true;
			// }

			const struct mg_request_info *info = mg_get_request_info(conn.connection());
			for(int header = 0; header < info->num_headers; header++) {
				if(!strcasecmp(info->http_headers[header].name,key)) {
					value = info->http_headers[header].value;
					return true;
				}
			}

			return false;
		}

		const char * Request::header(const char *name, const char *def) const noexcept {

			const struct mg_request_info *info = mg_get_request_info(conn.connection());

			for(int header = 0; header < info->num_headers; header++) {
				if(!strcasecmp(info->http_headers[header].name,name)) {
					return info->http_headers[header].value;
				}
			}

			if(def) {
				return def;
			}

			throw runtime_error(Udjat::String{"The required http header '",name,"' is not available"});

		}

		String Request::cookie(const char *name, const char *def) const {

			const char *cookie = header("Cookie");

			if(cookie && *cookie) {
				char buffer[4096];
				int length = mg_get_cookie(cookie,name,buffer,4095);
				if(length > 0) {
					buffer[length] = 0;
					return buffer;
				}
			}

			if(def) {
				return def;
			}

			throw runtime_error(Udjat::String{"The required http cookie '",name,"' is not available"});

		}

	}

 }
