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
 #include <udjat/tools/http/handler.h>
 #include <udjat/tools/http/server.h>
 #include <iostream>
 #include <udjat/tools/quark.h>

 using namespace std;

 namespace Udjat {

	HTTP::Handler::Handler(const char *u) : uri(u) {

		if(!(u && *u)) {
			throw system_error(EINVAL,system_category(),"http-handler attribute is required");
		}

		if(u[0] != '/') {
			throw system_error(EINVAL,system_category(),"http-handler should start with '/'");
		}

		if(u[strlen(u)-1] != '/') {
			throw system_error(EINVAL,system_category(),"http-handler should end with '/'");
		}

	}

	HTTP::Handler::Handler(const pugi::xml_node &node, const char *tagname) : HTTP::Handler(Quark(node,tagname,"").c_str()) {
	}

	HTTP::Handler::~Handler() {
		if(server) {
			server->remove(this);
		}
	}

 }
