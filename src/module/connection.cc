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

	int CivetWeb::Connection::success(const char *mime_type, const char *response, size_t length) const noexcept {
		mg_send_http_ok(conn, mime_type, length);
		mg_write(conn, response, length);
		return 200;
	}

	int CivetWeb::Connection::failed(int status, const char *message) const noexcept {

		mg_response_header_start(conn, status);
		mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
		mg_response_header_add(conn, "Expires", "0", -1);

		// TODO: Format response based on request mime-type
		mg_response_header_add(conn, "Content-Type","text/plain; charset=utf-8",-1);

		if((status > 199) && (status != 204) && (status != 304)) {

			// Has body

			// TODO: Check for Configuration if we should change status to 200
			//       and use 'rc=-1' to avoid failure on legacy applications.
			// Config::Value<bool> legacy{"httpd","use_legacy_responses",false};

			// TODO: Format body based on request mime-type.
			Logger::Message body{
				_("Erro {} ({}) on HTTP request\n{}\n"),
					status,
					mg_get_response_code_text(conn, status),
					message
			};

			mg_response_header_add(conn, "Content-Length", std::to_string(body.size()).c_str(), -1);

 			mg_response_header_send(conn);
			mg_write(conn, body.c_str(), body.size());

		} else {

			// No body.

			mg_response_header_send(conn);

		}


//		mg_send_http_error(conn, code, message);

		return status;
	}

 }

