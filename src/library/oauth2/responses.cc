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
 #include <udjat/tools/template.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/url.h>
 #include <private/client.h>
 #include <udjat/tools/memory.h>
 #include <udjat/tools/variant.h>

 #if defined(HAVE_JSON_C)
	#include <json.h>
 #endif // HAVE_JSON_C

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	HTTP::StatusCode OAuth::Context::failed(HTTP::StatusCode code, const char *message, const char *body) const {

		debug(__FUNCTION__,"(",code,",'",message,"')");
		
		Udjat::Template tmplt{"failed",Udjat::MimeType::html};

		auto text = tmplt.to_string([this,code,message,body](const char *key, std::ostream &stream) {

			if(!strcasecmp(key,"code")) {
				Logger::String{"Using obsolete '%{code}' on template"}.warning();
				stream << code;
				return true;
			}

			if(!strcasecmp(key,"status-code")) {
				stream << code;
				return true;
			}

			if(!strcasecmp(key,"status-message")) {
				stream << message;
				return true;
			}
			
			if(!strcasecmp(key,"status-body")) {
				stream << body;
				return true;
			}

			if(!strcasecmp(key,"status-icon")) {
				stream << "/icon/computer-fail-symbolic";
				return true;
			}

			{
				Udjat::Value val;
				if(this->get_property(key,val)) {
					stream << val;
					return true;
				}
			}

			return false;

		});

		return send_html_response(code,text.c_str());
		
	}

	HTTP::StatusCode OAuth::Context::send_template(HTTP::StatusCode code, const char *action, const char *name) {

		Udjat::Template tmplt{name,Udjat::MimeType::html};

		// Expand request arguments.
		auto text = tmplt.to_string([this,code,action](const char *key, std::ostream &stream) {

			if(!strcasecmp(key,"code")) {
				Logger::String{"Using obsolete '%{code}' on template"}.warning();
				stream << code;
				return true;
			}

			if(!strcasecmp(key,"error-code")) {
				stream << code;
				return true;
			}

			if(!strcasecmp(key,"action")) {
				stream << String{"/oauth2/",action}.c_str();
				return true;
			}

			{
				Udjat::Value v;
				if(this->get_property(key,v)) {
					stream << v;
					return true;
				}
			}

			return false;

		});

		return send_html_response(code,text.c_str());
	}

 }

