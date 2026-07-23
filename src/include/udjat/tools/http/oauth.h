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
 #include <udjat/tools/string.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/http/authentication.h>
 #include <udjat/tools/http/statuscodes.h>

 namespace Udjat {

	namespace OAuth {

		class UDJAT_API Context : public HTTP::Authentication, public Abstract::Object {
		private:

			/// @brief Handle callback from oauth server.
			/// @return The HTTP status code.
			HTTP::StatusCode callback();

		protected:
			String path;
			String uri;			///< @brief The URI who originated the authentication request.
			String code;
			bool apicall = false;
			uint16_t sequencial = 0;

 			/// @brief Sent HTTP header.
 			virtual void send_header(bool cookie = true) const = 0;

			/// @brief Do a POST request.
			virtual String post(const char *url, const char *payload) const = 0;

			/// @brief Send template response.
			/// @param code The HTTP status code
			/// @param action The action name (for template expansion)
			/// @param tmplt The template name.
			/// @return The HTTP status code.
			HTTP::StatusCode send_template(HTTP::StatusCode code, const char *action, const char *tmplt);

			inline HTTP::StatusCode send_template(HTTP::StatusCode code, const char *tmplt) {
				return send_template(code,"",tmplt);
			}

			/// @brief Send HTML response using current context.
			/// @param tmplt The template name.
			/// @param code The HTTP status code.
			/// @return The HTTP status code.
			virtual HTTP::StatusCode send_html_response(HTTP::StatusCode code, const char *text) const = 0;

 			/// @brief Send redirect response.
			/// @return The HTTP status code.
 			virtual HTTP::StatusCode send_redirect_response(const char *location, bool cookie = true) const = 0;

			/// @brief Format and send error page.
			/// @param code The http status code.
			/// @param message The message to user.
			/// @param body The message body.
			/// @return The HTTP status code.
			virtual HTTP::StatusCode failed(HTTP::StatusCode code, const char *message, const char *body = "") const;

			/// @brief Authentication complete, redirect to main page.
			/// @return The HTTP status code.
			HTTP::StatusCode authenticated();

		public:

			Context(const char *path = nullptr);
			virtual ~Context();

			bool get_property(const char *key, Udjat::Value &value) const override;

			inline bool empty() const noexcept {
				return path.empty();
			}

			/// @brief Handle authentication requests.
			/// @return The HTTP status code.
			HTTP::StatusCode handle();

			/// @brief Pop one element from path.
			/// @return 
			String pop();

			/// @brief Start authentication flow.
			/// @param api True if the request started from an API call.
			/// @param target URL to redirect when the flow finished.
			/// @return HTTP error code to forward.
			HTTP::StatusCode authenticate(const char *target = "");

			/// @brief Run 'signin'
			/// @param request The request info
			/// @param context The current context.
			/// @return 0 if the user was authenticated.
			/// @retval EPERM Access denied.
			UDJAT_API int signin();

		};

 	}

 }
