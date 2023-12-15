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
  * @brief Implements the pubkey output.
  *
  */

 // References:

 //	https://www.tutorialspoint.com/oauth2.0/oauth2.0_obtaining_an_access_token.htm

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <udjat/tools/logger.h>
 #include <private/request.h>

 using namespace std;

 int oauthWebHandler(struct mg_connection *conn, void *) {

#ifdef HAVE_LIBSSL

	try {

		CivetWeb::Request request{mg_get_request_info(conn)};

		debug("Authentication path: '",request.path(),"'");

		switch(request.select("authorize","login",nullptr)) {
		case 0:	// Authorize
			{
				String client_id{request.getArgument("client_id")};
				String redirect_uri{request.getArgument("redirect_uri")};
				String response_type{request.getArgument("response_type")};

				debug("client_id='",client_id,"'");
				debug("redirect_uri='",redirect_uri,"'");
				debug("response_type='",response_type,"'");

				if(client_id.empty() || redirect_uri.empty() || response_type.empty()) {
					mg_send_http_error(conn, 400, "Invalid request");
					return 400;
				}

				// TODO: Check client ID.

				// TODO: Create session

				// Redirect to login page.
				String target{"/oauth2/login?",request.query()};
				mg_response_header_start(conn, 303);
				mg_response_header_add(conn, "Location",target.c_str(),target.size());
				mg_response_header_add(conn, "Content-Length", "0", -1);
				mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
				mg_response_header_add(conn, "Expires", "0", -1);

				mg_response_header_send(conn);

				return 303;
			}
			break;

		case 1:	// Login
			{

			}
			break;

		default:
			mg_send_http_error(conn, 400, "Invalid request: '%s'", request.path());
			return 400;
		}



	} catch(const exception &e) {

		mg_send_http_error(conn, 500, "%s", e.what());
		return 500;

	} catch(...) {

		mg_send_http_error(conn, 500, "%s", _("Unexpected error"));
		return 500;

	}

#endif // HAVE_LIBSSL

	mg_send_http_error(conn, 404, _("Not available"));
	return 404;

 }
