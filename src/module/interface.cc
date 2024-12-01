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

 #include <private/module.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/string.h>
 #include <udjat/module/civetweb.h>
 #include <udjat/tools/civetweb/service.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/response.h>
 #include <civetweb.h>
 #include <private/request.h>

 using namespace Udjat;

 CivetWeb::Service::Interface::Interface(const XML::Node &node, const char *p) : Udjat::Interface{node}, path{p} {
	if(path[0] == '/' || (strlen(path)>1 && path[strlen(path)-1] == '/')) {
		throw runtime_error(String{"Path '",path,"' is invalid, cant start or end with '/'"});
	}
 }

 CivetWeb::Service::Interface::~Interface() {
 }

 void CivetWeb::Service::Interface::call(const char *method, HTTP::Request &request, HTTP::Response &response) {
	debug(method,"(",request.c_str(),")");
	for(auto &handler : *this) {
		if(strcasecmp(method,handler.c_str())) {
			continue;
		}
		handler.call(request,response);
	}
 }

 void CivetWeb::Service::Interface::build_handlers(const XML::Node &node) {
	for(auto child = node.child("handler"); child; child = child.next_sibling("handler")) {
		emplace_back(child);
	}
	if(empty()) {
		emplace_back("get",node);
	}
 }

 int CivetWeb::Service::request_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept {

	try {

		unsigned int apiver = (PACKAGE_VERSION_MAJOR * 100) + PACKAGE_VERSION_MINOR;
		const mg_request_info *info = mg_get_request_info(conn); 
		const char *path = info->local_uri;

		if(path && *path && !strncasecmp(path,"/api/",5)) {
			path += 4;
			if(isdigit(path[1])) {
				path++;
				apiver = 0;
				while(*path && *path != '/') {
					if(isdigit(*path)) {
						apiver *= 10;
						apiver += (*path - '0');
					}
					path++;
				}
			}
		}

		HTTP::Response response{MimeTypeFactory(conn)};

		//
		// Check for interfaces
		//
		for(auto &interface : srvc->interfaces) {

			size_t szpath = strlen(interface.c_str());

			if(strncasecmp(interface.c_str(),path+1,szpath)) {
				debug("Ignoring '",interface.c_str(),"'");
				continue;
			}

			path += (szpath+1);
			if(*path && *path != '/') {
				debug("Ignoring '",interface.c_str(),"'");
				continue;
			}

			if(Logger::enabled(Logger::Debug)) {
				Logger::String{"Handling ",info->request_method,"(",path,") with interface '", interface.c_str(),"' version ",apiver}.trace();
			}

			CivetWeb::Request request{conn,path,apiver};
			interface.call(info->request_method,request,response);

			return send(conn,response);

		}

		response.failed(ENOENT);
		return send(conn,response);

	} catch(const exception &e) {
		HTTP::Response response{MimeTypeFactory(conn)};
		response.failed(e);
		return send(conn,response);
	} catch(...) {
		HTTP::Response response{MimeTypeFactory(conn)};
		response.failed(_("Unexpected error"));
		return send(conn,response);
	}


 }
