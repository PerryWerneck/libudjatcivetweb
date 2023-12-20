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

 #include <private/module.h>
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
 #include <udjat/tools/http/value.h>
 #include <udjat/tools/logger.h>
 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 int http_error(struct mg_connection *conn, const MimeType mimetype, int code, const char *title, const char *body) {

	const struct mg_request_info *request_info = mg_get_request_info(conn);

	debug("----------------------------------------");

	HTTP::Value response{Udjat::Value::Object};
	auto &error = response["error"];
	error["code"] = code;
	error["message"] = title;
	error["body"] = body;

	Logger::String{
		request_info->remote_addr," ",
		request_info->request_method," ",
		request_info->local_uri," ",
		code," ",title," (",mimetype,")"
	}.write(Logger::Trace,"civetweb");

	String text;

	try {

		if(Config::Value<bool>("httpd","error-templates",true)) {

	#ifdef DEBUG
			Application::DataFile page{"./templates/error."};
	#else
			Application::DataFile page{"templates/www/error."};
	#endif // DEBUG
			page += to_string(mimetype,true);

			debug("Searching for error page in '",page.c_str(),"'");

			if(!access(page.c_str(),R_OK)) {

				text = File::Text(page.c_str()).c_str();

				text.expand([&response,request_info](const char *key, std::string &value) {
					value = response[key].to_string().c_str();
					return !value.empty();
				},true,true);
			}

		}

	} catch(const std::exception &e) {

		Logger::String{e.what()}.error("civetweb");
		text.clear();

	} catch(...) {

		Logger::String{"Unexpecte error formatting error page"}.error("civetweb");
		text.clear();

	}

	if(text.empty()) {

		if(mimetype != MimeType::html) {

			try {

				// Format from request.
				text = response.to_string(mimetype);

			} catch(const std::exception &e) {

				Logger::String{e.what()}.error("civetweb");
				text = response.to_string(MimeType::sh);

			}

		}
		// TODO: else { format default html }

	}

	if(text.empty()) {
		mg_send_http_error(conn, code, title);
	} else {
		mg_response_header_start(conn, code);

		mg_response_header_add(conn, "Content-Type",std::to_string(mimetype),-1);
		mg_response_header_add(conn, "Content-Length", std::to_string(text.size()).c_str(), -1);
		mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
		mg_response_header_add(conn, "Expires", "0", -1);

		mg_response_header_send(conn);

		mg_write(conn, text.c_str(), text.size());
	}

	return code;
 }

 int http_error( struct mg_connection *conn, int status, const char *message ) {

	MimeType mimetype = MimeType::html; // TODO: Get mimetype from header.
	http_error(conn, mimetype, status, message);
	return 1;

 }

/*
	for(String &value : getProperty("accept").split(",")) {
		auto mime = MimeTypeFactory(value.c_str(),MimeType::custom);
		if(mime != MimeType::custom) {
			mimetype = mime;
			break;
		}
	}

 }

 int http_error( struct mg_connection *conn, int status, const char *message ) {

	Udjat::MimeType mimetype = (Udjat::MimeType) 0;

	const struct mg_request_info *request_info = mg_get_request_info(conn);

	if(!mimetype && request_info->local_uri_raw && *request_info->local_uri_raw) {
		//
		// Not found on headers, try by the path
		//
		const char *ptr = strrchr(request_info->local_uri_raw,'.');
		if(ptr) {
			mimetype = MimeTypeFactory(ptr+1);
		}
	}

	// clog << "civetweb\t" << request_info->remote_addr << " " << status << " " << message << " (" << mimetype << ")" << endl;
	Logger::String{
		request_info->remote_addr," ",
		request_info->request_method," ",
		request_info->local_uri," ",
		status," ",message," (",mimetype,")"
	}.write(Logger::Trace,"civetweb");

	if(Config::Value<bool>("httpd","error-templates",true)) {

		try {

			string response;

#ifdef DEBUG
			Application::DataFile page{"./templates/error."};
#else
			Application::DataFile page{"templates/www/error."};
#endif // DEBUG
			page += to_string(mimetype,true);

			debug("Searching for error page in '",page.c_str(),"'");

			if(!access(page.c_str(),R_OK)) {
				response = File::Text(page.c_str()).c_str();
			} else if(mimetype == Udjat::json) {
				response = "{\"error\":{\"application\":\"${application}\",\"code\":${code},\"message\":\"${message}\"}}";
			} else {
				Logger::String{
					"No access to '",page.c_str(),"', using default response"
				}.write(Logger::Debug,"civetweb");
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
 */

