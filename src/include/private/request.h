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
 #include <udjat/tools/request.h>
 #include <udjat/tools/http/request.h>
 #include <civetweb.h>

 namespace Udjat {

	namespace CivetWeb {

		class UDJAT_PRIVATE Request : public HTTP::Request {
		private:
			const struct mg_connection *conn;
			const struct mg_request_info *info;

		public:
			Request(struct mg_connection *conn);

			const char *c_str() const noexcept override;
			String getProperty(const char *name, const char *def) const override;
			const char * query(const char *def = "") const override;
			String getArgument(const char *name, const char *def = "") const override;

		};


	}

 }

