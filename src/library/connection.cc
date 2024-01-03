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

 #include <config.h>
 #include <stdexcept>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/response.h>

 using namespace std;

 namespace Udjat {

	HTTP::Connection::Connection() {
	}

	HTTP::Connection::~Connection() {
	}

	/*
	int HTTP::Connection::response(const HTTP::Exception &error) const noexcept {
		return failed(error.codes().http, error.what());
	}

	int HTTP::Connection::response(const system_error &error) const noexcept {
		return failed(HTTP::Exception::translate(error), error.what());
	}

	int HTTP::Connection::response(const std::exception &e) const noexcept {
		return failed(500,e.what());
	}
	*/

	int HTTP::Connection::send(int code, const char *title, const char *body) const noexcept {

		HTTP::Response response{(MimeType) *this};
		response.failed(title,code);

		if(body && *body) {
			response["body"] = body;
		}

		send(response);

		if(code < 100) {
			return HTTP::Exception::translate(code);
		}

		return code;
	}

 }

