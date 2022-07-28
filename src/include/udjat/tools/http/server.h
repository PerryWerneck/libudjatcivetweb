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
			protected:
				friend class Server;
				const char * uri;
				Server *server;

				/// @brief Create a new httpd handler, insert it to server.
				/// @param server The server to receive the new handler.
				/// @param uri the URI for the handler.
				Handler(const char *uri, Server *server);

			public:
				virtual ~Handler();

				inline const char * c_str() const noexcept {
					return uri;
				}

				/// @brief Handle request.
				virtual int handle(const Connection &conn, const HTTP::Request &request, const MimeType mimetype) = 0;

			};

			/// @brief Add request handler.
			/// @param uri the URI for the handler.
			/// @param handler The request handler.
			virtual void push_back(const Handler *handler);

			/// @brief Remove request handler.
			/// @param uri the URI for the handler.
			virtual void remove(const Handler *handler);

			/// @brief Get active HTTP server.
			static Server & getInstance();
			virtual ~Server();

		};

	}

 }
