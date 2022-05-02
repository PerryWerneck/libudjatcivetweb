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
 #include <unistd.h>
 #include <dirent.h>

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		Icon::Icon(const char *name) {

			{
				static const char * paths[] = {
					"/usr/share/icons/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/",
					"/usr/share/icons/"
				};

				for(size_t ix = 0; ix < (sizeof(paths)/sizeof(paths[0]));ix++) {

					string filename{paths[ix]};
					filename += name;
					filename += ".svg";

					if(access(filename.c_str(),F_OK) == 0) {
						assign(filename);
#ifdef DEBUG
						cout << "Found '" << *this << "'" << endl;
#endif // DEBUG
						return;
					}
#ifdef DEBUG
					else {
						cout << "Not Found '" << filename << "'" << endl;
					}
#endif // DEBUG

				}
			}

			{
				File::Path path;
				string filename{name};
				filename += ".svg";

#ifdef DEBUG
				cout << "Searching for '" << filename << "'" << endl;
#endif // DEBUG

				for(auto theme : Config::Value<std::vector<string>>("theme","icon","Adwaita,gnome,hicolor,HighContrast")) {

					path = "/usr/share/icons/";
					path += theme;

#ifdef DEBUG
					cout << "Searching '" << path << "'" << endl;
#endif // DEBUG

					if(path.find(filename.c_str(),true)) {
#ifdef DEBUG
						cout << "Found '" << path << "'" << endl;
#endif // DEBUG
						assign(path);
						return;
					}


				}


			}


			/*
			Config::Value<std::vector<string>> themes("theme","icon","Adwaita,gnome,hicolor,HighContrast");
			for(auto theme : themes) {


				cout << "----------------- " << theme << " -------------------------------------" << endl;

				if(find(string{"/usr/share/icons/"} + theme + "/scalable", name)) {
					return;
				}

			}
			*/

			throw system_error(ENOENT,system_category(),string{"Can't find icon '"} + name + "'");
		}


	}

 }
