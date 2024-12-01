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
 #include <udjat/module/abstract.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/response.h>
 #include <vector>
 
 #include <civetweb.h>

 namespace Udjat {

	namespace CivetWeb {

		class UDJAT_API Service : public Udjat::Service, public HTTP::Server, private Interface::Factory {
		private:

			static Service *instance;

			static int request_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept;

			class Interface : public Udjat::Interface, public std::vector<Udjat::Interface::Handler> {
			private:
				const char *path = nullptr;
				
			public:

				Interface(const XML::Node &node, const char *path);
				virtual ~Interface();

				inline const char * c_str() const {
					return path;
				}

				void build_handlers(const XML::Node &node);
				
				void call(const char *method, HTTP::Request &request, HTTP::Response &response);
			};

			std::vector<Interface> interfaces;

		protected:
			struct mg_context *ctx = nullptr;

		public:

			static Service & get_instance();

			Service(const ModuleInfo &info, const pugi::xml_node &node);
			Service(const ModuleInfo &info, const char *name = "httpd");

			virtual ~Service();

			void start() noexcept override;
			void stop() noexcept override;

			bool push_back(HTTP::Handler *handler) override;
			bool remove(HTTP::Handler *handler) override;

			Udjat::Interface & InterfaceFactory(const XML::Node &node) override;

		};

	}

 }
