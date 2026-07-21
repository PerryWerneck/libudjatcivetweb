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

//  #include <config.h>

//  #undef LOG_DOMAIN
//  #define LOG_DOMAIN "httpd"
//  #include <udjat/tools/logger.h>

//  #include <stdexcept>
//  #include <udjat/tools/http/server.h>
//  #include <udjat/tools/http/handler.h>
//  #include <udjat/tools/logger.h>
//  #include <udjat/tools/configuration.h>
//  #include <udjat/tools/interface.h>
//  #include <iostream>

//  using namespace std;

//  namespace Udjat {

// 	HTTP::Server * HTTP::Server::instance = nullptr;

// 	HTTP::Server & HTTP::Server::getInstance() {
// 		if(instance) {
// 			return *instance;
// 		}

// 		throw runtime_error("The HTTP service is unavailable");
// 	}

// 	HTTP::Server::Server(const char *name) : Interface::Factory{name}, apiver{100} {

// 		// Check for secondary instance.
// 		if(instance) {
// 			Logger::String{"Building a new HTTP server instance"}.trace();
// 		} else {
// 			instance = this;
// 		}
// 	}

// 	HTTP::Server::Server(const XML::Node &node) : Server{String{node,"interface-name","web"}.as_quark()} {
// 		apiver = XML::AttributeFactory(node,"api-version").as_uint(apiver);
// 	}

// 	HTTP::Server::~Server() {
// 		if(instance == this) {
// 			instance = nullptr;
// 			Logger::String{"Deleting default HTTP server instance"}.trace();
// 		} else {
// 			Logger::String{"Deleting non default HTTP server instance"}.trace();
// 		}
// 	}

// 	Udjat::Interface & HTTP::Server::InterfaceFactory(const Properties &props) {

// 		const char * path{props["http-path"].as_quark()};

// 		if(!(path && *path)) {
// 			path = props["path"].as_quark();
// 		}

// 		if(!(path && *path)) {
// 			path = props["name"].as_quark();
// 		}

// 		for(Interface &intf : interfaces) {
// 			if(!strcasecmp(path,intf.name())) {
// 				Logger::String{"Reusing interface '",path,"'"}.trace();
// 				return intf;
// 			}
// 		}

// 		interfaces.emplace_back(props,path);
// 		return interfaces.back();
// 	}

//  }

