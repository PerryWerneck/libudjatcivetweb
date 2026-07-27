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
 #include <udjat/module.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/properties.h>
 #include <functional>
 #include <vector>
 
 #include <civetweb.h>

 namespace Udjat {

	namespace CivetWeb {

		class UDJAT_API Service : public Udjat::Service {
		private:

			static int default_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept;

			static int request_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept;

			/// @brief Handle /icon/ requests
			static int icon_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept;

			/// @brief Handle /theme/ requests
			static int theme_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept;

			/// @brief Handle /PRODUCT_NAME/ requests
			// static int product_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept;

			/// @brief Handle /image/ requests.
			static int image_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept;

			/// @brief Handle favicon requests.
			static int favicon_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept;

			static int user_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept;

			static int auth_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept;

		protected:
			struct mg_context *ctx = nullptr;

		public:

			Service(const Udjat::Properties &props);
			Service(const char *name = "httpd", const char *description = nullptr);

			virtual ~Service();

			inline const char *name() const noexcept {
				return Udjat::Service::name();
			}

			void start() noexcept override;
			void stop() noexcept override;

		};

	}

 }
