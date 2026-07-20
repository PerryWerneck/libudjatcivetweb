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

 #include <udjat/defs.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/statuscodes.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/authentication.h>
 #include <udjat/tools/http/request.h>
 #include <memory>
 
 namespace Udjat {

	namespace HTTP {

		class Request;

		class UDJAT_API Connection {
		protected:
			typedef HTTP::Connection super;

			/// @brief Authentication for this connection.
			std::shared_ptr<HTTP::Authentication> auth;

			/// @brief Send file to client.
			/// @param mimetype The mime-type for http header.
			/// @param max_age The max-age value to http headers (0 to no-cache).
			/// @param filename The file to send.
			/// @return The status code (404 if the file was not found).
			virtual HTTP::StatusCode send_file(const MimeType mimetype, time_t max_age, const char *filename) noexcept = 0;

			/// @brief Send response to client.
			/// @param status The status for http header.
			/// @param payload The payload (if available).
			/// @return The status code.
			virtual HTTP::StatusCode send_response(const HTTP::Status &status, const std::string &payload) noexcept = 0;

			/// @brief Build request for this connection.
			/// @param The request for this connection.
			virtual std::shared_ptr<HTTP::Request> RequestFactory() noexcept;

		public:
			Connection() = default;
			virtual ~Connection();

			inline std::shared_ptr<HTTP::Authentication> authentication() const noexcept {
				return auth;
			}

			/// @brief Send standard favicon.
			/// @return HTTP status code.
			HTTP::StatusCode favicon() noexcept;

 			/// @brief Send standard icon.
 			/// @param name The icon name.
			/// @return HTTP status code.
 			HTTP::StatusCode icon(const char *name) noexcept;

			/// @brief Send standard image.
 			/// @param name The image name.
			/// @return HTTP status code.
 			HTTP::StatusCode image(const char *name) noexcept;
			
			/// @brief Handle http request.
			/// @return HTTP status code.
			HTTP::StatusCode handle() noexcept;

		};

	}

 }
