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
 #include <udjat/tools/http/value.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <map>
 #include <string>

 #ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
	#include <in6addr.h>
 #else
	#include <sys/types.h>
	#include <pwd.h>
	#include <arpa/inet.h>
 #endif // _WIN32

 namespace Udjat {

	namespace OAuth {

		struct Context {
			String token;				///< @brief The authentication token.
			String message;				///< @brief The Message for client.
			String location;			///< @brief The new location.
			time_t expiration_time;		///< @brief The expiration time.
		};

		UDJAT_API int authorize(HTTP::Request &request, Context &context);

		/// @brief Run 'signin'
		/// @param request The request info
		/// @param context The current context.
		/// @return 0 if the user was authenticated.
		/// @retval EPERM Access denied.
		UDJAT_API int signin(HTTP::Request &request, Context &context);

		/// @brief Get access token.
		/// @param request The request info
		/// @param context The current context.
		/// @param response The response data.
		/// @return 0 if the response was set.
		/// @retval EPERM The authentication code is invalid.
		UDJAT_API int access_token(HTTP::Request &request, Context &context, HTTP::Value &response);

		/// @brief OAuth2 API client
		class UDJAT_API Client {
		private:

			#pragma pack(1)
			struct Cookie {
				uint8_t type = 0;
				time_t expiration_time = 0;
				uint32_t scopes = 7;
#ifdef _WIN32
				union {
					in_addr v4;		// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/ns-winsock2-in_addr
					in6_addr v6;	// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms738560(v=vs.85)
				} ip;
#else
				uint32_t uid = (uint32_t) (-1);
				union {
					in_addr_t v4;
					struct in6_addr v6;
				} ip;
#endif // _WIN32

				inline void clear() noexcept {
					type = 0;
					expiration_time = 0;
					scopes = 7;
					memset(&ip,0,sizeof(ip));
				}

			} data;
			#pragma pack()

		public:
			Client(HTTP::Request &request);
			~Client();

			/// @brief Update context.
			void get(Context &context);

			/// @brief Get authentication token for client.
			String encrypt();

			/// @brief Validate authentication token for client.
			bool decrypt(const char *str);

			inline time_t expires() const noexcept {
				return data.expiration_time;
			}

		};

		/// @brief OAuth2 user
		class UDJAT_API User {
		private:

			#pragma pack(1)
			struct Token {
				uint8_t type = 0;
				time_t expiration_time = 0;
				uint16_t scope = 0x000F;
				char username[40] = "";
#ifdef _WIN32
				union {
					in_addr v4;		// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/ns-winsock2-in_addr
					in6_addr v6;	// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms738560(v=vs.85)
				} ip;
#else
				unsigned int uid = (unsigned int) -1;
				union {
					in_addr_t v4;
					struct in6_addr v6;
				} ip;
#endif // _WIN32

				inline void clear() noexcept {
					type = 0;
					expiration_time = 0;
					scope = 0x000F;
					memset(username,0,sizeof(username));
					memset(&ip,0,sizeof(ip));
				}

			} data;
			#pragma pack()

			void set(HTTP::Request &request);

		public:
			User();
			User(HTTP::Request &request);
			~User();

			inline operator bool() const noexcept {
#ifdef _WIN32
				return false;
#else
				return data.uid != (unsigned int) -1;
#endif // _WIN32
			}

			/// @brief Authenticate user.
			bool authenticate(HTTP::Request &request, std::string &message);

			/// @brief Set user info from authentication code.
			/// @param value The authentication code.
			/// @return true if the code is valid and user was updated.
			/// @retval true Got user information from authentcation code.
			/// @retval false The authentication code is not valid.
			bool code(const char *value);

			/// @brief Get authentication code.
			/// @return The authentication code for the user.
			String code();

			/// @brief Update context with user data
			void get(OAuth::Context &context);

			/// @brief Encript user authentication token.
			String encrypt();

			/// @brief Decript user authentication token.
			bool decrypt(const char *str);

			inline time_t expires() const noexcept {
				return data.expiration_time;
			}

			/// @brief Update request token with user data and encrypt it.
			String encrypt(Udjat::HTTP::Request::Token &token);

			/// @brief Get user info.
			bool get(Udjat::Value &value);

			static bool get(uint64_t uid, uint16_t scope, Udjat::Value &value);

		};


	}

 }
