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
  * @brief Implements Request::exec().
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/report.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	int HTTP::Request::exec(HTTP::Connection &connection) {

		HTTP::Response response{(MimeType) connection};
		Udjat::Interface::call(pop().c_str(),*this,response);
		return connection.send(response);
		
	}

	String HTTP::Request::cookie(const char *) const {
		return "";
	}

 }
