/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Declares a SSL key pair singleton for authentication.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/string.h>
 #include <mutex>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API KeyPair {
		private:
			std::mutex guard;
			void *key = nullptr;
			KeyPair();

		public:
			static KeyPair & getInstance();
			~KeyPair();

			inline operator bool() const noexcept {
				return (bool) key;
			}

			/// @brief Build a new key pair.
			void reset();

			/// @brief Encrypt string.
			/// @return Base64 with the encrypted value.
			String encrypt(const char *str);

		};


	}

 }
