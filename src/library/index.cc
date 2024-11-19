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
  * @brief Implements the default index page.
  *
  */

 #include <config.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/image.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/worker.h>
 #include <udjat/agent/state.h>
 #include <udjat/module.h>
 #include <cstring>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/icon.h>
 #include <sstream>

 using namespace std;
 using namespace Udjat;

 int HTTP::Connection::info(const char *path) {

	debug("local_uri='",path,"'");

	if(!strcasecmp(path,Config::Value<string>("http","appinfo","/"))) {

		debug("Sending application info");

		stringstream page;

		page << "<!DOCTYPE html>"
				"<html lang=\"" << _("en") << "\">"
				"<head>"
				"<meta charset=\"utf-8\">";

		page << "<title>" << Application::Name() << "</title>";

		page <<	"</head><body>";

		page << "<h1>" << Application::Name() << "</h1>";

		// Agent information
		try {

			auto root = Udjat::Abstract::Agent::root();
			if(root) {

				page	<< "<h2>" << _("Active agents") << "</h2><ul>"
						<< "<li><a href=\"/api/1.0/htmlagent.html\">";

				{
					auto icon = HTTP::Icon::getInstance(std::to_string(Abstract::Agent::root()->state()->level()));
					if(icon) {
						page << "<img src=\"icon/" << icon << ".svg\" height=\"16\" style=\"vertical-align:bottom\"/>&nbsp;";
					}
				}

				page	<< _("Application") << "</a></li>";

				root->for_each([&page,root](Abstract::Agent &agent){

					if(&agent == root.get()) {
						return;
					}

					const char *summary = agent.summary();

					if(!(summary && *summary)) {
						summary = agent.label();
					}

					if(!(summary && *summary)) {
						summary = agent.name();
					}

					page << "<li><a href=\"/api/1.0/agent" << agent.path() << ".html\">";

					{
						auto icon = HTTP::Icon::getInstance(std::to_string(agent.state()->level()));
						if(icon) {
							page << "<img src=\"icon/" << icon << ".svg\" height=\"16\" style=\"vertical-align:bottom\"/>&nbsp;";
						}
					}

					page << agent.name() << "&nbsp;<small>(" << summary << ")</small></a></li>";

				});

				page << "</ul>";
			}

		} catch(const std::exception &e) {

			cerr << "civetweb\tCant get agent list: " << e.what() << endl;

		}

		// Workers
		{
			page << "<h2>" << _("Workers") << "</h2><ul>";

			Udjat::Worker::for_each([&page](const Worker &worker){

				page 	<< "<li><a href=\"" << "/api/1.0/"
						<< worker.c_str()
						<< ".html\">"
						<< worker.c_str()
						<< "</a>";

				return false;

			});

			page << "</ul>";
		}

		// Application information
		{
			auto module = Udjat::Module::find("information");
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

		return send(
				to_string(MimeType::html),
				page.str().c_str(),
				page.str().size()
			);

	} else if(!strcasecmp(path,"/favicon.ico")) {

		static const char *names[] = {
			STRINGIZE_VALUE_OF(PRODUCT_NAME) ".ico",
			"favicon.ico"
		};

		debug("Searching for favicon");

		for(size_t ix = 0; ix < N_ELEMENTS(names);ix++) {

			HTTP::Image image{names[ix]};
			if(image) {

				debug("Found '",image.c_str(),"'");

				return send(
						HTTP::Get,
						image.c_str(),
						false,
						nullptr,
						Config::Value<unsigned int>("theme","image-max-age",604800)
					);

			}

		}

	}

	return send(Udjat::Response{
		(MimeType) *this
	}.failed(system_error(ENOENT,system_category())));

 }
