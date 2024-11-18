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
 #include <udjat/tools/response.h>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Response : public Udjat::Response {
		public:
			Response(Udjat::MimeType mimetype) : Udjat::Response{mimetype} {
			}

			int status_code() const noexcept;

			std::string to_string() const noexcept override;

			/// @brief Enumerate headers.
			virtual void for_each(const std::function<void(const char *header_name, const char *header_value)> &call) const noexcept;

		};

	}	
 }
 


/*
 #include <udjat/tools/http/value.h>
 #include <map>

 namespace Udjat {

	namespace HTTP {


	}

 }
*/

