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
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <private/request.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>

#ifdef HAVE_LIBSSL

 /*
 static int login_page(struct mg_connection *conn, CivetWeb::Request &request, AuthenticationToken token, const char *login_message) {

#ifdef DEBUG
	String text{Application::DataFile{"./templates/login.html"}.load()};
#else
	String text{Application::DataFile{"templates/www/login.html"}.load()};
#endif // DEBUG

	// TODO: Expand common values

	// Last, expand request arguments.
	text.expand([request,login_message](const char *key, std::string &value) {

		if(!strcasecmp(key,"login_message")) {
			value = login_message;
		} else {
			value = request.getArgument(key);
		}
		return true;
	});

	mg_response_header_start(conn, 200);
	mg_response_header_add(conn, "Content-Type",std::to_string(MimeType::html),-1);
	mg_response_header_add(conn, "Content-Length", std::to_string(text.size()).c_str(), -1);
	header_send(conn,token);

	mg_write(conn, text.c_str(), text.size());

	return 200;

 }
 */

 static void header_send(struct mg_connection *conn, const OAuth::Token &token) {

	int max_age = token.expiration_time - time(0);

	if(Config::Value<bool>("oauth","allow-cache",true) && max_age > 0) {
		mg_response_header_add(conn, "Cache-Control", String{"private, max-age=",max_age}.c_str(),-1);
		mg_response_header_add(conn, "Expires", HTTP::TimeStamp{token.expiration_time}.to_string().c_str(), -1);
	} else {
		mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
		mg_response_header_add(conn, "Expires", "0", -1);
	}

	// Setup cookie
	string cookie{"session-data="};
	cookie += token.cookie;
	cookie += "; path=/; Expires=";
	cookie += HTTP::TimeStamp::to_string(token.expiration_time).c_str();

	debug("Cookie='",cookie,"'");
	mg_response_header_add(conn, "Set-Cookie", cookie.c_str(),-1);

	mg_response_header_send(conn);

 }

 static int login_page(struct mg_connection *conn, CivetWeb::Request &request, const OAuth::Token &token, const char *login_message) {

#ifdef DEBUG
        String text{Application::DataFile{"./templates/login.html"}.load()};
#else
        String text{Application::DataFile{"templates/www/login.html"}.load()};
#endif // DEBUG

        // TODO: Expand common values

        // Last, expand request arguments.
        text.expand([request,login_message](const char *key, std::string &value) {

			if(!strcasecmp(key,"login_message")) {
				value = login_message;
			} else {
				value = request.getArgument(key);
			}
			return true;
        });

        mg_response_header_start(conn, 200);
        mg_response_header_add(conn, "Content-Type",std::to_string(MimeType::html),-1);
        mg_response_header_add(conn, "Content-Length", std::to_string(text.size()).c_str(), -1);
        header_send(conn,token);

        mg_write(conn, text.c_str(), text.size());

        return 200;

 }


 int oauthWebHandler(struct mg_connection *conn, void *) {

	CivetWeb::Request request{conn};

	int code = 500;				///< @brief The HTTP return code.
	OAuth::Token response;		///< @brief The HTTP response string.

	try {

		debug("--------------------------------------------");

		// Check for operation.
		switch(request.select("authorize","login","signin",nullptr)) {
		case 0:	// Authorize
			debug("---> authorize");
			code = OAuth::authorize(request,response);
			if(code == 303) {
				// Redirect to login page
				String target{"login?",request.query()};
				mg_response_header_start(conn, 303);
				mg_response_header_add(conn, "Location",target.c_str(),target.size());
				mg_response_header_add(conn, "Content-Length", "0", -1);
				header_send(conn,response);
				return 303;
			}
			break;

		case 1:	// Login
			debug("---> login");
			OAuth::User{request}.get(response);
			return login_page(conn,request,response,"");

		case 2:	// signin
			debug("---> signin");
			{
				OAuth::User user{request};
				string message;
				if(!user.authenticate(request,message)) {
					Logger::String{request.address().c_str()," authentication failed: ",message.c_str()}.error("oauth2");
					user.get(response);
					return login_page(conn,request,response,message.c_str());
				}
				Logger::String{request.address().c_str()," authentication granted: ",message.c_str()}.info("oauth2");
			}
			break;

		default:
			code = 400;
			Logger::String message{"Invalid request: '",request.path(),"'"};
			message.error("oauth2");
			response.message.assign(message);
		}

	} catch(const exception &e) {

		code = 500;
		response.message = e.what();

	} catch(...) {

		code = 500;
		response.message = _("Unexpected error");

	}

	mg_send_http_error(conn, code, response.message.c_str());
	return code;

 }

 #endif // HAVE_LIBSSL

