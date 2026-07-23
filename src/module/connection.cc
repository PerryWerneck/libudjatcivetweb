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

 #define LOG_DOMAIN "civetweb"
 #include <config.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/civetweb/connection.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/statuscodes.h>
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/http/authentication.h>
 #include <memory>
 #include <private/request.h>

 using namespace std;

 namespace Udjat {

	CivetWeb::Connection::Connection(struct mg_connection *c) : Udjat::HTTP::Connection(), conn(c) {
	
		// Check for authentication.
		try {

			auth.clear(); // Just in case.

			const char *cookie = mg_get_header(conn, "Cookie");
			if(cookie && *cookie) {
				char buffer[4096];
				int length = mg_get_cookie(cookie,auth.cookie_name().c_str(),buffer,4095);
				if(length > 0) {
					buffer[length] = 0;
					auth.token(buffer);
				}
			}

		} catch(const std::exception &e) {

			const struct mg_request_info *request_info = mg_get_request_info(conn);

			Logger::String{
				request_info->remote_addr," ",
				request_info->request_method," ",
				request_info->local_uri," --- ",
				e.what()
			}.warning("httpd");

		}
	
	}

	HTTP::StatusCode CivetWeb::Connection::logger(HTTP::StatusCode code, const char *message, Logger::Level level) const {

		const struct mg_request_info *request_info = mg_get_request_info(conn);

		Logger::String{
			request_info->remote_addr," ",
			request_info->request_method," ",
			request_info->local_uri," ",
			code," ",message
		}.write(level,"httpd");

		return code;

	}

	String CivetWeb::Connection::address() const noexcept {

		const struct mg_request_info *info = mg_get_request_info(conn);

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

	Udjat::MimeType CivetWeb::Connection::mimetype(const Udjat::MimeType def) const noexcept {

		static const char *headers[] = { "Content-Type", "Accept" };

		for(const char *header : headers) {

			// Check 'accept' header.
			const char *hdr = mg_get_header(conn, header);

			if(hdr && *hdr) {

				for(String &value : String{hdr}.split(",")) {

					auto mime = MimeTypeFactory(value.c_str(),MimeType::none);
					if(mime != MimeType::none) {
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

	HTTP::StatusCode CivetWeb::Connection::send(const HTTP::Status &status, const char *payload) noexcept {

		if(Logger::enabled(Logger::Debug)) {
			const struct mg_request_info *request_info = mg_get_request_info(conn);
			Logger::String{
				request_info->remote_addr," ",
				request_info->local_uri," Sending response ",
				status.code
			}.info();
		}

		size_t length = strlen(payload);
		if(status.code == HTTP::NoContent || status.code == HTTP::NotModified) {
			length = 0;
		}

		mg_response_header_start(conn, status.code);

		mg_response_header_add(conn, "Content-Length", std::to_string(length).c_str(), -1);

		// Set status.headers.
		status.http_headers([this](const char *name, const char *value){
			mg_response_header_add(conn,name,value,-1);
		});

		// Set authentication info.
		auth.http_headers([this](const char *name, const char *value){
			mg_response_header_add(conn,name,value,-1);
		});

		// Send...
		mg_response_header_send(conn);
		if(length) {
			mg_write(conn, payload, length);
		}

		return status.code;
	}

 }

