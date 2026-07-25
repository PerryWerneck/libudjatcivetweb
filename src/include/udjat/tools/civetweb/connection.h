/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/http/statuscodes.h>
 #include <civetweb.h>

 namespace Udjat {

	namespace CivetWeb {

		class UDJAT_PRIVATE Connection : public Udjat::HTTP::Connection {
		private:
			struct mg_connection *conn;

		public:
			Connection(struct mg_connection *c);

			inline struct mg_connection * connection() const {
				return conn;
			}

			inline operator struct mg_connection *() const {
				return conn;
			}

			inline const struct mg_request_info * request_info() const noexcept {
				return mg_get_request_info(conn);
			}

			inline const char * request_uri() const noexcept {
				return mg_get_request_info(conn)->request_uri;
			}

			inline const HTTP::Method method() const {
				return HTTP::MethodFactory(mg_get_request_info(conn)->request_method);
			}

			inline const char * local_uri() const noexcept {
				return mg_get_request_info(conn)->local_uri;
			}

			/// @brief Get mimetype from connection for convenience.
			/// @param def Default value.
			/// @return The mimetype for connection.
			Udjat::MimeType mimetype(const Udjat::MimeType def = MimeType::html) const noexcept override; 

			String address() const noexcept override;
			HTTP::StatusCode send(const char *filename, time_t maxage, const MimeType mimetype = MimeType::none) noexcept override;
			HTTP::StatusCode send(const HTTP::Status &status, const char *payload) noexcept override;
			HTTP::StatusCode redirect(const char *location) const override;
			HTTP::StatusCode logger(HTTP::StatusCode code, const char *message, Logger::Level level) const override;
			
			inline HTTP::StatusCode send(const HTTP::Status &status, bool apicall = true) noexcept {
				return HTTP::Connection::send(status,apicall);
			}

		};

	}

 }

