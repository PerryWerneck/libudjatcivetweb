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
 #include <udjat/tools/http/image.h>
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

		Image::Image(const char *n) {

			static const char * defpaths =
					"/usr/share/pixmaps/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/," \
					"/usr/share/pixmaps/distribution-logos/";

			Config::Value<std::vector<string>> paths("theme","imgpath",defpaths);

			string name{n};
			if(!strchr(n,'.')) {
				name += ".svg";
			}

			debug("Searching for image '",name.c_str(),"'");

			// First search for filenames.
			for(string &path : paths) {

				string filename{path + name};

				if(access(filename.c_str(),R_OK) == 0) {
					assign(filename);
					debug("Found '",c_str(),"'");
					return;
				}
#ifdef DEBUG
				else {
					debug("Not found '",c_str(),"'");
				}
#endif

			}

			// Then search paths
			string filter{"*/"};
			filter += name;

			for(string &path : paths) {

				try {
					File::Path folder{path};
					if(folder && folder.find(name.c_str(),true)) {
						assign(folder);
						debug("Found '",c_str(),"'");
						return;
					}
#ifdef DEBUG
					else {
						debug("Not found '",c_str(),"'");
					}
#endif
				} catch(const std::exception &e) {
					cout << "image\t" << e.what() << endl;
				}

			}

			clear();

		}


	}

 }
