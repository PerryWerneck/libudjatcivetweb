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

 /**
  * @brief Implements the swagger.json output.
  *
  * References:
  *
  * https://samanthaneilen.github.io/2018/12/08/Using-and-extending-swagger.json-for-API-documentation.html
  *
  */

 #include "private.h"
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/worker.h>
 #include <udjat/module.h>
 #include <cstring>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/string.h>
 #include <sstream>

 using namespace std;
 using namespace Udjat;

 int rootWebHandler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {

	CivetWeb::Connection connection{conn};

	debug("local_uri='",connection.local_uri(),"'");

	if(!strcasecmp(connection.local_uri(),Config::Value<string>("http","appinfo","/"))) {

		debug("Sending application info");

		stringstream page;

		page << "<!DOCTYPE html>"
				"<html lang=\"en\">"
				"<head>"
				"<meta charset=\"utf-8\">";

		page << "<title>" << Application::Name() << "</title>";

		page <<	"</head><body>";

		page << "<h1>" << Application::Name() << "</h1>";

		// Agent information
		{
			auto root = Udjat::Abstract::Agent::root();
			if(root) {

				page	<< "<h2>" << _("Agent info") << "</h2><ul>"
						<< "<li><a href=\"/api/1.0/agent.html\">" << _("Service information") << "</a></li>";

				for(auto agent : *root) {

					const char *summary = agent->summary();

					if(!(summary && *summary)) {
						summary = agent->name();
					}

					page 	<< "<li><a href=\"/api/1.0/agent/" << agent->name() << ".html\">"
							<< summary
							<< "</a></li>";
				}

				page << "</ul>";
			}

		}

		// Workers
		{
			page << "<h2>URL workers</h2><ul>";

			Udjat::Worker::for_each([&page](const Worker &worker){

				page 	<< "<li><a href=\"" << "/api/1.0/"
						<< worker.c_str()
						<< ".html\">"
						<< worker.c_str()
						<< "</a>";
			});

			page << "</ul>";
		}

		// Application information
		{
			auto module = Module::find("information");
			if(module) {

				auto options = (*module)["options"];

				if(!options.empty()) {

					page << "<h2>" << ( (*module)["description"]) << "</h2><ul>";

					for(auto option : String{options}.split(",")) {

						page 	<< "<li><a href=\"" << "/api/1.0/info/"
								<< option
								<< ".html\">"
								<< option
								<< "</a></li>";

					}

					page << "</ul>";

				}

			}

		}

		page << "</body></html>";

		return connection.success(
						to_string(MimeType::html),
						page.str().c_str(),
						page.str().size()
				);

	}


	mg_send_http_error(conn, 404, strerror(ENOENT));
	return 404;
 }
