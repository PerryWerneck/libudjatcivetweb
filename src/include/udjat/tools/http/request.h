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
 #include <udjat/tools/http/connection.h>

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

	namespace HTTP {

		class UDJAT_API Request : public Udjat::Request {
		private:
			/// @brief Request method.
			const HTTP::Method method = HTTP::Get;

		protected:

			/// @brief Parse URL query into named values.
			/// @param query The query string.
			void parse_query(const char *query);

		public:

			#define TOKEN_USERNAME_LEN 40

			#pragma pack(1)
			/// @brief Authentication token
			struct Token {
				uint8_t type = 0x10;
				uint16_t scope = 0x000F;
				time_t expiration_time = 0;
				uint64_t uid = (uint64_t) -1;
				char username[TOKEN_USERNAME_LEN+1] = "";	///< @brief The user name
#ifdef _WIN32
				union {
					in_addr v4;		// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/ns-winsock2-in_addr
					in6_addr v6;	// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms738560(v=vs.85)
				} ip;
#else
				union {
					in_addr_t v4;
					in6_addr v6;
				} ip;
#endif // _WIN32

				inline void clear() noexcept {
					type = 0x10;
					scope = 0x000F;
					expiration_time = 0;
					uid = (uint64_t) -1;
					memset(username,0,sizeof(username));
					memset(&ip,0,sizeof(ip));
				}

			};
			#pragma pack()

			bool decrypt(HTTP::Request::Token &token) const;

			constexpr Request(const char *path = "", HTTP::Method m = HTTP::Get) : Udjat::Request{path}, method{m} {
			}

			Request(const char *path, const char *method) : Request{path,HTTP::MethodFactory(method)} {
			}

			virtual ~Request();

			inline operator HTTP::Method() const noexcept {
				return this->method;
			}

			inline HTTP::Method verb() const noexcept {
				return this->method;
			}

			inline bool operator==(HTTP::Method method) const noexcept {
				return this->method == method;
			}

			bool cached(const Udjat::TimeStamp &timestamp) const override;

			/// @brief Get Authentication token.
			/// @return true if the token has valid authentication.
			bool get(Request::Token &token) const noexcept;

			bool authenticated() const noexcept override;

			/// @brief The client address.
			virtual Udjat::String address() const = 0;

			/// @brief The request mime-type.
			MimeType mimetype() const noexcept;

			bool for_each(const std::function<bool(const char *name, const char *value)> &call) const override;

			/// @brief HTTP cookie.
			virtual Udjat::String cookie(const char *name) const;

			bool getProperty(const char *key, std::string &value) const override;

		};


	}

 }

