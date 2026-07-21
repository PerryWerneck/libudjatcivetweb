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
 #include <udjat/tools/civetweb/connection.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/statuscodes.h>
 #include <memory>
 #include <private/request.h>

//  #include <private/module.h>
//  #include <sys/types.h>
//  #include <sys/stat.h>
//  #include <private/module.h>
//  #include <private/request.h>
//  #include <udjat/tools/response.h>
//  #include <udjat/tools/http/response.h>
//  #include <udjat/tools/http/timestamp.h>
//  #include <udjat/tools/http/exception.h>
//  #include <udjat/tools/logger.h>
//  #include <udjat/tools/intl.h>
//  #include <udjat/tools/string.h>
//  #include <udjat/tools/configuration.h>
//  #include <udjat/tools/application.h>
//  #include <udjat/tools/http/template.h>

//  #ifdef HAVE_UNISTD_H
// 	#include <unistd.h>
//  #endif // HAVE_UNISTD_H

 using namespace std;
//  using namespace Udjat;

 namespace Udjat {

	const MimeType CivetWeb::Connection::mimetype(const MimeType def = MimeType::json) const noexcept {

		// Get mimetype from request header.
		const struct mg_request_info *request_info = mg_get_request_info(conn);
		if(request_info->local_uri && request_info->local_uri && !strncasecmp(request_info->local_uri,"/api/",5)) {
			return MimeTypeFactory(conn,MimeType::json);
		}

		return def;

	}

	std::shared_ptr<HTTP::Request> CivetWeb::Connection::RequestFactory() noexcept {
		return make_shared<CivetWeb::Request>(conn);
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

	const char * CivetWeb::Connection::header(const char *name, const char *def) const noexcept {

		const struct mg_request_info *info = mg_get_request_info(conn);

		for(int header = 0; header < info->num_headers; header++) {
			if(!strcasecmp(info->http_headers[header].name,name)) {
				return info->http_headers[header].value;
			}
		}

		if(def) {
			return def;
		}

		throw runtime_error(String{"The required http header '",name,"' is not available"});
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

	String CivetWeb::Connection::cookie(const char *name, const char *def) const {

		const char *cookie = mg_get_header(conn, "Cookie");

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

		throw runtime_error(String{"The required http cookie '",name,"' is not available"});
	}

	HTTP::StatusCode CivetWeb::Connection::redirect(const char *location) const {

		if(Logger::enabled(Logger::Debug)) {

			const struct mg_request_info *request_info = mg_get_request_info(conn);
			Logger::String{
				request_info->remote_addr," ",
				request_info->local_uri," redirected to ",
				location
			}.write(Logger::Info,"httpd");
			
		}

		mg_response_header_start(conn, (int) HTTP::SeeOther);
		mg_response_header_add(conn, "Location",location,-1);
		mg_response_header_add(conn, "Content-Length", "0", -1);

		// TODO: Set authentication cookie

		mg_response_header_send(conn);

		return HTTP::SeeOther;
	}

	HTTP::StatusCode CivetWeb::Connection::send(const char *filename, time_t max_age, const MimeType mimetype = MimeType::none) noexcept {

		if(Logger::enabled(Logger::Debug)) {

			const struct mg_request_info *request_info = mg_get_request_info(conn);
			Logger::String{
				request_info->remote_addr," ",
				request_info->local_uri," Sending static file ",
				filename
			}.write(Logger::Info,"httpd");
			
		}

		throw runtime_error("Incomplete");

	}

	HTTP::StatusCode CivetWeb::Connection::send(const HTTP::Status &status, const MimeType mimetype, const std::string &payload) noexcept {

		if(Logger::enabled(Logger::Debug)) {

			const struct mg_request_info *request_info = mg_get_request_info(conn);
			Logger::String{
				request_info->remote_addr," ",
				request_info->local_uri," Sending response ",
				status.code
			}.write(Logger::Info,"httpd");
			
		}

		size_t length = payload.size();
		if(status.code == HTTP::NoContent || status.code == HTTP::NotModified) {
			length = 0;
		}

		mg_response_header_start(conn, status.code);
		mg_response_header_add(conn, "Content-Type",std::to_string(mimetype),-1);
		mg_response_header_add(conn, "Content-Length", std::to_string(length).c_str(), -1);
		mg_response_header_send(conn);

		// TODO: Send authentication cookie
		// TODO: Send cache header based on code (200=standard cache, others=no cache)

		// Send response.
		if(length) {
			mg_write(conn, payload.c_str(), length);
		}

		return status.code;

	}

	// int CivetWeb::Connection::send(int code, const char *mime_type, const char *text, size_t length) const noexcept {

	// 	mg_response_header_start(conn, code);
	// 	mg_response_header_add(conn, "Content-Type",mime_type,-1);
	// 	mg_response_header_add(conn, "Content-Length", std::to_string(length).c_str(), -1);
	// 	mg_response_header_send(conn);

	// 	// TODO: Send cache header based on code (200=standard cache, others=no cache)

	// 	// Send response.
	// 	mg_write(conn, text, length);

	// 	return code;
	// }

	// int CivetWeb::Connection::send(const Udjat::HTTP::Response &response) const noexcept {
	// 	return ::send(conn,response);
	// }

 }

//  bool parse_query_string(struct mg_connection *conn,const std::function<bool(const char *key, const char *value)> &call) {


// 	return false;
//  }

//  Udjat::MimeType MimeTypeFactory(struct mg_connection *conn, const Udjat::MimeType def) noexcept {

// 	//
// 	// Check for 'mimetype=' on query
// 	//
// 	{
// 		const struct mg_request_info *request_info = mg_get_request_info(conn);

// 		if(request_info->query_string) {

// 			size_t length = strlen(request_info->query_string);
// 			char buffer[256];
// 			memset(buffer,0,sizeof(buffer));

// 			if(mg_get_var(request_info->query_string,length, "mimetype", buffer, sizeof(buffer)-1) > 0) {
// 				auto mime = MimeTypeFactory(buffer);
// 				if(mime != MimeType::none) {
// 					return mime;
// 				}
// 			}

// 		}

// 	}

// 	//
// 	// Check headers
// 	//
// 	for(const char *header : { "Content-Type", "Accept" }) {

// 		const char *hdr = mg_get_header(conn, header);

// 		if(hdr && *hdr) {

// 			for(String &value : String{hdr}.split(",")) {

// 				auto mime = MimeTypeFactory(value.c_str(),MimeType::none);
// 				if(mime != MimeType::none) {
// 					debug("Got mimetype from header '",header,"'");
// 					return mime;
// 				}
// 			}
// 		}

// 	}

// 	// Use default
// 	const struct mg_request_info *info{mg_get_request_info(conn)};
// 	Logger::String{info->remote_addr,": Unexpected mime-type on ",info->request_uri,", using ",std::to_string(def)}.warning();
// 	return def;

//  }

//  int http_error(struct mg_connection *conn, int code, const char *message, const char *body) noexcept {

// 	MimeType mimetype{MimeTypeFactory(conn)};

// 	const struct mg_request_info *request_info = mg_get_request_info(conn);

// 	Logger::String{
// 		request_info->remote_addr," ",
// 		request_info->request_method," ",
// 		request_info->local_uri," ",
// 		code," ",message," (",std::to_string(mimetype),")"
// 	}.error("civetweb");

// 	try {

// 		if(CivetWeb::Connection::apicall(conn)) {

// 			// It's an API call, send with HTTP::Response

// 			/// @brief Customized error response.
// 			class Response : public HTTP::Response {
// 			private:
// 				int code;

// 			public:
// 				Response(MimeType mimetype, int c, const char *message, const char *details)
// 					: HTTP::Response{mimetype}, code{c} {
// 					failed(message,details);
// 				}

// 				int status_code() const noexcept override {
// 					return code;
// 				}

// 				void for_each(const std::function<void(const char *header_name, const char *header_value)> &call) const noexcept override {
// 					call("Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0");
// 					call("Expires", "0");
// 				}

// 			};

// 			return ::send(conn,Response{mimetype,code,message,body});

// 		} else {

// 			// It's a HTML request, send formatted page.
			
// 			Udjat::HTTP::Template text{"error",Udjat::MimeType::html};

// 			// Expand request arguments.
// 			text.expand([code,message,body](const char *key, std::string &value) {

// 				if(!strcasecmp(key,"code")) {
// 					Logger::String{"Using obsolete '${code}' on template, change to ${error-code}"}.warning();
// 					value = std::to_string(code);
// 					return true;
// 				}

// 				if(!strcasecmp(key,"error-code")) {
// 					value = std::to_string(code);
// 					return true;
// 				}

// 				if(!strcasecmp(key,"message")) {
// 					value = message;
// 					return true;
// 				}

// 				if(!strcasecmp(key,"body")) {
// 					value = body;
// 					return true;
// 				}

// 				return false;

// 			});

// 			size_t length = text.size();

// 			mg_response_header_start(conn, code);
// 			mg_response_header_add(conn, "Content-Type",std::to_string(mimetype),-1);
// 			mg_response_header_add(conn, "Content-Length", std::to_string(length).c_str(), -1);
// 			mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
// 			mg_response_header_add(conn, "Expires", "0", -1);
// 			mg_response_header_send(conn);

// 			// Send response.
// 			mg_write(conn, text.c_str(), length);

// 			return code;

// 		}

//  	} catch(const std::exception &e) {

// 		Logger::String{"Error sending standard response: ",e.what()}.warning();

// 	} catch(...) {

// 		Logger::String{"Unexpected error sending standard response"}.warning();

// 	}

// 	// Send error without body.
// 	mg_response_header_start(conn, code);
// 	mg_response_header_add(conn, "Content-Type",std::to_string(mimetype),-1);
// 	mg_response_header_add(conn, "Content-Length", "0", -1);
// 	mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
// 	mg_response_header_add(conn, "Expires", "0", -1);
// 	mg_response_header_send(conn);

// 	return code;
//  }

//  int http_error(struct mg_connection *conn, int code, const char *message) noexcept {
//  	return http_error(conn,code,message,"");
//  }

