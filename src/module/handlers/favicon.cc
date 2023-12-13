/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements the faviconn output.
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/image.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/worker.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/icon.h>
 #include <fcntl.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;

 int faviconWebHandler(struct mg_connection *conn, void *) {

	try {

		// TODO: Search workers for favicon.

		//
		// Get default favicon
		//
#ifndef _WIN32
		Config::Value<string> filename{"theme","favicon","/usr/share/pixmaps/distribution-logos/favicon.ico"};
#else
		Udjat::HTTP::Icon filename = Udjat::HTTP::Icon::getInstance("favicon");
#endif // _WIN32

		if(!filename.empty() && access(filename.c_str(),R_OK) == 0) {
			CivetWeb::Connection(conn).send(
				HTTP::Get,
				filename.c_str(),
				false,
				"image/x-icon",
				Config::Value<unsigned int>("theme","icon-max-age",604800)
			);
		}

		mg_send_http_error(conn, 404, "Cant find icon");

	} catch(const HTTP::Exception &error) {

		mg_send_http_error(conn, error.codes().http, "%s", error.what());
		return error.codes().http;

	} catch(const system_error &e) {

		int code = HTTP::Exception::translate(e);
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
