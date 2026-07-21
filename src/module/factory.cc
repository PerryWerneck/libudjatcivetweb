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
 #include <udjat/module.h>
 #include <udjat/tools/properties.h>
 #include <udjat/module/civetweb.h>
 #include <udjat/tools/string.h>
 #include <private/client.h>

 namespace Udjat {

	/// @brief Client & server module
	class Hybrid : public CivetWeb::Module {
	private:
		CivetWeb::Client::Factory http{"http","CivetWEB " CIVETWEB_VERSION " HTTP server module for " STRINGIZE_VALUE_OF(PRODUCT_NAME)};
		CivetWeb::Client::Factory https{"https","CivetWEB " CIVETWEB_VERSION " HTTPS server module for " STRINGIZE_VALUE_OF(PRODUCT_NAME)};

	public:
		Hybrid(const Udjat::Properties &props) : CivetWeb::Module{props} {
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

	Udjat::Module * CivetWeb::Module::Factory(const Udjat::Properties &props) {
		Udjat::Module *module;
		if(props.get("http-client",true)) {
			module = new Hybrid(props);
		} else {
			module = new CivetWeb::Module(props);
		}
		module->autoclean();
		return module;
	}

	CivetWeb::Module::Module(const Udjat::Properties &props) 
		: Udjat::Module{props.get("name","httpd").as_quark()}, 
			Udjat::CivetWeb::Service(props) 
		{ }

	CivetWeb::Module::Module(const char *name, const char *description)
		: Udjat::Module{name,description}, Udjat::CivetWeb::Service{Udjat::Properties{}}{ 

		}

	CivetWeb::Module::~Module() {
	}

 }


