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
 #include <string>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Image : public std::string {
		public:

			/// @param name Icon name.
			Image(const char *name);

			Image(const std::string &name) : Image(name.c_str()) {
			}

			template <typename T>
			Image(const T &name) : Image(std::to_string(name)) {
			}

			inline operator bool() const {
				return !empty();
			}

		};
	}

 }
