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
 #include <udjat/tools/method.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/mimetype.h>
 #include <stdexcept>
 #include <system_error>
 #include <map>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Connection {
		public:
			Connection();
			virtual ~Connection();

			/// @brief Get the active mimetype for this connection.
			virtual operator MimeType() const = 0;

			/// @brief Send default HTML response.
			/// @param path Local path from request.
			/// @return Status code.
			/// @retval 200 Index page was sent.
			/// @retval 404 No index page.
			int info(const char *path);

			/// @brief Send response.
			/// @param mime_type The content type to be sent.
			/// @param length Length of the following body data.
			/// @return Fixed value '200'.
			virtual int success(const char *mime_type, const char *response, size_t length) const noexcept = 0;

			/// @brief Send file.
			/// @param Method The HTTP method from client.
			/// @param filename The filename to send.
			/// @param allow_index If true and filename is a directory, send a simple html index.
			/// @param mime_type The mime type for file (will be replaced with html if the filename is a directory)
			/// @param max_age File cache time, in seconds.
			/// @return HTML response code.
			virtual int send(const HTTP::Method method, const char *filename, bool allow_index = false, const char *mime_type = nullptr, unsigned int max_age = 0) const = 0;

			int success(const char *mime_type, const std::string &response) const {
				return success(mime_type,response.c_str(),response.size());
			}

			int response(const HTTP::Exception &error) const noexcept;
			int response(const std::system_error &error) const noexcept;
			int response(const std::exception &e) const noexcept;

			/// @brief Send error.
			/// @param code The HTTP status code (see HTTP standard).
			/// @param message The error message.
			/// @return Error code.
			virtual int failed(int code, const char *message) const noexcept = 0;

		};

	}

 }
