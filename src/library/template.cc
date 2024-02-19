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

 /**
  * @brief Implement template pages.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/template.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <fcntl.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 namespace Udjat {

	HTTP::Template::Template(const char *name, const MimeType mimetype) {

		debug(__FUNCTION__,"(",name,",",std::to_string(mimetype),")");

		if(mimetype == MimeType::custom) {
			return;
		}

		Application::DataFile filename{"templates/www/"};

		filename += name;
		filename += ".";
		filename += std::to_string(mimetype,true);

		if(!filename) {
			Logger::String{"Cant find template '",filename.c_str(),"' (",std::to_string(mimetype),")"}.trace("http");
			return;
		}

		Logger::String{"Loading template from '",filename.c_str(),"'"}.trace("http");
		assign(filename.load());

        expand([](const char *key, std::string &value) {

 			if(!strcasecmp(key,"app-name")) {
				value = Application::Name();
				return true;
 			}

			if(!strcasecmp(key,"css-name")) {

				value = Config::Value<std::string>{"theme","httpd",""};

				if(value.empty()) {
					value = "/";
					value += Application::Name();
					value += "/css/style.css";
				}

				return true;
			}

			return false;

        },false,false);


	}

 }
