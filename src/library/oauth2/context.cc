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

 //	https://www.tutorialspoint.com/oauth2.0/oauth2.0_obtaining_an_access_token.htm
 // https://www.freebsd.org/doc/en/articles/pam/pam-essentials.html


 #include <config.h>

 #ifdef LOG_DOMAIN
 	#undef LOG_DOMAIN
 #endif 
 #define LOG_DOMAIN "oauth"

 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/template.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/url.h>
 #include <private/client.h>

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	OAuth::Context::Context(const char *p) : path{p} {
		while(path[0] == '/') {
			path.erase(0,1);
		}
		pop();	// Remove '/oauth2'

		debug("---- Build OAuth2 context for '",path.c_str(),"'");	

#ifdef DEBUG
		{
			string v;
			getProperty("client-id",v);
			getProperty("client-secret",v);
		}
#endif

	}

	OAuth::Context::~Context() {
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

	bool OAuth::Context::getProperty(const char *key, std::string &value) const {

		if(!strcasecmp(key,"message")) {
			value = this->status.message;
			return true;
		}

		if(!strcasecmp(key,"body")) {
			value = this->status.body;
			return true;
		}

		if(!strcasecmp(key,"authentication-state")) {

			size_t szBuffer = uri.size()+sizeof(uint16_t)+1;
			char * buffer[szBuffer];
			memset(buffer,0,szBuffer);

			if(!this->sequencial) {
				static uint16_t sequencial = 0;
				const_cast<OAuth::Context *>(this)->sequencial = ++sequencial;
			}

			*((uint16_t *) buffer) = this->sequencial;

			memcpy((buffer+sizeof(uint16_t)),uri.c_str(),uri.size());
			buffer[szBuffer] = 0;			

			value = encrypt(buffer,szBuffer).escape();

			return true;
		}

		if(!strcasecmp(key,"redirect-uri")) {
			value = uri;
			return true;
		}

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
				"client-id",
				STRINGIZE_VALUE_OF(PRODUCT_NAME) 
			},
			{ 
				"client-secret",
				"" 
			},
			{ 
				"package-version", 
				PACKAGE_VERSION 
			},
		};

		debug("[[[[",key,"]]]]");

		for(const auto &cfg : cfgvals) {
			if(!strcasecmp(key,cfg.key)) {
				value = Config::Value<string>{"authentication",key,cfg.def};
				debug(key,"='",value.c_str(),"'");
				if(value.empty()) {
					Logger::String{"Missing required value '",key,"' in authentication engine configuration"}.error();
					throw runtime_error(_("Invalid authentication engine configuration. Please check server settings."));
				}
				return true;
			}
		}

		if(!strcasecmp(key,"username")) {
			value = ""; // FIX-ME: Get username from context.
			return true;
		}

		if(!strcasecmp(key,"authentication-code")) {
			value = code;
			return true;
		}

		Logger::String{"Missing required value '",key,"' in authentication engine configuration"}.error();
		throw runtime_error(_("Invalid authentication engine configuration. Please check server settings."));
	}

	int OAuth::Context::handle() {

		try {

			if(empty()) {
				Logger::String{"Empty html request, sending login page"}.trace();
				set(HTTP::Authentication::LoginPage);
				return send_template(200,"signin","login");
			}

			debug("--------------- Checking for options ---------------");
			String action = pop();

			debug("Requested action: '",action.c_str(),"'");

			switch(action.select("callback",NULL)) {
			case 0: // callback

				// Reference: https://docs.github.com/pt/apps/oauth-apps/building-oauth-apps/authorizing-oauth-apps#web-application-flow

				debug("---> callback");
				debug("uri=",uri.c_str())
				debug("code=",code.c_str())

				if(code.empty()) {
					message(strerror(EPERM),_("Invalid response from authentication server"));
					return send_template(400,"error");
				}

				// Get access token
				String access_token;
				String token_type;
				{
					Config::Value<String> url{"authentication","get-token-url",""};
					debug("get-token-url='",url.c_str(),"'");
					url.expand([this](const char *key, std::string &value) {
						return this->getProperty(key,value);
					});
					if(url.empty()) {
						Logger::Message{"Missing required value for authentication attribute '{}'","get-token-url"}.error();
						message(strerror(EPERM),_("Invalid authentication engine configuration. Please check server settings."));
						return send_template(400,"error");
					}

					Config::Value<String> payload{"authentication","get-token-payload",""};
					debug("get-token-payload='",payload.c_str(),"'");
					payload.expand([this](const char *key, std::string &value) {
						return this->getProperty(key,value);
					});

					HTTP::Method method = HTTP::MethodFactory(Config::Value<string>{"authentication","get-token-method","post"}.c_str());

					debug("URL: ",url.c_str());
					debug("Payload: ",payload.c_str());
					debug("Method: ",std::to_string(method));

					auto response = URL{url.c_str()}
						.call(
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
						throw runtime_error(error_description);
					}

					if(!error.empty()) {
						throw runtime_error(error);
					}

					if(access_token.empty()) {
						throw runtime_error(_("Empty token on authentication server response"));
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
							message(strerror(EPERM),_("Invalid authentication engine configuration. Please check server settings."));
							return send_template(400,"error");
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

					} catch(const std::exception &e) {

						Logger::String{url.c_str()," returned  '",e.what(),"'"}.error();

						message(e.what(),_("Failed to retrieve user information from the authentication server."));
						return send_template(400,"error");

					}
				}

				throw runtime_error("Incomplete");

			}

			// Unknow request, send error page.
			message(
				_("Unrecognized request"),
				Logger::Message{_("The requested action '{}' is not available on this server"), action.c_str()}.c_str()
			);
			return send_template(404,"","error");

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error();
			Authentication::reset();	
			message(_("We're sorry, but we encountered an error while processing your request."),e.what());
			return send_template(404,"error");

		}


	}

	int OAuth::Context::send_template(int code, const char *action, const char *tmplt) {

		Udjat::HTTP::Template text{tmplt,Udjat::MimeType::html};

		// Last, expand request arguments.
		text.expand([this,code,action](const char *key, std::string &value) {

			if(!strcasecmp(key,"code")) {
				Logger::String{"Using obsolete '%{code}' on template"}.warning();
				value = std::to_string(code);
				return true;
			}

			if(!strcasecmp(key,"error-code")) {
				value = std::to_string(code);
				return true;
			}

			if(!strcasecmp(key,"action")) {
				value = String{"/oauth2/",action};
				return true;
			}

			return this->getProperty(key,value);

		});

		return send_html_response(code,text.c_str());
	}

 }

