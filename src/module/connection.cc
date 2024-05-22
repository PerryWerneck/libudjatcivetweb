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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <private/module.h>
 #include <udjat/tools/abstract/response.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;
 using namespace Udjat;

 namespace Udjat {

	CivetWeb::Connection::operator MimeType() const {

		const struct mg_request_info *info{mg_get_request_info(conn)};

		if(strncasecmp(info->local_uri,"/api/",5) && Config::Value<bool>("httpd","allow-legacy-path",true)) {

			// Path doesn't start with /api/ and the legacy mode is enabled. Do the path starts with mimetype?
			const char * ptr = strchr(info->local_uri+1,'/');
			if(ptr) {
				string prefix{info->local_uri+1,((size_t)(ptr-info->local_uri))-1};
				auto mime = MimeTypeFactory(prefix.c_str(),MimeType::custom);
				if(mime != MimeType::custom) {
					return mime;
				}
			}

			/*
			Logger::String{"Rejecting request ",request_uri()," from ",info->remote_addr}.error("civetweb");
			throw system_error(
						ENOENT,system_category(),
						Logger::Message{
							_("Request path should be /api/{}/[REQUEST]"),
							Config::Value<string>{"http","apiver","[APIVER]"}.c_str()
						}
					);
			*/

		}

		// Get mimetype from request header.
		return MimeTypeFactory(conn,MimeType::json);

	}

	int CivetWeb::Connection::send(const char *mime_type, const char *text, size_t length) const noexcept {

		mg_response_header_start(conn, 200);
		mg_response_header_add(conn, "Content-Type",mime_type,-1);
		mg_response_header_add(conn, "Content-Length", std::to_string(length).c_str(), -1);
		mg_response_header_send(conn);

		// Send response.
		mg_write(conn, text, length);

		return 200;
	}

	int CivetWeb::Connection::send(const Abstract::Response &response) const noexcept {
		return ::send(conn,response);
	}

 }

 Udjat::MimeType MimeTypeFactory(struct mg_connection *conn, const Udjat::MimeType def) noexcept {

	static const char *headers[] = { "Content-Type", "Accept" };

	for(const char *header : headers) {

		// Check 'accept' header.
		const char *hdr = mg_get_header(conn, header);

//		debug("header[",header,"]='",hdr,"'");

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

 int http_error(struct mg_connection *conn, int code, const char *message, const char *body) noexcept {

	MimeType mimetype{MimeTypeFactory(conn)};

 	try {

		return ::send(conn,HTTP::Response{mimetype}.failed(code,message,body));

 	} catch(...) {

	}

	const struct mg_request_info *request_info = mg_get_request_info(conn);

	Logger::String{
		request_info->remote_addr," ",
		request_info->request_method," ",
		request_info->local_uri," ",
		code," ",message," (",std::to_string(mimetype),")"
	}.error("civetweb");

	// Send error without body.
	mg_response_header_start(conn, code);
	mg_response_header_add(conn, "Content-Type",std::to_string(mimetype),-1);
	mg_response_header_add(conn, "Content-Length", "0", -1);
	mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
	mg_response_header_add(conn, "Expires", "0", -1);
	mg_response_header_send(conn);

	return code;
 }

 int http_error(struct mg_connection *conn, int code, const char *message) noexcept {
 	return http_error(conn,code,message,"");
 }

