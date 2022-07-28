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
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/request.h>
 #include <map>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Server {
		private:
			static Server *instance;

		protected:
			Server();

		public:

			class UDJAT_API Handler {
			private:
				friend class Server;
				const Server *server = nullptr;

			public:
				/// @brief Create a new httpd handler.
				/// @param uri The URI to hook the handler on.
				Handler(const char *uri);

				virtual ~Handler();

				/// @brief Handle request.
				virtual void handle(const Connection &conn, const HTTP::Request &request, const MimeType mimetype) = 0;

			};

			/// @brief Add request handler.
			/// @param uri the URI for the handler.
			/// @param handler The request handler.
			// virtual void push_back(const char *uri, const Handler &handler) = 0;

			/// @brief Remove request handler.
			/// @param uri the URI for the handler.
			// virtual void remove(const char *uri = 0;

			/// @brief Get active HTTP server.
			static Server & getInstance();
			virtual ~Server();

		};

	}

 }
