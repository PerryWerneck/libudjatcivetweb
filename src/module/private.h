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

 /// @brief Web handler.
 int webHandler(struct mg_connection *conn, function<string (const string &uri, const char *method, const MimeType mimetype)> worker) noexcept;

 /// @brief Handler for API requests.
 int apiWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for report requests.
 int reportWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for swagger request.
 int swaggerWebHandler(struct mg_connection *conn, void *cbdata);

 namespace Udjat {

	namespace CivetWeb {

		class Worker;

		/// @brief CivetWeb protocol header
		class Header : public Udjat::Protocol::Header {
		private:
			friend class Worker;

			std::string name;

		public:
			Header(const Header &src) = delete;
			Header(const Header *src) = delete;

			Header(const char *n) : name(n) {
			}

			inline bool operator == (const char *name) const noexcept {
				return strcasecmp(name,this->name.c_str()) == 0;
			}

		};

		/// @brief CivetWeb protocol worker.
		class Worker : public Udjat::Protocol::Worker {
		private:
			std::list<Header> headers;

			/// @brief Connect to server, send request.
			struct mg_connection * connect();

		public:
			Worker(const char *url = "", const HTTP::Method method = HTTP::Get, const char *payload = "");

			Udjat::Protocol::Header & header(const char *name) override;

			Udjat::String get(const std::function<bool(double current, double total)> &progress) override;
			bool save(const char *filename, const std::function<bool(double current, double total)> &progress) override;

		};

		/// @brief Base class for HTTP Protocol
		class Protocol : public Udjat::Protocol {
		protected:
			int use_ssl;

		public:
			Protocol(const char *name, const ModuleInfo &info, int use_ssl);
			virtual ~Protocol();

			Udjat::String call(const URL &url, const HTTP::Method method, const char *payload = "") const override;
			bool get(const URL &url, const char *filename, const std::function<bool(double current, double total)> &progress) const override;

		};

	}

 }

