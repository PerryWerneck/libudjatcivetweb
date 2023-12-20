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

 Udjat::MimeType MimeTypeFactory(struct mg_connection *conn, const Udjat::MimeType def) {

	static const char *headers[] = { "Content-Type", "Accept" };

	for(const char *header : headers) {

		// Check 'accept' header.
		const char *hdr = mg_get_header(conn, header);

		if(hdr && *hdr) {

			for(String &value : String{hdr}.split(",")) {

				auto mime = MimeTypeFactory(value.c_str(),MimeType::custom);
				if(mime != MimeType::custom) {
					return mime;
				}
			}
		}

	}

	// Use default
	const struct mg_request_info *info{mg_get_request_info(conn)};
	Logger::String{info->remote_addr,": Unexpected mime-type on ",info->request_uri,", using ",std::to_string(def)}.warning("civetweb");
	return def;

 }

 /// @brief Application error handler.
 int http_error(struct mg_connection *conn, const MimeType mimetype, int code, const char *title, const char *body) {

	const struct mg_request_info *request_info = mg_get_request_info(conn);

	// Format standard response.
	HTTP::Value response{Udjat::Value::Object};
	auto &error = response["error"];
	error["code"] = code;
	error["message"] = title;
	error["body"] = body;

	Logger::String{
		request_info->remote_addr," ",
		request_info->request_method," ",
		request_info->local_uri," ",
		code," ",title," (",std::to_string(mimetype),")"
	}.error("civetweb");

	String text;

	try {

		if(Config::Value<bool>("httpd","error-templates",true)) {

			//
			// If exist an error template for this mimetype use it.
			//

#ifdef DEBUG
			Application::DataFile page{"./templates/error."};
#else
			Application::DataFile page{"templates/www/error."};
#endif // DEBUG
			page += to_string(mimetype == MimeType::custom ? MimeType::html : mimetype,true);

			if(!access(page.c_str(),R_OK)) {

				Logger::String{"Loading error page from '",page.c_str(),"'"}.trace("civetweb");

				text = page.load().expand([&error,request_info](const char *key, std::string &value) {
					value = error[key].to_string().c_str();
					return !value.empty();
				},true,true);

			}

		}

	} catch(const std::exception &e) {

		Logger::String{e.what()}.error("civetweb");
		text.clear();

	} catch(...) {

		Logger::String{"Unexpected error formatting error page"}.error("civetweb");
		text.clear();

	}

	if(text.empty()) {

		//
		// No template, format standard exit.
		//

		if(mimetype != MimeType::html) {

			//
			// Render response as 'mimetype' data.
			//
			try {

				// Format from request.
				text = response.to_string(mimetype);

			} catch(const std::exception &e) {

				// Failed, render as 'shell-script'.
				Logger::String{e.what()}.error("civetweb");
				text = response.to_string(MimeType::sh);

			}

		}
		// TODO: else { format default html }

	}

	if(text.empty()) {

		// No text, use civetweb standard response
		mg_send_http_error(conn, code, title);

	} else {

		// Send formatted response.
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

 /// @brief CivetWeb error handler.
 int http_error( struct mg_connection *conn, int status, const char *message ) {
	http_error(conn, MimeTypeFactory(conn,Udjat::MimeType::html), status, message);
	return 1;
 }

