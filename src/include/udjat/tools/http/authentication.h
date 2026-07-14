/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/authentication.h>
 #include <string>
 
 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Authentication : public Udjat::Authentication {
			protected:
				void token(const char *b64);

			public:

				enum Status : uint8_t {
					Undefined,
					LoginPage,
				};

				/// @brief Build an authentication from cookie.
				/// @param b64 Encrypted token.
				Authentication(const char *b64 = nullptr);

				/// @brief Get encrypted token.
				/// @return String with base 64 encrypted token.
				std::string token() const;

				/// @brief Get the authentication cookie name.
				static std::string cookie_name() noexcept;

				/// @brief Reset authentication to empty state.
				void clear() noexcept override;

				/// @brief Get expiration time.
				inline time_t expires() const noexcept {
					return expiration_time;
				}

				inline void set(const Status status) noexcept {
					current_status = status;
				}

			protected:
				time_t expiration_time = 0;
				Status current_status = Undefined;
				std::string avatar_url;
				Authentication::Role role = Authentication::None;

		};

	}

 }
