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
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/keypair.h>
 #include <private/request.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/timestamp.h>

 using namespace std;

 int oauthWebHandler(struct mg_connection *conn, void *) {

#ifdef HAVE_LIBSSL

	/// @brief Authentication token
	#pragma pack(1)
	struct AuthenticationToken {

		unsigned char type = 1;

		/// @brief The authenticated user id.
		unsigned int id = (unsigned int) -1;

		/// @brief Login expiration time.
		time_t expiration_time = 0;

	};
	#pragma pack()

	/// @brief Validate login arguments.
	struct LoginValidator {
		String client_id;
		String redirect_uri;
		String response_type;

		LoginValidator(CivetWeb::Request &request)
			: client_id{request.getArgument("client_id")},
				redirect_uri{request.getArgument("redirect_uri")},
				response_type{request.getArgument("response_type")} {

			debug("client_id='",client_id,"'");
			debug("redirect_uri='",redirect_uri,"'");
			debug("response_type='",response_type,"'");
		}

		operator bool() {
			return !(client_id.empty() || redirect_uri.empty() || response_type.empty());
		}

	};


	try {

		debug("--------------------------------------------");
		CivetWeb::Request request{conn};

		debug("Authentication path: '",request.path(),"'");

		switch(request.select("authorize","login","signin",nullptr)) {
		case 0:	// Authorize
			{
				if(!LoginValidator{request}) {
					mg_send_http_error(conn, 400, "Invalid request");
					return 400;
				}

				// TODO: Check if already authenticated.

				// TODO: Check client ID.

				// TODO: Create session.

				// Redirect to login page.
				String target{"login?",request.query()};
				mg_response_header_start(conn, 303);
				mg_response_header_add(conn, "Location",target.c_str(),target.size());
				mg_response_header_add(conn, "Content-Length", "0", -1);
				mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
				mg_response_header_add(conn, "Expires", "0", -1);

				// Create authentication token
				{
					AuthenticationToken token;
					memset(&token,0,sizeof(token));
					token.expiration_time = time(0) + Config::Value<time_t>("oauth","max-age",86400);

					string cookie{"session-data="};
					cookie += Udjat::HTTP::KeyPair::getInstance().encrypt(&token,sizeof(token));
					cookie += "; path=/; Expires=";
					cookie += HTTP::TimeStamp::to_string(token.expiration_time).c_str();

					debug("Cookie='",cookie,"'");

					mg_response_header_add(conn, "Set-Cookie", cookie.c_str(),-1);
				}

				mg_response_header_send(conn);

				return 303;
			}
			break;

		case 1:	// Login
			{

				if(!LoginValidator{request}) {
					mg_send_http_error(conn, 400, "Invalid request");
					return 400;
				}

#ifdef DEBUG
				String text{Application::DataFile{"./templates/login.html"}.load()};
#else
				String text{Application::DataFile{"templates/www/login.html"}.load()};
#endif // DEBUG

				// TODO: Expand common values

				// Last, expand request arguments.
				text.expand([request](const char *key, std::string &value) {
					value = request.getArgument(key);
					return true;
				});

				mg_response_header_start(conn, 200);
				mg_response_header_add(conn, "Content-Type",std::to_string(MimeType::html),-1);
				mg_response_header_add(conn, "Content-Length", std::to_string(text.size()).c_str(), -1);
				mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
				mg_response_header_add(conn, "Expires", "0", -1);

				mg_response_header_send(conn);
				mg_write(conn, text.c_str(), text.size());

				return 200;
			}
			break;

		case 2:	// signin
			{
				debug("Received signin response");

				if(!LoginValidator{request}) {
					mg_send_http_error(conn, 400, "Invalid request");
					return 400;
				}

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
