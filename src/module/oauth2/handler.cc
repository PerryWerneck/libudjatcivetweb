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

 #define LOG_DOMAIN "oauthd"

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <private/request.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <udjat/tools/http/template.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <udjat/authentication.h>
 #include <udjat/tools/http/timestamp.h>
 #include <string>
 #include <private/oauthd.h>

 using namespace Udjat;
 using namespace std;

// #include <udjat/tools/http/oauth.h>
//  #include <udjat/tools/http/response.h>
//  #include <udjat/tools/http/exception.h>
//  #include <udjat/tools/application.h>

// #ifdef HAVE_LIBSSL

 class Response : public HTTP::Response {
 private:
	int code;

 public:
	Response(MimeType mimetype, int c, const char *message) : HTTP::Response{mimetype}, code{c} {
		failed(message);
	}

	inline int status_code() const noexcept override {
		return code;
	}

	inline void for_each(const std::function<void(const char *header_name, const char *header_value)> &call) const noexcept override {
		call("Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0");
		call("Expires", "0");
	}

 };

 static void header_send(struct mg_connection *conn, const OAuth::Context &context) {

	time_t expires = context.authentication.expires();
	int max_age = expires - time(0);

	if(Config::Value<bool>("oauth","allow-cache",true) && max_age > 0) {
		mg_response_header_add(conn, "Cache-Control", String{"private, max-age=",max_age}.c_str(),-1);
		mg_response_header_add(conn, "Expires", HTTP::TimeStamp{expires}.to_string().c_str(), -1);
	} else {
		mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
		mg_response_header_add(conn, "Expires", "0", -1);
	}

	// Setup cookie
	String cookie{
		"oauth2-session=",
		context.authentication.token().c_str(),
		"; path=/oauth2; Expires=",
		HTTP::TimeStamp::to_string(expires).c_str()
	};

	debug("Cookie='",cookie,"'");
	mg_response_header_add(conn, "Set-Cookie", cookie.c_str(),-1);
	mg_response_header_send(conn);

 }

 static int login_page(struct mg_connection *conn, const OAuth::Context &context) {

		Udjat::HTTP::Template text{"login",Udjat::MimeType::html};

        // Last, expand request arguments.
        text.expand([](const char *key, std::string &value) {

			static const struct {
				const char *key;
				const char *def;
			} cfgvals[] = {
				{ 
					"login-message", 
					_("Authorized use only. All activity is monitored for security.")
				},
				{ 
					"domain", 
					"" 
				},
				{ 
					"login-title", 
					_("Access to ${product-name}") 
				},
				{ 
					"login-button", 
					_("Sign in") 
				},
				{ 
					"user-label", 
					_("Username") 
				},
				{ 
					"password-label",
					_("Password") 
				},
				{ 
					"product-name", 
					STRINGIZE_VALUE_OF(PRODUCT_NAME) 
				},
				{ 
					"package-version", 
					PACKAGE_VERSION 
				}

			};

			debug("[[[[",key,"]]]]");

			for(const auto &cfg : cfgvals) {
				if(!strcasecmp(key,cfg.key)) {
					value = Config::Value<string>{LOG_DOMAIN,key,cfg.def};
					return true;
				}
			}

			if(!strcasecmp(key,"username")) {
				value = ""; // FIX-ME: Get username from context.
				return true;
			}

			Logger::String{"Ignoring unexpected template item '",key,"'"}.warning();

			return false;
        },false,false);

        mg_response_header_start(conn, 200);
        mg_response_header_add(conn, "Content-Type",std::to_string(MimeType::html),-1);
        mg_response_header_add(conn, "Content-Length", std::to_string(text.size()).c_str(), -1);
        header_send(conn,context);

        mg_write(conn, text.c_str(), text.size());

        return 200;

 }

 static int redirect(struct mg_connection *conn,const OAuth::Context &context) {
	mg_response_header_start(conn, 303);
	mg_response_header_add(conn, "Location",context.location.c_str(),context.location.size());
	mg_response_header_add(conn, "Content-Length", "0", -1);
	header_send(conn,context);
	return 303;
 }

 int oauthWebHandler(struct mg_connection *conn, void *) {

	OAuth::Context context;
	CivetWeb::Request request{conn};
	MimeType mimetype{request.mimetype()};

	request.pop();	// Remove '/oauth2'

	try {

		if(!*request.path()) {
			Logger::String{"Empty html request, sending login page"}.info("oauth2");
			context.authentication.set(HTTP::Authentication::LoginPage);	
			return login_page(conn,context);
		}

		// Restore authentication.
		context.authentication.token(request.cookie("oauth2-session").c_str());

		debug("--------------- Checking for options ---------------");

		// switch(request.select("signin",nullptr)) {
		// case 0: // signin
		// 	debug("---> signin");
		// 	if(OAuth::signin(request,context)) {
		// 		// Signin failed.
		// 		return login_page(conn,context);
		// 	}
		// 	return redirect(conn,context);

		// default:
		// 	throw runtime_error("Invalid request");
		// }

		throw runtime_error("Incomplete");

	} catch(const std::exception &e) {

		return ::send(conn,::Response{mimetype,500,e.what()});

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

