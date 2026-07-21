/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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

 // References:
 //	https://docs.github.com/pt/apps/oauth-apps/building-oauth-apps/authorizing-oauth-apps#web-application-flow


 #include <config.h>

 #ifdef LOG_DOMAIN
 	#undef LOG_DOMAIN
 #endif 
 #define LOG_DOMAIN "oauth"

 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/authentication.h>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/url.h>
 #include <private/client.h>
 #include <udjat/tools/memory.h>

 #if defined(HAVE_JSON_C)
	#include <json.h>
 #endif // HAVE_JSON_C

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	int OAuth::Context::callback() {

		debug("---> callback");
		debug("uri=",uri.c_str())
		debug("code=",code.c_str())

		if(code.empty()) {
			return failed(400,strerror(EPERM),_("Invalid response from authentication server"));
		}

		// Get access token
		String access_token;
		String token_type;
		{
			Config::Value<String> url{"authentication","get-token-url",""};
			debug("get-token-url='",url.c_str(),"'");
			url.expand(this);

			// url.expand([this](const char *key, std::string &value) {
			// 	return this->getProperty(key,value);
			// });

			if(url.empty()) {
				Logger::Message{"Missing required value for authentication attribute '{}'","get-token-url"}.error();
				return failed(400,strerror(EPERM),_("Invalid authentication engine configuration. Please check server settings."));
			}

			Config::Value<String> payload{"authentication","get-token-payload",""};
			debug("get-token-payload='",payload.c_str(),"'");
			payload.expand(this);

			// payload.expand([this](const char *key, std::string &value) {
			// 	return this->getProperty(key,value);
			// });

			HTTP::Method method = HTTP::MethodFactory(Config::Value<string>{"authentication","get-token-method","post"}.c_str());

			debug("URL: ",url.c_str());
			debug("Payload: ",payload.c_str());
			debug("Method: ",std::to_string(method));

			auto response = URL{url.c_str()}
				.process(
					method,
					payload.c_str(),
					false
				);


			debug("Got response '",response.c_str(),"'");

			if(response.empty()) {
				throw runtime_error(_("Empty response from authentication server"));
			}

			String error;
			String error_description;

			for(const String &value : response.split("&")) {

				debug("response item='",value.c_str(),"'");

				if(value.has_prefix("access_token=")) {
					access_token = value.c_str()+13;

				} else if(value.has_prefix("token_type=")) {
					token_type = value.c_str()+11;
					if(strcasecmp(token_type.c_str(),"bearer")) {
						throw runtime_error(Logger::Message(_("Unexpected token type: '{}'"),token_type.c_str()));
					}

				} else if(value.has_prefix("error=")) {
					error = value.c_str()+6;
					error.unescape();

				} else if(value.has_prefix("error_description=")) {
					error_description = value.c_str()+18;
					error_description.unescape();

				} else if(value.has_prefix("error_uri=")) {
					String val = value.c_str()+10;
					val.unescape();
					Logger::String{val.c_str()}.error();

				}
				
			}

			if(!error_description.empty()) {
				return failed(
						400,
						_("Failed to retrieve information from the authentication server."),
						error_description.c_str()
					);
			}

			if(!error.empty()) {
				return failed(
						400,
						_("Failed to retrieve information from the authentication server."),
						error.c_str()
					);
			}

			if(access_token.empty()) {
				return failed(
						400,
						_("Failed to retrieve information from the authentication server."),
						_("Empty token on authentication server response")
					);
			}

		}

		debug("access_token=",access_token.c_str());

		// Get user account
		// GET https://api.github.com/user
		// curl -H "Authorization: Bearer OAUTH-TOKEN" https://api.github.com/user
		{
			URL url{Config::Value<string>{"authentication","get-user-account"}.c_str()};
			auto handler = url.handler();
			
			try {

				if(url.empty()) {
					Logger::Message{"Missing required value for authentication attribute '{}'","get-user-account"}.error();
					return failed(
						500,
						_( "Configuration error" ),
						_( "Invalid authentication engine configuration. Please check server settings." )
					);
				}
				
				handler->header(
					URL::Handler::AUTHORIZATION,
					String{token_type.c_str()," ",
					access_token.c_str()}.c_str()
				);
				
				// Github requires user-agent matching with the registered application.
				handler->header(
					URL::Handler::USER_AGENT,
					Config::Value<string>{"authentication","user-agent",STRINGIZE_VALUE_OF(PRODUCT_NAME)}.c_str()
				);

				debug("--- Getting user info ---");
				auto response = handler->get();

				debug("Got response: '",response.c_str(),"'");
				clear();

#if defined(HAVE_JSON_C)
				auto jobj = make_handle(json_tokener_parse(response.c_str()),json_object_put);

				if(!jobj) {
					throw runtime_error(_("Error parsing authentication response"));
				}

				json_object_object_foreach(jobj.get(), key, val) {
					if(json_object_get_type(val) == json_type_string) {
						debug(key,"='",json_object_get_string(val),"'");
						if(!strcasecmp(key,"name")) {
							Authentication::name(json_object_get_string(val));
						} else if(!strcasecmp(key,"email")) {
							login(json_object_get_string(val));
						} else if(!strcasecmp(key,"avatar_url")) {
							avatar_url = json_object_get_string(val);
						}
					}
				}
#else
				throw runtime_error("Unable to process authentication response: Json parser is not available");
#endif // HAVE_JSON_C

				if(Udjat::Authentication::role() == Authentication::None) {
					
					// Not authorized.

					const char *username = Udjat::Authentication::name();
					clear();

					if(!(username && *username)) {
						Logger::Message{"Missing required value for authentication response '{}'","name"}.error();
						return failed(
							400,
							strerror(EPERM),
							_("The authentication server did not provide a username.")
						);
					} else {
						Logger::Message{"Access unauthorized for '{}'",username}.error();
						return failed(
							400,
							strerror(EPERM),
							_("Access unauthorized. Please contact your system administrator if you believe this is an error.")
						);
					}

				}

			} catch(const std::exception &e) {

				Logger::String{url.c_str()," returned  '",e.what(),"'"}.error();

				return failed(
					500,
					_("Failed to retrieve user information from the authentication server."),
					e.what()
				);

			}
		}

		// Redirect to main page.
		debug("uri=",uri.c_str())

		return authenticated();

	}


 }

