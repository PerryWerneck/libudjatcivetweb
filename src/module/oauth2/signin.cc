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
  * @brief Implements OAuth::authorize.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/oauthd.h>
 #include <private/request.h>

//  #include <udjat/tools/intl.h>
//  #include <udjat/tools/http/request.h>
//  #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

 	int OAuth::Context::signin() {

		// debug("Username= '",request["username"].c_str(),"'");

		// // Check if username exists
		// // username = request['username']
		// // password = request['password']

		// // Mount redirect URI
		// location = String {
		// 	request["redirect_uri"].c_str(),
		// 	"?",
		// 	"state=",
		// 	request["state"].c_str()
		// };

		// switch(String{request["response_type"].c_str()}.select("code","token",nullptr)) {
		// case 0:	// Code flow
		// 	location += "&code=";
		// 	location += "undefined";
		// 	break;

		// case 1: // Implicit flow
		// 	location += "&access_token=";
		// 	location += "undefined";
		// 	location += "token_type=Bearer&expires_in=";
		// 	location += authentication->expires() - time(0);
		// 	location += "&scope=";
		// 	location += request["scope"];
		// 	break;

		// default:
		// 	throw runtime_error(Logger::Message{_("Unexpected response type '{}'"),request["response_type"].c_str()});
		// }

		return 0;

 	}

 }
