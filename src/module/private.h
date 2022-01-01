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

 #pragma once

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/mimetype.h>
 #include <cstring>
 #include <string>
 #include <stdexcept>
 #include <civetweb.h>
 #include <udjat/civetweb.h>
 #include <iostream>

 using namespace Udjat;
 using namespace std;

 /// @brief Web handler.
 int webHandler(struct mg_connection *conn, function<string (const string &uri, const char *method, const MimeType mimetype)> worker) noexcept;

 /// @brief Handler for API requests.
 int apiWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for report requests.
 int reportWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for swagger request.
 int swaggerWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief CivetWeb HTTP Response
 class URLResponse : public Udjat::URL::Response {
 private:

 public:
	URLResponse(struct mg_connection *conn);
	virtual ~URLResponse();
	bool isValid() const noexcept override;

 };

 /// @brief Base class for HTTP Protocol
 class Protocol : public Udjat::URL::Protocol {
 protected:
	int use_ssl;

 public:
	Protocol(const char *name, const char *port, const ModuleInfo *info, int use_ssl);
	virtual ~Protocol();
	std::shared_ptr<URL::Response> call(const URL &url, const URL::Method method, const char *mimetype, const char *payload) override;

 };

 class http_error : public std::exception {
 private:
	int id = 500;
	string message;

 public:
	http_error(int i, const char *m) : id(i), message(m) {
	}

	virtual ~http_error() {
	}

	inline int code() const noexcept {
		return this->id;
	}

	const char* what() const noexcept override {
		return this->message.c_str();
	}

 };
