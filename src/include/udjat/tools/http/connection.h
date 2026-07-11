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
 #include <udjat/tools/value.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/report.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/method.h>
 #include <stdexcept>
 #include <system_error>
 #include <map>

 namespace Udjat {

	namespace HTTP {

		class Request;

		class UDJAT_API Connection {
		protected:
			typedef HTTP::Connection super;

		public:
			Connection();
			virtual ~Connection();

			/// @brief Run method, handle exceptions.
			/// @return HTTP error code;
			int exec(const std::function<int(HTTP::Connection &connection)> &call) noexcept;

			/// @brief Run method, handle exceptions.
			int exec(const char *path) noexcept;

			/// @brief Get the active mimetype for this connection.
			virtual operator MimeType() const = 0;

			/// @brief Check if this connection is asking for an API call or HTML page.
			virtual bool apicall() const noexcept = 0;

			/// @brief Send default HTML response.
			/// @param path Local path from request.
			/// @return Status code.
			/// @retval 200 Index page was sent.
			/// @retval 404 No index page.
			int info(const char *path);

			/// @brief Get text for response.
			/// @param response Response to.
			/// @param mimetype The mimetype for response.
			/// @return true if the string
			static std::string get(const Udjat::Response &response, const MimeType mimetype);

			/// @brief Send status.
			/// @param code HTTP error code.
			/// @param status The status to send.
			/// @return http error response.

			/// @brief Send status.
			/// @param code HTTP status code.
			/// @param status The status to send.
			/// @return Same value of code.
			int send(const Udjat::HTTP::Response::Status &status) const noexcept;

			/// @brief Send status.
			/// @param code HTTP status code.
			/// @param status The status to send.
			/// @return Same value of code.
			int send(int code, const Udjat::HTTP::Response::Status &status) const noexcept;

			/// @brief Send string.
			/// @param code Error code.
			/// @param mime_type The mimetype
			/// @param response The http payload
			/// @param length The response length.
			/// @return code.
			virtual int send(int code, const char *mime_type, const char *payload, size_t length) const noexcept = 0;

			inline int send(const char *mime_type, const char *payload, size_t length) const noexcept {
				return send(200,mime_type,payload,length);
			}

			/// @brief Send exception.
			int send(const std::exception &e);

			/// @brief Send file.
			/// @param Method The HTTP method from client.
			/// @param filename The filename to send.
			/// @param allow_index If true and filename is a directory, send a simple html index.
			/// @param mime_type The mime type for file (will be replaced with html if the filename is a directory)
			/// @param max_age File cache time, in seconds.
			/// @return HTML response code.
			virtual int send(const HTTP::Method method, const char *filename, bool allow_index = false, const char *mime_type = nullptr, unsigned int max_age = 0) const = 0;

			/// @brief Send response.
			/// @param mime_type The content type to be sent.
			/// @param length Length of the following body data.
			/// @return Fixed value '200'.
			virtual int success(const char *mime_type, const char *response, size_t length) const noexcept;

			inline int success(const char *mime_type, const std::string &response) const noexcept {
				return success(mime_type,response.c_str(),response.size());
			}

			virtual int failed(int code, const char *message, const char *body) const noexcept;

			virtual std::shared_ptr<HTTP::Request> RequestFactory() = 0;
			virtual std::shared_ptr<HTTP::Response> ResponseFactory();

			/// @brief Send template
			/// @param code HTTP response code.
			/// @param tmplt Template name
			/// @param callback Callback to process ${name}.
			/// @return HTTP response code.
			int send_template(int code, const char *tmplt, const std::function<void(const char *key, std::ostream &stream)> &callback) const;

			// Standard handlers

			/// @brief Send standard favicon.
			/// @return HTTP status code.
			int favicon() noexcept;

 			/// @brief Send standard icon.
 			/// @param name The icon name.
			/// @return HTTP status code.
 			int icon(const char *name) noexcept;

			/// @brief Send standard image.
 			/// @param name The image name.
			/// @return HTTP status code.
 			int image(const char *name) noexcept;
			
			/// @brief Handle http request.
			/// @return HTTP status code.
			int handle() noexcept;

		};

	}

 }
