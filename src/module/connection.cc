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
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/configuration.h>

 using namespace std;

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

			Logger::String{"Rejecting request ",request_uri()," from ",info->remote_addr}.error("civetweb");
			throw HTTP::Exception(400, request_uri(), _("Request path should be /api/[APIVER]/[REQUEST]"));
		}

		// Get mimetype from request header.
		return MimeTypeFactory(conn,MimeType::json);

	}

	int CivetWeb::Connection::success(const char *mime_type, const char *response, size_t length) const noexcept {
		mg_send_http_ok(conn, mime_type, length);
		mg_write(conn, response, length);
		return 200;
	}

	int CivetWeb::Connection::failed(int status, const char *title, const char *body) const noexcept {

		if((status > 199) && (status != 204) && (status != 304)) {

			// Has body
			http_error(conn, MimeTypeFactory(conn,Udjat::MimeType::html), status, title, body);

		} else {

			// No body.
			mg_response_header_start(conn, status);
			mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
			mg_response_header_add(conn, "Expires", "0", -1);
			mg_response_header_send(conn);

		}

		return status;
	}

 }

