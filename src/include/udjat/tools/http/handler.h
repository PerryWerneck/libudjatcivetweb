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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/request.h>
 #include <cstring>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Handler {
		protected:
			const char * path;	///< @brief The path for this requests.

			/// @brief Create a new httpd handler, insert it to default server.
			/// @param path the path for the handler.
			Handler(const char *path);
			Handler(const XML::Node &node, const char *tagname = "http-handler");

		public:
			virtual ~Handler();

			inline operator bool() const noexcept {
				return (path && *path);
			}

			inline bool operator==(const char *p) const noexcept {
				return p && *p && strcasecmp(p,this->path) == 0;
			}

			inline const char * c_str() const noexcept {
				return path;
			}

			/// @brief Handle request.
			virtual int handle(const Udjat::HTTP::Connection &conn, const Udjat::HTTP::Request &request, const Udjat::MimeType mimetype) = 0;

		};

	}

 }
