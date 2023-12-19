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
  * @brief Dclare OAuth2 objects.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/http/request.h>
 #include <civetweb.h>
 #include <map>
 #include <string>

 #ifdef _WIN32
	#include <windows.h>
 #else
	#include <sys/types.h>
	#include <pwd.h>
	#include <arpa/inet.h>
 #endif // _WIN32

 namespace Udjat {

	namespace OAuth {

		struct Token {
			std::string cookie;			///< @brief The authentication cookie.
			std::string message;		///< @brief The Message for client.
			time_t expiration_time;		///< @brief The expiration time.

		};

		UDJAT_API int authorize(HTTP::Request &request, Token &token);

		/// @brief OAuth2 API client
		class UDJAT_API Client {
		private:

			#pragma pack(1)
			struct Cookie {
				uint8_t type;
				time_t expiration_time = 0;
#ifdef _WIN32
#else
				uint32_t uid = (uint32_t) (-1);
#endif // _WIN32
				union {
					in_addr_t v4;
					struct in6_addr v6;
				} ip;

			} data;
			#pragma pack()

		public:
			Client(HTTP::Request &request);
			~Client();

			/// @brief Get Token
			void get(Token &token);

			/// @brief Get authentication token for client.
			String encript();

			/// @brief Validate authentication token for client.
			bool decript(const char *str);

		};

		/// @brief OAuth2 user
		class UDJAT_API User {
		private:

			#pragma pack(1)
			struct Token {
				uint8_t type;
				time_t expiration_time = 0;
				uint16_t scope = 0x0001;
#ifdef _WIN32

#else
				unsigned int uid = (unsigned int) -1;
#endif // _WIN32
				union {
					in_addr_t v4;
					struct in6_addr v6;
				} ip;

			} data;
			#pragma pack()

			void set(HTTP::Request &request);

		public:
			User();
			User(HTTP::Request &request);
			~User();

			/// @brief Authenticate user.
			bool authenticate(HTTP::Request &request, std::string &message);

			/// @brief Get Token
			void get(OAuth::Token &token);

			/// @brief Encript user authentication token.
			String encript();

			/// @brief Decript user authentication token.
			bool decript(const char *str);

		};


	}

 }
