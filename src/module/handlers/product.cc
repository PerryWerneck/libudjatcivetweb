/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements the product file handler.
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/application.h>
 #include <stdexcept>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;
 using namespace std;

 int productWebHandler(struct mg_connection *conn, void *) noexcept {

	try {

		static const char *prefix = "/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/";

		const char *path = mg_get_request_info(conn)->local_uri;

		if(strncasecmp(path,prefix,strlen(prefix))) {
			throw logic_error(Logger::String{"Invalid product path '",path,"'"});
		}

#ifdef _WIN32
		Application::DataFile filename{"www"};
#else
		Application::DataFile filename{"/srv/www/htdocs"};
#endif // _WIN32

		filename += path;

		debug("Searching for '",filename.c_str(),"'");

		if(filename) {

			Logger::String{"Sending static file '", filename.c_str(),"'"}.trace("http");
			mg_send_file(conn,filename.c_str());
			return 200;

		} else {

			Logger::String{"Cant find static file '", filename.c_str(),"'"}.error("http");

		}


	} catch(const HTTP::Exception &e) {
		return http_error(conn, e.code(), e.what());

	} catch(const system_error &e) {
		return http_error(conn, HTTP::Exception::code(e), e.what());

	} catch(const exception &e) {
		return http_error(conn, 500, e.what());

	} catch(...) {
		return http_error(conn, 500, "Unexpected error");

	}

	return http_error(conn, 404, _("Not available"));

 }
