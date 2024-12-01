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
 #include <udjat/tools/http/handler.h>
 #include <udjat/tools/logger.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	HTTP::Server * HTTP::Server::instance = nullptr;

	HTTP::Server & HTTP::Server::getInstance() {
		if(instance) {
			return *instance;
		}

		throw runtime_error("The HTTP service is unavailable");
	}

	HTTP::Server::Server(const char *name) : Interface::Factory{name} {

		// Check for secondary instance.
		if(instance) {
			clog << "httpd\tBuilding a new HTTP server instance" << endl;
		} else {
			instance = this;
		}
	}

	HTTP::Server::Server(const XML::Node &node) : Server{String{node,"interface-name","web"}.as_quark()} {
	}

	HTTP::Server::~Server() {
		if(instance == this) {
			instance = nullptr;
			cout << "httpd\tDeleting default HTTP server instance" << endl;
		} else {
			clog << "httpd\tDeleting non default HTTP server instance" << endl;
		}
	}

	Udjat::Interface & HTTP::Server::InterfaceFactory(const XML::Node &node) {

		const char * path{String{node,"path"}.as_quark()};

		if(!(path && *path)) {
			path = String{node,"name"}.as_quark();
		}

		for(Interface &interface : interfaces) {
			if(!strcasecmp(path,interface.c_str())) {
				Logger::String{"Reusing interface '",path,"'"}.trace();
				interface.build_handlers(node);
				return interface;
			}
		}

		interfaces.emplace_back(node,path);
		return interfaces.back();
	}

 }

