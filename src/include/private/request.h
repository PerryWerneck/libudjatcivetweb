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
 #include <udjat/tools/http/authentication.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/http/request.h>
 #include <civetweb.h>
 #include <udjat/tools/civetweb/connection.h>
 #include <map>

 namespace Udjat {

	namespace CivetWeb {

		class UDJAT_PRIVATE Request : public HTTP::Request {
		private:
			CivetWeb::Connection conn;

		public:

			/// @brief Build request, process path and apiver.
			/// @param conn The connection for this request.
			Request(CivetWeb::Connection &conn);

			const char * query(const char *def = "") const override;

			// bool for_each(const std::function<bool(const char *name, const char *value)> &call) const override;
			bool get_property(const char *key, Udjat::Value &value) const override;

			const char * header(const char *name) const noexcept override;

			/// @brief The client address.
			Udjat::String address() const override;

			/// @brief The request URI.
			Udjat::String uri() const override;

			/// @brief redirect
			int redirect(const char *location) const;

			Udjat::String cookie(const char *name) const override;

		};


	}

 }

