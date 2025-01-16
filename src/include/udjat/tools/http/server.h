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
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/interface.h>
 #include <map>

 namespace Udjat {

	namespace HTTP {

		class Handler;
		
		class UDJAT_API Server : private Interface::Factory {
		private:
			static Server *instance;

		protected:
			unsigned int apiver;	///< @brief The default api version.

			class Handler : public Udjat::Interface::Handler {
			private:
				const HTTP::Method method;	///< @brief The HTTP request method for this handler.

			public:

				Handler(const HTTP::Method m, const char *name) : Udjat::Interface::Handler{name}, method{m} {
				}

				Handler(const HTTP::Method m, const XML::Node &node) : Udjat::Interface::Handler{node}, method{m} {
				}

				Handler(const XML::Node &node);

				bool operator==(const HTTP::Request &request) const;

			};
			
			class Interface : public Udjat::Interface, public std::vector<HTTP::Server::Handler> {
			private:
				const char *path = nullptr;
				
			public:

				Interface(const XML::Node &node, const char *path);
				virtual ~Interface();

				inline const char * c_str() const {
					return path;
				}

				void call(HTTP::Request &request, HTTP::Response &response);

				bool push_back(const XML::Node &node, std::shared_ptr<Action> action) override;
				Handler & push_back(const XML::Node &node) override;

			};

			std::vector<Interface> interfaces;

			Server(const char *name = "web");
			Server(const XML::Node &node);

			Udjat::Interface & InterfaceFactory(const XML::Node &node) override;

			/// @brief Execute API call.
			/// @param interface The interface name.
			/// @param request The request data.
			/// @param response The response data.
			/// @return The http return code
			/// @retval 0 No error, result is in response.
			int call(const char *intf, Request &request, Response &response);

		public:

			/// @brief Add request handler.
			/// @param uri the URI for the handler.
			/// @param handler The request handler.
			virtual bool push_back(HTTP::Handler *handler) = 0;

			/// @brief Remove request handler.
			/// @param uri the URI for the handler.
			virtual bool remove(HTTP::Handler *handler) = 0;

			/// @brief Get active HTTP server.
			static Server & getInstance();
			virtual ~Server();

		};

	}

 }
