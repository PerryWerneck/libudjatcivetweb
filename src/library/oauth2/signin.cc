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
 #include <udjat/tools/intl.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

 	int OAuth::signin(HTTP::Request &request, Context &context) {

		OAuth::User user{request};

		context.location.clear(); // Just in case

		if(!user.authenticate(request,context.message)) {
			Logger::String{request.address().c_str()," authentication failed: ",context.message.c_str()}.error("oauth2");
			user.get(context);
			context.message = _("Access Denied");
			return EPERM;
		}

		user.get(context);	// This will update response with the authentication info.

		// Mount redirect URI
		context.location = request["redirect_uri"];
		context.location += "?";
		context.location += "state=";
		context.location += request["state"];

		switch(request["response_type"].select("code","token",nullptr)) {
		case 0:	// Code flow
			context.location += "&code=";
			context.location += user.code().escape();
			break;

		case 1: // Implicit flow
			context.location += "&access_token=";
			context.location += context.token.escape();
			context.location += "token_type=Bearer&expires_in=";
			context.location += context.expiration_time - time(0);
			context.location += "&scope=";
			context.location += request["scope"];
			break;

		default:
			throw runtime_error("Unexpected response type");
		}

		return 0;

 	}

 }
