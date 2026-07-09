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

 #include <private/module.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <private/module.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/http/template.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;
 using namespace Udjat;

 namespace Udjat {

	CivetWeb::Connection::operator MimeType() const {

		// Get mimetype from request header.
		const struct mg_request_info *request_info = mg_get_request_info(conn);
		if(request_info->local_uri && request_info->local_uri && !strncasecmp(request_info->local_uri,"/api/",5)) {
			return MimeTypeFactory(conn,MimeType::json);
		}

		return MimeTypeFactory(conn,MimeType::html);

	}

	bool CivetWeb::Connection::apicall(struct mg_connection *conn) noexcept {

		const struct mg_request_info *request_info = mg_get_request_info(conn);

		if(request_info->local_uri && request_info->local_uri && !strncasecmp(request_info->local_uri,"/api/",5)) {
			return true;
		}

		return MimeTypeFactory(conn,MimeType::html) != MimeType::html;

	}

	bool CivetWeb::Connection::apicall() const noexcept {
		return apicall(conn);
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

	int CivetWeb::Connection::send(const Udjat::HTTP::Response &response) const noexcept {
		return ::send(conn,response);
	}

 }

 Udjat::MimeType MimeTypeFactory(struct mg_connection *conn, const Udjat::MimeType def) noexcept {

	//
	// Check for 'format=' on query
	//
	const char *query = mg_get_request_info(conn)->query_string;
	if(query && *query) {
		for(const auto &arg : String{query}.split("&")) {
			if(!strncasecmp(arg.c_str(),"format=",7)) {
				auto mime = MimeTypeFactory(arg.c_str()+7,MimeType::none);
				if(mime != MimeType::none) {
					return mime;
				}
			}
		}
	}

	//
	// Check headers
	//
	for(const char *header : { "Content-Type", "Accept" }) {

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
	Logger::String{info->remote_addr,": Unexpected mime-type on ",info->request_uri,", using ",std::to_string(def)}.warning();
	return def;

 }

 int http_error(struct mg_connection *conn, int code, const char *message, const char *body) noexcept {

	MimeType mimetype{MimeTypeFactory(conn)};

	const struct mg_request_info *request_info = mg_get_request_info(conn);

	Logger::String{
		request_info->remote_addr," ",
		request_info->request_method," ",
		request_info->local_uri," ",
		code," ",message," (",std::to_string(mimetype),")"
	}.error("civetweb");

	try {

		if(CivetWeb::Connection::apicall(conn)) {

			// It's an API call, send with HTTP::Response

			/// @brief Customized error response.
			class Response : public HTTP::Response {
			private:
				int code;

			public:
				Response(MimeType mimetype, int c, const char *message, const char *details)
					: HTTP::Response{mimetype}, code{c} {
					failed(message,details);
				}

				int status_code() const noexcept override {
					return code;
				}

				void for_each(const std::function<void(const char *header_name, const char *header_value)> &call) const noexcept override {
					call("Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0");
					call("Expires", "0");
				}

			};

			return ::send(conn,Response{mimetype,code,message,body});

		} else {

			// It's a HTML request, send formatted page.
			
			Udjat::HTTP::Template text{"error",Udjat::MimeType::html};

			// Expand request arguments.
			text.expand([code,message,body](const char *key, std::string &value) {

				if(!strcasecmp(key,"code")) {
					Logger::String{"Using obsolete '${code}' on template, change to ${error-code}"}.warning();
					value = std::to_string(code);
					return true;
				}

				if(!strcasecmp(key,"error-code")) {
					value = std::to_string(code);
					return true;
				}

				if(!strcasecmp(key,"message")) {
					value = message;
					return true;
				}

				if(!strcasecmp(key,"body")) {
					value = body;
					return true;
				}

				return false;

			});

			size_t length = text.size();

			mg_response_header_start(conn, code);
			mg_response_header_add(conn, "Content-Type",std::to_string(mimetype),-1);
			mg_response_header_add(conn, "Content-Length", std::to_string(length).c_str(), -1);
			mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
			mg_response_header_add(conn, "Expires", "0", -1);
			mg_response_header_send(conn);

			// Send response.
			mg_write(conn, text.c_str(), length);

			return code;

		}

 	} catch(const std::exception &e) {

		Logger::String{"Error sending standard response: ",e.what()}.warning();

	} catch(...) {

		Logger::String{"Unexpected error sending standard response"}.warning();

	}

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

