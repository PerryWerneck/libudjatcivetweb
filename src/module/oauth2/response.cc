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

 #include <config.h>

 #undef LOG_DOMAIN
 #define LOG_DOMAIN "oauthd"

 #include <udjat/defs.h>
 #include <udjat/tools/http/template.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/configuration.h>
 #include <string>
 #include <private/oauthd.h>
 #include <civetweb.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	void OAuth::Context::send_header() const {

		time_t expires = authentication->expires();
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
			request.session_cookie().c_str(),"=",
			authentication->token().c_str(),
			"; path=/oauth2; Expires=",
			HTTP::TimeStamp::to_string(expires).c_str()
		};

		debug("Cookie='",cookie,"'");
		mg_response_header_add(conn, "Set-Cookie", cookie.c_str(),-1);
		mg_response_header_send(conn);

	}

	int OAuth::Context::send_html_response(const char *tmplt, int code) const {

		Udjat::HTTP::Template text{tmplt,Udjat::MimeType::html};

		// Last, expand request arguments.
		text.expand([this,code](const char *key, std::string &value) {

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
				},
				{ 
					"action-signin", 
					"oauth2/signin" 
				},

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

			if(!strcasecmp(key,"code")) {
				value = std::to_string(code);
				return true;
			}

			if(!strcasecmp(key,"message")) {
				value = this->message;
				return true;
			}

			if(!strcasecmp(key,"body")) {
				value = this->body;
				return true;
			}

			if(request.getProperty(key,value)) {
				return true;
			}

			Logger::String{"Ignoring unexpected template item '",key,"'"}.warning();

			return false;
		},false,false);

		mg_response_header_start(conn, code);
		mg_response_header_add(conn, "Content-Type",std::to_string(MimeType::html),-1);
		mg_response_header_add(conn, "Content-Length", std::to_string(text.size()).c_str(), -1);
		send_header();

		mg_write(conn, text.c_str(), text.size());

		return code;

	}

 	int OAuth::Context::send_redirect_response() const {
		mg_response_header_start(conn, 303);
		mg_response_header_add(conn, "Location",location.c_str(),location.size());
		mg_response_header_add(conn, "Content-Length", "0", -1);
		send_header();
		return 303;
	}

 }

