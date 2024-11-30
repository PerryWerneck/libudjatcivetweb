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
  * @brief Implements the default http handler.
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <stdexcept>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/mimetype.h>
 #include <private/module.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/civetweb/interface.h>

 using namespace std;
 using namespace Udjat;

 int rootWebHandler(struct mg_connection *conn, void *) noexcept {

	try {

		const char *path = mg_get_request_info(conn)->local_uri;
		debug("Handling '",path,"'");


		return 0; // 0 to run internal civetweb handler.

	} catch(const exception &e) {
		HTTP::Response response{MimeTypeFactory(conn)};
		response.failed(e);
		return send(conn,response);
	} catch(...) {
		HTTP::Response response{MimeTypeFactory(conn)};
		response.failed(_("Unexpected error"));
		return send(conn,response);
	}

 }
