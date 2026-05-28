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
 #include <udjat/tools/xml.h>
 #include <udjat/module/civetweb.h>
 #include <udjat/tools/string.h>
 #include <private/client.h>

 namespace Udjat {

	/// @brief Client & server module
	class Hybrid : public CivetWeb::Module {
	private:
		CivetWeb::Client::Factory http{"http","CivetWEB " CIVETWEB_VERSION " HTTP module for " STRINGIZE_VALUE_OF(PRODUCT_NAME)};
		CivetWeb::Client::Factory https{"https","CivetWEB " CIVETWEB_VERSION " HTTPS module for " STRINGIZE_VALUE_OF(PRODUCT_NAME)};

	public:
		Hybrid(const XML::Node &node) : CivetWeb::Module{node} {
		}

		Hybrid(const char *name) : CivetWeb::Module{name} {
		}

		virtual ~Hybrid() {
		}

	};

	Udjat::Module * CivetWeb::Module::Factory(const char *name, bool client) {
		Udjat::Module *module;
		if(client) {
			module = new Hybrid(name);
		} else {
			module = new CivetWeb::Module(name);

		}
		module->autoclean();
		return module;
	}

	Udjat::Module * CivetWeb::Module::Factory(const XML::Node &node) {
		Udjat::Module *module;
		if(node.attribute("http-client").as_bool(true)) {
			module = new Hybrid(node);
		} else {
			module = new CivetWeb::Module(node);
		}
		module->autoclean();
		return module;
	}

	CivetWeb::Module::Module(const XML::Node &node) 
		: Udjat::Module{String{node,"name","httpd"}.as_quark()}, 
			Udjat::CivetWeb::Service(node) 
		{ }

	CivetWeb::Module::Module(const char *name, const char *description)
		: Udjat::Module{name,description}, Udjat::CivetWeb::Service{XML::Node{}}{ 

		}

	CivetWeb::Module::~Module() {
		interfaces.clear();
	}

 }


