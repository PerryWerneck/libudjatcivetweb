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

 int iconWebHandler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {

	try {

		Udjat::HTTP::Icon icon = Udjat::HTTP::Icon::getInstance(mg_get_request_info(conn)->local_uri);

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