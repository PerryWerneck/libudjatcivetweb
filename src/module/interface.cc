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
 #include <udjat/tools/civetweb/interface.h>
 #include <udjat/tools/logger.h>
 #include <civetweb.h>

 using namespace Udjat;

 Udjat::Interface & CivetWeb::Module::InterfaceFactory(const XML::Node &node) {

	/*
	class Interface : public CivetWeb::Interface {
	private:

		std::vector<Handler> handlers;

	public:
		Interface(const XML::Node &node);
		virtual ~Interface();


	};
	*/

 }

 CivetWeb::Interface::Interface(const XML::Node &node) : Udjat::Interface{node}, path{String{node,"path"}.as_quark()} {

	if(!(path && *path)) {
		path = String("/",Udjat::Interface::c_str(),"/").as_quark();
	}

	if(path[0] != '/' || strlen(path) < 2 || path[strlen(path)-1] != '/') {
		throw runtime_error(String{"Path '",path,"' is invalid, it should start and end with '/'"});
	}

 }

 CivetWeb::Interface::~Interface() {
 }

