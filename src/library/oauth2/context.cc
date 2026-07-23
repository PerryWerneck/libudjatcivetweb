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
 #include <udjat/tools/http/authentication.h>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/value.h>
 #include <private/client.h>
 #include <udjat/tools/memory.h>

 #if defined(HAVE_JSON_C)
	#include <json.h>
 #endif // HAVE_JSON_C

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	OAuth::Context::Context(const char *path) {

		if(path && *path) {

			while(*path == '/') {
				path++;
			}

			if(!strncasecmp(path,"oauth2",6)) {
				path += 6;
			}

			while(*path == '/') {
				path++;
			}

			this->path = path;
		}

		debug("--- Building oauth context to '",(path ? path : ""),"'");

	}

	OAuth::Context::~Context() {
	}

	HTTP::StatusCode OAuth::Context::authenticate(const char *target) {

		debug(__FUNCTION__,"(",target,")");

		clear();

		if(!Authentication::available()) {
			return failed(HTTP::Unavailable,_("Configuration Error"),_("An authentication method has not been configured for this webpage. Please reach out to the system administrator."));
		}

		Config::Value<Udjat::String> endpoint{"authentication","endpoint"};
		if(endpoint.empty()) {
			return failed(HTTP::Unavailable,_("Configuration Error"),_("Connection failed. The system authentication endpoint is undefined or improperly configured."));
		}

		endpoint.expand(this);

		debug("Redirecting to endpoint at '",endpoint.c_str(),"'");

		return send_redirect_response(endpoint.c_str());
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

	bool OAuth::Context::get_property(const char *key, Udjat::Value &value) const {

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

			value = encrypt(buffer,szBuffer).escape().c_str();

			return true;
		}

		if(!strcasecmp(key,"redirect-uri")) {
			value = uri.c_str();
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
				value = Config::Value<string>{"authentication",key,cfg.def}.c_str();
				debug(key,"='",value.c_str(),"'");
				if(value.empty()) {
					Logger::String{"Missing required value '",key,"' in authentication engine configuration"}.error();
					throw runtime_error(_("Invalid authentication engine configuration. Please check server settings."));
				}
				return true;
			}
		}

		if(!strcasecmp(key,"username")) {
			value = ""; // FIX-ME: Get username.
			return true;
		}

		if(!strcasecmp(key,"authentication-code")) {
			value = code;
			return true;
		}

		Logger::String{"Missing required value '",key,"' in authentication engine configuration"}.error();
		throw runtime_error(_("Invalid authentication engine configuration. Please check server settings."));

	}

	HTTP::StatusCode OAuth::Context::authenticated() {

		if(uri.empty()) {
			uri = Config::Value<string>{"authentication","authenticated","/"}.c_str();
		}

		// Reset expiration time.
		expiration_time = time(0) + Config::Value<time_t>("authentication","expiration-time",86400);

		// Redirect to index.
		return send_redirect_response(uri.c_str(),true);

	}

 }

