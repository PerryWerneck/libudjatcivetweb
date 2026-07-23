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
  * @brief Implements simple oauth2 authenticator.
  *
  */

 // References:

 //	https://www.tutorialspoint.com/oauth2.0/oauth2.0_obtaining_an_access_token.htm
 // https://www.freebsd.org/doc/en/articles/pam/pam-essentials.html


 #include <config.h>

 #undef LOG_DOMAIN
 #define LOG_DOMAIN "oauthd"
 #include <udjat/tools/logger.h>

 #include <udjat/defs.h>
 #include <private/module.h>
 #include <private/oauth.h>
 #include <private/request.h>
 #include <udjat/tools/intl.h>
 
 using namespace Udjat;
 
 int oauthWebHandler(struct mg_connection *conn, void *) {

	CivetWeb::OAuthContext context{conn};
	try {

		return context.handle();

	} catch(const std::exception &e) {

		Logger::String{e.what()}.error();

		return (int) context.send_html_response(
			HTTP::SystemError,
			_("An unexpected error occurred during the login process.")
		);

	}

 }

