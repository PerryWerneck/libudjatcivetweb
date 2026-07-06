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
 #include <udjat/tools/string.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/http/authentication.h>
 
 namespace Udjat {

	namespace OAuth {

		class UDJAT_API Context : public HTTP::Authentication, public Abstract::Object {
		protected:
			struct {
				String message;											///< @brief The Message for client.
				String body;
			} status;
			
			String path;
			String redirect_uri;
			String code;

 			/// @brief Sent HTTP header.
 			virtual void send_header() const = 0;

		public:

			Context(const char *path);
			virtual ~Context();

			bool getProperty(const char *key, std::string &value) const override;

			inline bool empty() const noexcept {
				return path.empty();
			}

			inline void message(const char *msg, const char *body = "") noexcept {
				status.message = msg;
				status.body = body;
			}

			/// @brief Handle authentication requests.
			/// @return The HTTP status code.
			int handle();

			/// @brief Send template response.
			/// @param code The HTTP status code
			/// @param action The action name (for template expansion)
			/// @param tmplt The template name.
			/// @return The HTTP status code.
			int send_template(int code, const char *action, const char *tmplt);

			/// @brief Send HTML response using current context.
			/// @param tmplt The template name.
			/// @param code The HTTP status code.
			/// @return The HTTP status code.
			virtual int send_html_response(int code, const char *text) const = 0;

 			/// @brief Send redirect response.
			/// @return The HTTP status code.
 			virtual int send_redirect_response(const char *location) const = 0;

			/// @brief Pop one element from path.
			/// @return 
			String pop();

			/// @brief Run callback from oauth server.
			/// @return The HTTP status code.
			int callback();

			/// @brief Run 'signin'
			/// @param request The request info
			/// @param context The current context.
			/// @return 0 if the user was authenticated.
			/// @retval EPERM Access denied.
			UDJAT_API int signin();

		};

 	}

 }
