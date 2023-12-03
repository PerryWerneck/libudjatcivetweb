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

	HTTP::Server::Server() {

		// Check for secondary instance.
		if(instance) {
			clog << "httpd\tBuilding a new HTTP server instance" << endl;
		} else {
			instance = this;
		}
	}

	HTTP::Server::~Server() {
		if(instance == this) {
			instance = nullptr;
		} else {
			clog << "httpd\tDeleting non default HTTP server instance" << endl;
		}
	}

	bool HTTP::Server::push_back(HTTP::Handler *handler) {
		/*
		if(handler->server) {
			handler->server->remove(handler);
		}

		if(Logger::enabled(Logger::Trace)) {
			Logger::String{"Adding handler for '",handler->uri,"'"}.write(Logger::Trace,"civetweb");
		}

		handler->server = this;
		*/
		return true;
	}

	/// @brief Remove request handler.
	/// @param uri the URI for the handler.
	bool HTTP::Server::remove(HTTP::Handler *handler) {

		/*
		if(handler->server && handler->server != this) {
			throw runtime_error("Handler is for another server");
		}

		if(Logger::enabled(Logger::Trace)) {
			Logger::String{"Removing handler for '",handler->uri,"'"}.write(Logger::Trace,"civetweb");
		}

		handler->server = nullptr;
		*/

		return true;
	}

 }

