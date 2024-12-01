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
 #include <udjat/tools/logger.h>
 #include <udjat/tools/civetweb/service.h>

 #include <private/module.h>
 #include <civetweb.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 int CivetWeb::Service::image_handler(struct mg_connection *conn, CivetWeb::Service *) noexcept {

	try {

		const char *path = mg_get_request_info(conn)->local_uri;
		while(*path && *path == '/') {
			path++;
		}

		const char *ptr = strchr(path,'/');

		if(ptr) {
			path = ptr+1;
		}

		debug("searching for image '",path,"'");

		Udjat::HTTP::Image filename{path};

		if(filename) {

			Logger::String{"Sending static file '", filename.c_str(),"'"}.trace("http");
			mg_send_file(conn,filename.c_str());
			return 200;

		} else {

			Logger::String{"Cant find static file '", filename.c_str(),"'"}.error("http");

		}

	} catch(const exception &e) {
		HTTP::Response response{MimeTypeFactory(conn)};
		response.failed(e);
		return send(conn,response);
	} catch(...) {
		HTTP::Response response{MimeTypeFactory(conn)};
		response.failed(_("Unexpected error"));
		return send(conn,response);
	}

	return http_error(conn, 404, _("Not available"));

 }
