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
 #include <udjat/tools/request.h>
 #include <private/request.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <udjat/tools/http/template.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <udjat/authentication.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/application.h>
 #include <string>
 #include <private/oauthd.h>

 using namespace Udjat;
 using namespace std;

 OAuth::Context::Context(struct mg_connection *c) : conn{c}, path{mg_get_request_info(c)->local_uri} {
 
	while(path[0] == '/') {
		path.erase(0,1);
	}

	debug("Request path: '",path.c_str(),"'");

	// Get session cookie
	{
		cookie_name = String{"oauth-session"};
		const char *cookie = mg_get_header(conn,"Cookie");
		if(cookie && *cookie) {
			char buffer[4096];
			int length = mg_get_cookie(cookie,cookie_name.c_str(),buffer,4095);
			if(length > 0) {
				buffer[length] = 0;
				Authentication::token(buffer);
			}
		}
	}

 }

 String OAuth::Context::pop() {

	if(path.empty()) {
		return "";
	}

	auto pos = path.find('/');
	if(pos == string::npos) {
		String rc = path;
		path.clear();
		return rc;
	}

	String rc = path.substr(0, pos);
	path.erase(0,pos+1);

	return rc;
 }

 int oauthWebHandler(struct mg_connection *conn, void *) {

	OAuth::Context context{conn};
	context.pop();	// Remove '/oauth2'

	try {

		if(context.path.empty()) {
			Logger::String{"Empty html request, sending login page"}.trace();
			context.action = "signin";
			context.set(HTTP::Authentication::LoginPage);
			return context.send_html_response("login");
		}

		debug("--------------- Checking for options ---------------");
		String requested_action = context.pop();

		debug("Requested action: '",requested_action.c_str(),"'");

		// switch(context.pop().select("siXgnin",nullptr)) {
		// case 0: // signin
		// 	debug("---> signin");
		// 	if(context.signin()) {
		// 		// Signin failed.
		// 		throw runtime_error("Incomplete");
		//		message = _("Access denied");
		// 		return context.send_html_response("login");
		// 	}
		// 	throw runtime_error("Incomplete");
		// 	// return context.redirect();	// Redirect, signin already set the destination.

		// }

		context.body = Logger::Message{_("The requested action '{}' is not available in this server"), requested_action.c_str()}.c_str();
		return context.send_html_response("error",404);

	} catch(const std::exception &e) {

		Logger::String{e.what()}.error();
		context.Authentication::reset();	
		context.message = _("Internal error processing request");
		context.body = e.what();
		return context.send_html_response("error",500);

	}

	// try {


	// 	debug("------------------> '",request.path(),"'");

	// 	// Check for operation.
	// 	switch(request.select("authorize","login","signin","access_token","userinfo",nullptr)) {
	// 	case 0:	// Authorize
	// 		debug("---> authorize");
	// 		code = OAuth::authorize(request,context);
	// 		if(code == 303) {
	// 			return redirect(conn,context);
	// 		}
	// 		break;

	// 	case 1:	// Login
	// 		debug("---> login");
	// 		OAuth::User{request}.get(context);
	// 		context.message.clear();
	// 		return login_page(conn,request,context);

	// 	case 2:	// signin

	// 	case 3: // access_token
	// 		debug("---> access_token");
	// 		{
	// 			Udjat::Value response{Value::Object};

	// 			if(!OAuth::access_token(request,context,response)) {

	// 				string text{response.to_string(mimetype)};

	// 				if(!text.empty()) {

	// 					mg_response_header_start(conn, 200);
	// 					mg_response_header_add(conn, "Content-Type",std::to_string(mimetype),-1);
	// 					mg_response_header_add(conn, "Content-Length", std::to_string(text.size()).c_str(), -1);
	// 					header_send(conn,context);
	// 					mg_write(conn, text.c_str(), text.size());
	// 					return 200;

	// 				} else {

	// 					Logger::String message{"Empty response: '",request.path(),"'"};
	// 					message.error("oauth2");
	// 					code = 503;
	// 					context.message.assign(message);

	// 				}
	// 			} else {

	// 				code = 400;
	// 				context.message.assign("Access denied");

	// 			}

	// 		}
	// 		break;

	// 	case 4:	// userinfo.
	// 		{
	// 			Udjat::Value response{Value::Object};
	// 			HTTP::Request::Token token;

	// 			if(!request.get(token)) {

	// 				Logger::String message{"Access denied - Invalid user"};
	// 				message.error("oauth2");
	// 				code = 401;
	// 				context.message.assign(message);

	// 			} else {

	// 				OAuth::User::get(token.uid,token.scope,response);
	// 				if(response.empty()) {
	// 					Logger::String message{"Empty response from user backend"};
	// 					message.error("oauth2");
	// 					code = 503;
	// 					context.message.assign(message);
	// 				} else {
	// 					string text{response.to_string(mimetype)};

	// 					debug("Response:\n",text.c_str());

	// 					mg_response_header_start(conn, 200);
	// 					mg_response_header_add(conn, "Content-Type",std::to_string(mimetype),-1);
	// 					mg_response_header_add(conn, "Content-Length", std::to_string(text.size()).c_str(), -1);
	// 					header_send(conn,context);
	// 					mg_write(conn, text.c_str(), text.size());
	// 					return 200;
	// 				}

	// 			}

	// 		}
	// 		break;

	// 	default:
	// 		code = 404;
	// 		Logger::String message{"Unexpected request"};
	// 		message.error("oauth2");
	// 		context.message.assign(message);
	// 	}

	// } catch(const exception &e) {

	// 	code = 500;
	// 	context.message = e.what();

	// } catch(...) {

	// 	code = 500;
	// 	context.message = _("Unexpected error");

	// }

 	// return ::send(conn,Response{mimetype,code,message.c_str()});

 }

// 	debug("OAuth handler exit with error ",code);
// 	/// @brief Customized error response.


//  }

//  #endif // HAVE_LIBSSL

