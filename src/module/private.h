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
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/string.h>
 #include <cstring>
 #include <string>
 #include <stdexcept>
 #include <civetweb.h>
 #include <udjat/civetweb.h>
 #include <iostream>
 #include <list>
 #include <cstring>

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	namespace CivetWeb {

		class UDJAT_PRIVATE Connection : public Udjat::HTTP::Connection {
		private:
			struct mg_connection *conn;

		public:
			Connection(struct mg_connection *c) : Udjat::HTTP::Connection(), conn(c) {
			}

			int success(const char *mime_type, const char *response, size_t length) const noexcept override;
			int failed(int code, const char *message) const noexcept override;
			int send(const HTTP::Method method, const char *filename, bool allow_index, const char *mime_type, unsigned int max_age) const override;

			inline struct mg_connection * connection() {
				return conn;
			}

			inline const struct mg_request_info * request_info() const noexcept {
				return mg_get_request_info(conn);
			}

			inline const char * request_uri() const noexcept {
				return mg_get_request_info(conn)->request_uri;
			}

			inline const char * local_uri() const noexcept {
				return mg_get_request_info(conn)->local_uri;
			}

		};

		class Header : public Udjat::Protocol::Header {
		public:
			Header(const char *name) : Protocol::Header(name) {
			}

			Protocol::Header & assign(const Udjat::TimeStamp &value) override;

		};

		/// @brief CivetWeb protocol worker.
		class Worker : public Udjat::Protocol::Worker {
		private:

			/// @brief Request headers.
			std::list<Header> headers;

			/// @brief Connect to server, send request.
			struct mg_connection * connect();

		public:
			Worker(const char *url = "", const HTTP::Method method = HTTP::Get, const char *payload = "");

			Udjat::String get(const std::function<bool(double current, double total)> &progress) override;
			int test(const std::function<bool(double current, double total)> &progress) noexcept override;

			bool save(const char *filename, const std::function<bool(double current, double total)> &progress, bool replace) override;

			Protocol::Header & header(const char *name) override;

		};

		/// @brief Base class for HTTP Protocol
		class Protocol : public Udjat::Protocol {
		public:
			Protocol(const char *name, const ModuleInfo &info);
			virtual ~Protocol();

			std::shared_ptr<Worker> WorkerFactory() const override;

		};

	}

 }

 /// @brief Web handler.
 int webHandler(const CivetWeb::Connection &connection, function<string (const CivetWeb::Connection &connection, const char *path, const char *method, const MimeType mimetype)> worker) noexcept;

 /// @brief Handler for API requests.
 int apiWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for icon requests.
 int iconWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for image requests.
 int imageWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for report requests.
 //int reportWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for swagger request.
 int swaggerWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for swagger request.
 int rootWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for custom requests.
 int customWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Send error page.
 int http_error( struct mg_connection *conn, int status, const char *msg );
