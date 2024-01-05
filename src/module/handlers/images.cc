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
 #include <udjat/tools/http/image.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/http/connection.h>

 #include <private/module.h>
 #include <civetweb.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 int imageWebHandler(struct mg_connection *conn, void *) {

	try {

		const char *path = mg_get_request_info(conn)->local_uri;
		while(*path && *path == '/') {
			path++;
		}

		const char *ptr = strchr(path,'/');

		if(ptr) {
			path = ptr+1;
		}

		Udjat::HTTP::Image image{path};

		return CivetWeb::Connection(conn).send(
			HTTP::Get,
			image.c_str(),
			false,
			"image/svg+xml",
			Config::Value<unsigned int>("theme","image-max-age",604800)
		);

	} catch(const HTTP::Exception &e) {
		return send(conn, HTTP::Response{MimeTypeFactory(conn)}.failed(e));

	} catch(const system_error &e) {
		return send(conn, HTTP::Response{MimeTypeFactory(conn)}.failed(e));

	} catch(const exception &e) {
		return send(conn, HTTP::Response{MimeTypeFactory(conn)}.failed(e));

	} catch(...) {
		return send(conn, HTTP::Response{MimeTypeFactory(conn)}.failed(_("Unexpected error")));

	}

 }
