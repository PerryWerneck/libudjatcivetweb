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

 #include <udjat/tools/protocol.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/response.h>
 #include <civetweb.h>
 #include <private/request.h>
 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 HTTP::Server::Interface::Interface(const XML::Node &node, const char *p) : Udjat::Interface{node}, path{p} {
	if(path[0] == '/' || (strlen(path)>1 && path[strlen(path)-1] == '/')) {
		throw runtime_error(String{"Path '",path,"' is invalid, cant start or end with '/'"});
	}
 }

 HTTP::Server::Interface::~Interface() {
 }

 void HTTP::Server::Interface::call(const char *method, HTTP::Request &request, HTTP::Response &response) {

	debug(method,"(",request.c_str(),")");

	if(empty()) {

		Logger::String{"Empty interface, echoing request properties"}.info(Udjat::Interface::c_str());
		response.merge(request);

	} else {

		for(auto &handler : *this) {
			if(strcasecmp(method,handler.c_str())) {
				continue;
			}
			if(request.verb() == HTTP::Get) {
				// Get method, extract arguments from path.
				request.rewind();
				handler.for_each([&](const Handler::Introspection &intr){
					if(intr.direction == intr.Both || intr.direction == intr.Input) {
						string vlr;
						request.pop(vlr);
						debug(intr.name,"='",vlr,"'");
						request[intr.name].set(vlr,intr.type);
					}
					return false;
				});
			}
			request.rewind();
			handler.call(request,response);
		}

	}

 }

 void HTTP::Server::Interface::build_handlers(const XML::Node &node) {
	for(auto child = node.child("handler"); child; child = child.next_sibling("handler")) {
		emplace_back(child);
	}
	if(empty()) {
		emplace_back("get",node);
	}
 }

