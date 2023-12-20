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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/timestamp.h>

 #ifdef _WIN32
	#include <windows.h>
 #else
	#include <sys/types.h>
	#include <pwd.h>
	#include <arpa/inet.h>
 #endif // _WIN32

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Request : public Udjat::Request {
		public:

			#pragma pack(1)
			/// @brief Authentication token
			struct Token {
				uint8_t type = 0x10;
				uint16_t scope = 0x000F;
				time_t expiration_time = 0;
				uint64_t uid = (uint64_t) -1;
				char username[40] = "";	///< @brief The user name
#ifdef _WIN32

#else
				union {
					in_addr_t v4;
					struct in6_addr v6;
				} ip;
#endif // _WIN32

			};
			#pragma pack()

			Request(const char *path = "", HTTP::Method m = HTTP::Get);

			Request(const char *path, const char * method) : Request{path,HTTP::MethodFactory(method)} {
			}

			const char *c_str() const noexcept override;
			bool cached(const Udjat::TimeStamp &timestamp) const override;

			std::string exec(const MimeType mimetype);

			/// @brief Get Authentication token.
			/// @return true if the token has valid authentication.
			bool get(Request::Token &token) const noexcept;

			bool authenticated() const noexcept override;

			/// @brief The client address.
			virtual String address() const;

			/// @brief HTTP cookie.
			virtual String cookie(const char *name) const;

		};


	}

 }

