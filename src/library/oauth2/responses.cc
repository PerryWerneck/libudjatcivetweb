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
 #include <udjat/tools/http/template.h>
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

	int OAuth::Context::failed(int code, const char *message, const char *body) const {

		debug(__FUNCTION__,"(",code,",'",message,"')");
		
		Udjat::HTTP::Template text{"error",Udjat::MimeType::html};

		text.expand([this,code,message,body](const char *key, std::string &value) {

			if(!strcasecmp(key,"code")) {
				Logger::String{"Using obsolete '%{code}' on template"}.warning();
				value = std::to_string(code);
				return true;
			}

			if(!strcasecmp(key,"error-code")) {
				value = std::to_string(code);
				return true;
			}

			if(!strcasecmp(key,"message")) {
				value = message;
				return true;
			}
			
			if(!strcasecmp(key,"body")) {
				value = body;
				return true;
			}

			if(!strcasecmp(key,"icon")) {
				value = "/icon/computer-fail-symbolic";
				return true;
			}

			return this->getProperty(key,value);

		});

		return send_html_response(code,text.c_str());
		
	}

	int OAuth::Context::send_template(int code, const char *action, const char *tmplt) {

		Udjat::HTTP::Template text{tmplt,Udjat::MimeType::html};

		// Expand request arguments.
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

