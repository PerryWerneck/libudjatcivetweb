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
  * @brief Implements OAuth::access_token.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/value.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

 	int OAuth::access_token(HTTP::Request &request, Context &context, HTTP::Value &response) {

		OAuth::User user{request};

		if(user.code(request["code"].c_str())) {

			// Update context
			context.token = user.encrypt();
			context.expiration_time = user.expires();
			context.message.clear();
			context.location.clear();

			// Check expiration time
			int expires_in = (context.expiration_time - time(0));
			if(expires_in < 0) {
				Logger::String message{"Token is already expired"};
				context.message = message.c_str();
				message.error("oauth2");
				return EPERM;
			}

			response["token_type"] = "Bearer";
			response["expires_in"] = expires_in;
			response["scope"] = "*";
			response["refresh_token"] = context.token.c_str();

			// Setup access token
			HTTP::Request::Token token;
			response["access_token"] = user.encrypt(token).c_str();

			return 0;
		}

		Logger::String message{"Error parsing access code"};
		context.message = message.c_str();
		message.error("oauth2");
		return EPERM;

 	}

 }
