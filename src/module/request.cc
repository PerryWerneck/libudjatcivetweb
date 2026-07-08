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

 #include <civetweb.h>

 using namespace std;

 namespace Udjat {

	namespace CivetWeb {

		Request::Request(struct mg_connection *c) : Request{c,mg_get_request_info(c)->local_uri, (unsigned int) ((PACKAGE_VERSION_MAJOR * 100) + PACKAGE_VERSION_MINOR)} {

			// Extract API version.
			api_call = pop("/api"); 
			if(api_call) {
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
			: HTTP::Request{path,mg_get_request_info(c)->request_method}, conn{c}, info{(mg_request_info *) mg_get_request_info(c)} {

			apiver = ver;

			debug("request_path='",Udjat::Request::c_str(),"' (",path,")");
			
			debug("request_uri='",mg_get_request_info(c)->request_uri,"'");
			debug("local_uri_raw='",mg_get_request_info(c)->local_uri_raw,"'");
			debug("local_uri='",mg_get_request_info(c)->local_uri,"'");

// #ifdef DEBUG
// 			{
// 				for(int header = 0; header < info->num_headers; header++) {
// 					debug("header(",info->http_headers[header].name,")='",info->http_headers[header].value,"'");
// 				}
// 			}
// #endif // DEBUG

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
			
			// Check for authentication
			try {

				auto cookie = session_cookie();
				debug("Authentication cookie: '",cookie,"'");
				this->auth = make_shared<HTTP::Authentication>(cookie.c_str());

			} catch(const std::exception &e) {

				// Authentication failed, trace the message and clear it.
				debug("*** Ignoring authentication cookie ***");
				Logger::String{e.what()}.trace();
				this->auth.reset();

			}

		}

 		const char * Request::query(const char *) const {
			return info->query_string;
		}

		Udjat::String Request::uri() const {
			return mg_get_request_info(conn)->local_uri;
		}

		bool Request::getProperty(const char *key, std::string &value) const {

			if(!strcasecmp(key,"redirect-uri")) {
				Udjat::String uri{
					Config::Value<string>{"authentication","redirect-uri",""}.c_str(),
					
				};
				value = uri.escape();
				return true;
			}

			return HTTP::Request::getProperty(key,value);
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

		int Request::redirect(const char *location) const {

			debug("Redirecting to '",location,"'");

			mg_response_header_start(conn, 303);
			mg_response_header_add(conn, "Location",location,-1);
			mg_response_header_add(conn, "Content-Length", "0", -1);
			mg_response_header_send(conn);

			return 303;
		}

		int Request::send(int code, const char *text) const {

			auto length = strlen(text);

			mg_response_header_start(conn, code);
			mg_response_header_add(conn, "Content-Length", std::to_string(length).c_str(), -1);
			mg_response_header_add(conn, "Content-Type",std::to_string(mimetype()),-1);

			auto auth = dynamic_pointer_cast<HTTP::Authentication>(authentication());

			if(auth) {
				// Setup cookie
				Udjat::String cookie{
					HTTP::Authentication::cookie_name().c_str(),"=",
					auth->token().c_str(),
					"; path=/; Expires=",
					HTTP::TimeStamp::to_string(auth->expires()).c_str()
				};
				debug("Cookie='",cookie,"'");
				mg_response_header_add(conn, "Set-Cookie", cookie.c_str(),-1);
			}
#ifdef DEBUG
			else {
				debug("Sending response ",code," without authentication cookie");
			}
#endif // DEBUG

			mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
			mg_response_header_add(conn, "Expires", "0",-1);

			mg_response_header_send(conn);

			mg_write(conn, text, length);

			return code;
		}

		int Request::authentication_required() const {

			if(!Authentication::available()) {
				// Authentication is not available, return error 500.
				return failed(503,strerror(ENOTSUP),_("Authentication required, but no authentication engine is available"));
			}

			if(!html()) {
				// Not HTML or no authentication, just return 'forbidden'.
				debug("API call or not html request, returning 403");

				auto auth = authentication();
				if(!auth || auth->level() == Authentication::None) {
					return failed(403,strerror(EPERM),_("This resource requires an authenticated user"));
				}

				return failed(403,strerror(EPERM),_("You dont have access to this resource"));
				
			}

			debug("HTML request, Redirecting to login page");
			// if(!strcasecmp(Config::Value<string>{"authentication","engine","undefined"}.c_str(),"internal")) {

			// 	debug("request_uri='",mg_get_request_info(conn)->request_uri,"'");

			// 	// Logger::String{"Empty html request, sending login page"}.trace();
			// 	// OAuth::Context context{conn};
			// 	// context.action = "signin";
			// 	// context.set(HTTP::Authentication::LoginPage);
			// 	// return context.send_html_response("login");

			// 	return redirect("/oauth2");
			// }

			Config::Value<Udjat::String> endpoint{"authentication","endpoint"};

			if(endpoint.empty()) {
				// Missing authentication entrypoint, error.
				return failed(500,strerror(ENOTSUP),_("The authentication endpoint is undefined"));
			}

			endpoint.expand(CivetWeb::OAuthContext{conn});

			debug("--------------------> Redirecting to '",endpoint.c_str());
			return redirect(endpoint.c_str());

		}


	}

 }
