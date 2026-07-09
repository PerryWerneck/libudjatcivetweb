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
				return callback();

			}

			// Unknow request, send error page.
			return failed(
				404,
				_("Unrecognized request"),
				Logger::Message{_("The requested action '{}' is not available on this server"), action.c_str()}.c_str()
			);

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error();
			clear();
			return failed(
				500,
				_("We're sorry, but we encountered an error while processing your request."),
				e.what()
			);

		}

	}


 }

