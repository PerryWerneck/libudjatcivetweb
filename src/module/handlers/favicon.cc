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

 using namespace Udjat;

 int faviconWebHandler(struct mg_connection *conn, void *) {

 	debug("Searching for favicon",mg_get_request_info(conn)->local_uri);

	try {

		const auto info{mg_get_request_info(conn)};
		String icon_name;

		Worker::for_each([&icon_name,info](const Worker &worker) {

			const char *converted = worker.check_path(info->local_uri);
			if(!converted) {
				icon_name = converted;
				return true;
			}

			return false;
		});

		if(icon_name.empty()) {
			icon_name = "favicon.ico";
		}

		Udjat::HTTP::Image image{icon_name.c_str()};

		CivetWeb::Connection(conn).send(
			HTTP::Get,
			image.c_str(),
			false,
			"image/svg+xml",
			Config::Value<unsigned int>("theme","favicon-max-age",604800)
		);

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
