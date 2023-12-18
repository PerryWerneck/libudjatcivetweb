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

 namespace Udjat {

 	int OAuth::authorize(HTTP::Request &request, std::string &token) {

		if(request["grant_type"] == "client_credentials") {

			OAuth::Client client{request};
			Logger::String{request.address().c_str()," is asking for client credentials"}.trace("oauth2");

			// TODO: Identify client.


			// Allow response
			token = client.token();
			return 200;
		}

		return 500;
 	}

 }
