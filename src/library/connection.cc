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
 #include <udjat/tools/response.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/intl.h>

 using namespace std;

 namespace Udjat {

	HTTP::Connection::Connection() {
	}

	HTTP::Connection::~Connection() {
	}

	int HTTP::Connection::success(const char *mime_type, const char *response, size_t length) const noexcept {
		return send(mime_type,response,length);
	}

	int HTTP::Connection::send(const std::exception &e) {
		HTTP::Response response{(MimeType) *this};
		response.failed(e);
		return send(response);
	}

	int HTTP::Connection::exec(const std::function<int(HTTP::Connection &connection)> &call) noexcept {

		try {

			return call(*this);

		} catch(const std::exception &e) {
			return send(e);

		} catch(...) {
			HTTP::Response response{*this};
			response.failed(_("Unexpected error"));
			return send(response);

		}

	}

 }

