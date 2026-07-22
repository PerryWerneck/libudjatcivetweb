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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/civetweb/service.h>
 #include <udjat/tools/civetweb/connection.h>
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/string.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	int CivetWeb::Service::product_handler(struct mg_connection *conn, CivetWeb::Service *) noexcept {

		CivetWeb::Connection client{conn};

		try {

#ifdef _WIN32
			Application::DataFile htdocs = Config::Value<string>{"httpd","doc-path","www/htdocs/"}.c_str();
#else
			Application::DataFile htdocs = Config::Value<string>{"httpd","doc-path","/srv/www/htdocs/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/"}.c_str();
#endif // _WIN32

			String filename {
				htdocs.c_str(),
				client.local_uri()
			};

			return (int) client.send(
				filename.c_str(),
				(time_t) Config::Value<unsigned int>{"theme","file-max-age",3600},
				MimeTypeFactory(filename.c_str())
			);

		} catch(const std::exception &e) {

			return (int) client.send(e,_("Unexpected error"));

		} catch(...) {

			return (int) client.send(HTTP::SystemError,_("Unexpected error"));

		}

	}

 }
