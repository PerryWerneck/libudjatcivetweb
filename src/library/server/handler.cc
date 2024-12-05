/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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
 #include <stdexcept>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/logger.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	HTTP::Server::Handler::Handler(const XML::Node &node) 
		: Udjat::Interface::Handler{node}, method{HTTP::MethodFactory(node)} {	
	}

	bool HTTP::Server::Handler::operator==(const HTTP::Request &request) const {

		if(request.verb() != method) {
			debug("Method '",std::to_string(request.verb()),"' mismatch");
			return false;
		}

		if(!strcasecmp(c_str(),std::to_string(request.verb()))) {
			debug("Handler name match request action, accepting it");
			return true;
		}

		const char *ptr = request.path();
		if(ptr && *ptr && *ptr != '/') {
			throw runtime_error(Logger::String{"The request path '",ptr,"' is invalid. It should start with '/'"});
		}

		ptr++;

		debug("checking for name '",ptr,"'");

		// TODO: Pending implementation.
		
		return false;
	}

 }

