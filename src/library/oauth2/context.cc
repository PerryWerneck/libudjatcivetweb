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

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	OAuth::Context::Context(const char *p) : path{p} {
		while(path[0] == '/') {
			path.erase(0,1);
		}
		pop();	// Remove '/oauth2'

		debug("---- Build OAuth2 context for '",path.c_str(),"'");	

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

			size_t szBuffer = redirect_uri.size()+sizeof(uint16_t)+1;
			char * buffer[szBuffer];
			memset(buffer,0,szBuffer);

			{
				static uint16_t sequencial = 0;
				*((uint16_t *) buffer) = sequencial++;
			}

			memcpy((buffer+sizeof(uint16_t)),redirect_uri.c_str(),redirect_uri.size());
			buffer[szBuffer] = 0;			

			value = encrypt(buffer,szBuffer).escape();

			return true;
		}

		if(!strcasecmp(key,"redirect-uri")) {
			value = redirect_uri;
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
			// { 
			// 	"client_id",	// Legacy! 
			// 	STRINGIZE_VALUE_OF(PRODUCT_NAME) 
			// },
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
				return true;
			}
		}

		if(!strcasecmp(key,"username")) {
			value = ""; // FIX-ME: Get username from context.
			return true;
		}

		throw runtime_error(Logger::String{"Required attribute '",key,"' is undefined"});
		return false;
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

			// switch(action.select("signin",nullptr)) {
			// case 0: // signin
			// 	debug("---> signin");
			// 	if(signin()) {
			// 		message(_("Access denied"));
			// 		return send_template(200,"signin","login");
			// 	}
			// 	throw runtime_error("Incomplete");

			// }

			// Unknow request, send error page.
			message(
				_("Unknonw request"),
				Logger::Message{_("The requested action '{}' is not available on this server"), action.c_str()}.c_str()
			);
			return send_template(404,"","error");

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error();
			Authentication::reset();	
			message(_("Internal error processing request"),e.what());
			return send_template(404,"","error");

		}


	}

	int OAuth::Context::send_template(int code, const char *action, const char *tmplt) {

		Udjat::HTTP::Template text{tmplt,Udjat::MimeType::html};

		// Last, expand request arguments.
		text.expand([this,code,action](const char *key, std::string &value) {

			if(!strcasecmp(key,"action")) {
				value = String{"/oauth2/",action};
				return true;
			}

			if(!strcasecmp(key,"code")) {
				value = std::to_string(code);
				return true;
			}

			return this->getProperty(key,value);

		});

		return send_html_response(code,text.c_str());
	}

 }

