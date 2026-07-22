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
 #include <udjat/tools/string.h>

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
			const HTTP::Method http_method = HTTP::Get;

		protected:

			/// @brief Is this request an API call?
			bool api_call = false;

		public:

			Request(const char *path = "", HTTP::Method method = HTTP::Get) : Udjat::Request{path}, http_method{method} {
			}

			Request(const char *path, const char *method) : Request{path,HTTP::MethodFactory(method)} {
			}

			virtual ~Request();

			/// @brief Get connection who originated this request.
			virtual HTTP::Connection & connection() const = 0;

			/// @brief Get HTTP header for this request.
			/// @param name Header name
			/// @param def Default value if not found (nullptr to launch exception if not found);
			/// @return Header value, or 'def' if not found.
			virtual const char * header(const char *name, const char *def = "") const noexcept = 0;

			virtual Udjat::String cookie(const char *name, const char *def = nullptr) const;

			bool get_property(const char *key, Udjat::Value &value) const override;

			inline HTTP::Method method() const noexcept {
				return http_method;
			}

			inline bool operator==(HTTP::Method method) const noexcept {
				return http_method == method;
			}

			inline bool operator!=(HTTP::Method method) const noexcept {
				return http_method != method;
			}

			bool cached(const Udjat::TimeStamp &timestamp) const override;

			/// @brief The client address.
			inline Udjat::String address() const {
				return connection().address();
			}

			/// @brief The request URI.
			virtual Udjat::String uri() const = 0;

			/// @brief The request mime-type.
			MimeType mimetype() const noexcept;

			bool for_each(const std::function<bool(const char *name, const char *value)> &call) const override;

		};


	}

 }

