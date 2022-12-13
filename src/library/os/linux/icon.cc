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
 #include <udjat/tools/http/icons.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/logger.h>
 #include <unistd.h>
 #include <dirent.h>
 #include <udjat/tools/configuration.h>

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		Icon::Icon(const char *n) {

			static const char * defpaths =
					"/usr/share/icons/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/," \
					"/usr/share/icons/Adwaita/," \
					"/usr/share/icons/gnome/," \
					"/usr/share/icons/hicolor/," \
					"/usr/share/icons/HighContrast/";

			Config::Value<std::vector<string>> paths("theme","iconpath",defpaths);

			string name{n};
			if(!strchr(n,'.')) {
				name += ".svg";
			}

			// First search for filenames.
			for(string &path : paths) {

				string filename{path + name};

				if(access(filename.c_str(),R_OK) == 0) {
					assign(filename);
					debug("Found '",c_str(),"'");
					return;
				}

			}

			// Then search paths
			for(string &path : paths) {

				try {
					File::Path folder{path};
					if(folder && folder.find(name.c_str(),true)) {
						assign(folder);
						debug("Found '",c_str(),"'");
						return;
					}
				} catch(const std::exception &e) {
					cout << "icon\t" << e.what() << endl;
				}

			}

			clear();

		}


	}

 }
