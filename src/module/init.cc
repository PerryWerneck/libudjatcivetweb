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
 #include <udjat/defs.h>
 #include <udjat/module/civetweb.h>
 #include <udjat/tools/xml.h>

 const Udjat::ModuleInfo udjat_module_info{ "CivetWEB " CIVETWEB_VERSION " HTTP module for " STRINGIZE_VALUE_OF(PRODUCT_NAME) };

 Udjat::Module * udjat_module_init() {
	return Udjat::CivetWeb::Module::Factory(udjat_module_info,"httpd");
 }

 Udjat::Module * udjat_module_init_from_xml(const Udjat::XML::Node &node) {
	return Udjat::CivetWeb::Module::Factory(udjat_module_info,node);
 }

/*
 #include <udjat/tools/civetweb/service.h>
 #include <udjat/tools/civetweb/protocol.h>

 #include <udjat/defs.h>
 #include <udjat/module.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/expander.h>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/http/handler.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/worker.h>
 #include <udjat/module/abstract.h>
 #include <unistd.h>

 #include <private/module.h>

 using namespace Udjat;
 using namespace std;

 const Udjat::ModuleInfo udjat_module_info{ "CivetWEB " CIVETWEB_VERSION " HTTP module for " STRINGIZE_VALUE_OF(PRODUCT_NAME) };

 Udjat::Module * udjat_module_init() {
	return udjat_module_init_from_xml(XML::Node{});
 }

 Udjat::Module * udjat_module_init_from_xml(const pugi::xml_node &node) {

	class Module : public Udjat::Module, public Udjat::CivetWeb::Service { 
	private:
		struct {
			CivetWeb::Protocol http{"http",udjat_module_info};
			CivetWeb::Protocol https{"https",udjat_module_info};
		} protocols;

	public:

		Module(const pugi::xml_node &node) : Udjat::Module{"http",udjat_module_info}, Udjat::CivetWeb::Service(udjat_module_info,node) {
		}

		virtual ~Module() {
		}


	};

	return new Module(node);
 }


*/
