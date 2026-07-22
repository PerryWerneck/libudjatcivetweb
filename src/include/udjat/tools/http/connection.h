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
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/authentication.h>
 #include <udjat/tools/logger.h>
 #include <memory>
 #include <string>
 
 namespace Udjat {

	namespace HTTP {

		class Request;

		class UDJAT_API Connection {
		protected:
			typedef HTTP::Connection super;

			/// @brief Authentication for this connection.
			HTTP::Authentication auth;

		public:
			Connection() = default;
			virtual ~Connection();

			/// @brief Get authentication for this connection.
			/// @return The authentication object associated with this connection.
			inline const HTTP::Authentication & authentication() const noexcept {
				return auth;
			}

			/// @brief Send file to client.
			/// @param mimetype The mime-type for http header (MimeType::None to get it from filename).
			/// @param max_age The max-age value to http headers (0 to no-cache).
			/// @param filename The file to send.
			/// @return The status code (404 if the file was not found).
			virtual HTTP::StatusCode send(const char *filename, time_t maxage, const MimeType mimetype = MimeType::none) noexcept = 0;

			/// @brief Send response to client.
			/// @param status The status for http header.
			/// @param payload The payload.
			/// @return The response code.
			virtual HTTP::StatusCode send(const HTTP::Status &status, const MimeType mimetype, const char *payload) noexcept = 0;

			/// @brief Send response to client.
			/// @param status The response status.
			/// @param apicall True if the request is an api call.
			/// @return The response code.
			HTTP::StatusCode send(const HTTP::Status &status, const MimeType mimetype, bool apicall = true) noexcept;

			/// @brief Send success response to client.
			/// @param payload The response payload
			/// @return The response code (200)
			inline HTTP::StatusCode send(const MimeType mimetype, const char *payload) noexcept {
				return send(HTTP::Status{HTTP::Ok},mimetype,payload);
			}

			/// @brief Send a redirect response.
			/// @param location The new location.
			/// @return The status code (Usually HTTP::Redirect)
			virtual HTTP::StatusCode redirect(const char *location) const = 0;
		
			/// @brief Get the client address (if available).
			/// @return The client address, empty if not available.
			virtual String address() const noexcept = 0;

			virtual bool get_property(const char *key, Udjat::Value &value) const;

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
			/// @param request The HTTP request to handle.
			/// @return HTTP status code.
			HTTP::StatusCode handle(HTTP::Request &request) noexcept;

			/// @brief Send logger message.
			virtual HTTP::StatusCode logger(HTTP::StatusCode code, const char *message, Logger::Level level) const;

			inline HTTP::StatusCode info(HTTP::StatusCode code, const char *message) const {
				return logger(code,message,Logger::Info);
			}

			inline HTTP::StatusCode warning(HTTP::StatusCode code, const char *message) const {
				return logger(code,message,Logger::Warning);
			}
			
			inline HTTP::StatusCode error(HTTP::StatusCode code, const char *message) const {
				return logger(code,message,Logger::Error);
			}
			
			inline HTTP::StatusCode notice(HTTP::StatusCode code, const char *message) const {
				return logger(code,message,Logger::Notice);
			}
			
		};

	}

 }
