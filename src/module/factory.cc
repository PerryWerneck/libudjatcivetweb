/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/defs.h>
 #include <udjat/module/abstract.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/xml.h>
 #include <udjat/module/civetweb.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/civetweb/protocol.h>

 namespace Udjat {


	/// @brief Client & server module
	class Hybrid : public CivetWeb::Module {
	private:
		CivetWeb::Protocol *protocols[2];

		void init(const ModuleInfo &info) {
			protocols[0] = new CivetWeb::Protocol{"http",info};
			protocols[1] = new CivetWeb::Protocol{"https",info};
		}

	public:
		Hybrid(const ModuleInfo &info, const XML::Node &node) : CivetWeb::Module{info,node} {
			init(info);
		}

		Hybrid(const ModuleInfo &info, const char *name) : CivetWeb::Module{info,name} {
			init(info);
		}

		virtual ~Hybrid() {
			delete protocols[0];
			delete protocols[1];
		}

	};

	Udjat::Module * CivetWeb::Module::Factory(const ModuleInfo &info, const char *name, bool client) {
		if(client) {
			return new Hybrid(info,name);
		}
		return new CivetWeb::Module(info,name);
	}

	Udjat::Module * CivetWeb::Module::Factory(const ModuleInfo &info, const XML::Node &node) {
		if(node.attribute("http-client").as_bool(true)) {
			return new Hybrid(info,node);
		}
		return new CivetWeb::Module(info,node);
	}

	CivetWeb::Module::Module(const ModuleInfo &info, const XML::Node &node) 
		: Udjat::Module{String{node,"name","httpd"}.as_quark(),udjat_module_info}, 
			Udjat::CivetWeb::Service(udjat_module_info,node) 
		{ }

	CivetWeb::Module::Module(const ModuleInfo &info, const char *name)
		: Udjat::Module{name,udjat_module_info}, 
			Udjat::CivetWeb::Service{udjat_module_info,XML::Node{}}
		{ }

	CivetWeb::Module::~Module() {

	}

 }
