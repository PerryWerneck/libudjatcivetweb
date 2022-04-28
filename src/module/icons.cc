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

 /**
  * @brief Implements the swagger.json output.
  *
  * References:
  *
  * https://samanthaneilen.github.io/2018/12/08/Using-and-extending-swagger.json-for-API-documentation.html
  *
  */

 #include "private.h"
 #include <udjat/tools/http/icons.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/configuration.h>
 #include <sys/types.h>
 #include <sys/stat.h>

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

 int iconWebHandler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {

	try {

		const char *path = mg_get_request_info(conn)->local_uri;
		while(*path && *path == '/') {
			path++;
		}

		const char *ptr = strchr(path,'/');

		cout << "********************** " << ptr << endl;

		if(ptr) {
			path = ptr+1;
		}

		Udjat::HTTP::Icon icon = Udjat::HTTP::Icon::getInstance(path);

		mg_response_header_start(conn, 200);

		struct stat st;
		if(stat(icon.c_str(), &st) < 0) {
			throw system_error(errno,system_category(),icon.c_str());
		}

		//
		// Send file.
		//
		unsigned int maxage = Config::Value<unsigned int>("theme","icon-max-age",604800);
		if(maxage) {
			mg_response_header_add(conn, "Cache-Control", (string{"public,max-age="} + std::to_string(maxage) + ",immutable").c_str(), -1);
			mg_response_header_add(conn, "Expires", HTTP::TimeStamp(time(0)+maxage).to_string().c_str(), -1);
		}

		mg_response_header_add(conn, "Last-Modified", HTTP::TimeStamp(st.st_mtime).to_string().c_str(), -1);
		mg_response_header_add(conn, "Content-Length", std::to_string(st.st_size).c_str(), -1);
		mg_response_header_add(conn, "Content-Type", "image/svg+xml", -1);

		mg_response_header_send(conn);

		mg_send_file_body(conn,icon.c_str());

		return 200;

	} catch(const HTTP::Exception &error) {

		mg_send_http_error(conn, error.codes().http, error.what());
		return error.codes().http;

	} catch(const system_error &e) {

		int code = HTTP::Exception::translate(e);
		mg_send_http_error(conn, code, e.what());
		return code;

	} catch(const exception &e) {

		mg_send_http_error(conn, 500, e.what());
		return 500;

	} catch(...) {

		mg_send_http_error(conn, 500, "Unexpected error");
		return 500;

	}

	mg_send_http_error(conn, 404, "Not available");
	return 404;

 }
