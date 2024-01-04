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

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <udjat/tools/http/icon.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

 using namespace Udjat;

 int iconWebHandler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {

 	debug("Searching for icon",mg_get_request_info(conn)->local_uri);

	try {

		const char *path = strrchr(mg_get_request_info(conn)->local_uri,'/');
		if(path) {
			path++;
		}

		if(!(path && *path)) {
			mg_send_http_error(conn, 400, "Unable to handle icon %s", mg_get_request_info(conn)->local_uri);
			return 400;
		}

		debug("path='",path,"'");
		Udjat::HTTP::Icon icon = Udjat::HTTP::Icon::getInstance(path);

		if(icon.empty()) {
			mg_send_http_error(conn, 404, "Cant find icon '%s'", path);
			return 404;
		}

		CivetWeb::Connection(conn).send(
			HTTP::Get,
			icon.c_str(),
			false,
			"image/svg+xml",
			Config::Value<unsigned int>("theme","icon-max-age",604800)
		);

	} catch(const HTTP::Exception &error) {

		mg_send_http_error(conn, error.codes().http, "%s", error.what());
		return error.codes().http;

	} catch(const system_error &e) {

		int code = HTTP::Exception::code(e);
		mg_send_http_error(conn, code, "%s", e.what());
		return code;

	} catch(const exception &e) {

		mg_send_http_error(conn, 500, "%s", e.what());
		return 500;

	} catch(...) {

		mg_send_http_error(conn, 500, "%s", "Unexpected error");
		return 500;

	}

	mg_send_http_error(conn, 404, "Not available");
	return 404;

 }
