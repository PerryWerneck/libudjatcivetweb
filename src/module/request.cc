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
 #include <udjat/tools/url.h>
 #include <ctype.h>

 #include <civetweb.h>

 using namespace std;

 namespace Udjat {

	namespace CivetWeb {

		Request::Request(struct mg_connection *c) : Request(c,mg_get_request_info(c)->local_uri, (unsigned int) ((PACKAGE_VERSION_MAJOR * 100) + PACKAGE_VERSION_MINOR)) {

			if(pop("/api")) {
				const char *reqpath = path();
				if(*reqpath != '/') {
					throw runtime_error(Logger::String{"Unexpected path: '",reqpath,"', requests should be in the format /api/[",apiver,"]/interface"});
				}
				if(isdigit(reqpath[1])) {
					apiver = 0;
					reqpath++;
					while(*reqpath && *reqpath != '/') {
						if(isdigit(*reqpath)) {
							apiver *= 10;
							apiver += (*reqpath - '0');
						}
						reqpath++;
					}
					reset(reqpath);
				}				
			}

			debug("Request path set to '",path(),"'");

		}

		Request::Request(struct mg_connection *c, const char *path, unsigned int ver)
			: HTTP::Request{path,mg_get_request_info(c)->request_method}, conn{c}, info{mg_get_request_info(c)} {

			apiver = ver;

			debug("request_path='",Udjat::Request::c_str(),"' (",path,")");
			
			debug("request_uri='",mg_get_request_info(c)->request_uri,"'");
			debug("local_uri_raw='",mg_get_request_info(c)->local_uri_raw,"'");
			debug("local_uri='",mg_get_request_info(c)->local_uri,"'");

#ifdef DEBUG
			{
				for(int header = 0; header < info->num_headers; header++) {
					debug("header(",info->http_headers[header].name,")='",info->http_headers[header].value,"'");
				}
			}
#endif // DEBUG

			// https://github.com/civetweb/civetweb/blob/master/examples/embedded_c/embedded_c.c
			if(!strcasecmp(header("Content-Type"),"application/x-www-form-urlencoded")) {
				
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

				mg_handle_form_request(c, &input.fdh);

			}

			parse_query(info->query_string);
			
		}

 		const char * Request::query(const char *) const {
			return info->query_string;
		}

		String Request::address() const {

			for(int header = 0; header < info->num_headers; header++) {
				if(!strcasecmp(info->http_headers[header].name,"X-Forwarded-For")) {
					Udjat::String proxy{info->http_headers[header].value};
					auto separator = proxy.find(',');
					if(separator != string::npos) {
						proxy.resize(separator);
					}
					return proxy;
				}
			}

			return info->remote_addr;
		}

		String Request::cookie(const char *name) const {

			const char *cookie = mg_get_header(conn, "Cookie");

			if(cookie && *cookie) {
				char buffer[4096];
				int length = mg_get_cookie(cookie,name,buffer,4095);
				if(length > 0) {
					buffer[length] = 0;
					return buffer;
				}
			}

			// Return default response.
			return HTTP::Request::cookie(name);
		}

		const char * Request::header(const char *name) const noexcept {

			for(int header = 0; header < info->num_headers; header++) {
				if(!strcasecmp(info->http_headers[header].name,name)) {
					return info->http_headers[header].value;
				}
			}

			return "";
		}

	}

 }
