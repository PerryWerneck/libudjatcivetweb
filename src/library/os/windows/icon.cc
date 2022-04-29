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
 #include <udjat/tools/application.h>
 #include <fcntl.h>

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		Icon::Icon(const char *name) {

			if(find(Application::DataDir{"icons"},name,"svg")) {
				return;
			}

			throw system_error(ENOENT,system_category(),string{"Can't find icon '"} + name + "'");

		}

		bool Icon::find(const string &dirname, const char *name, const char *ext) {

			// Cleanup path.
			string path{dirname};
			for(char *ptr = (char *) path.c_str();*ptr;ptr++) {
				if(*ptr == '/') {
					*ptr = '\\';
				}
			}

			if(path[path.size()-1] == '\\') {
				path.resize(path.size()-1);
			}

			// First check file.
			string filename = path + "\\" + name + "." + ext;
			if(access(filename.c_str(),F_OK) == 0) {
				assign(filename);
#ifdef DEBUG
				cout << "Found " << filename << endl;
#endif // DEBUG
				return true;
			}
#ifdef DEBUG
			else {
				cout << "Not found " << filename << endl;
			}
#endif // DEBUG

			// Not found, scan path.
			WIN32_FIND_DATA FindFileData;

			HANDLE hFind = FindFirstFile((path + "\\*").c_str(), &FindFileData);
			do {

				if(FindFileData.cFileName[0] == '.') {
					continue;
				}

				string filename = path + "\\" + FindFileData.cFileName;

				DWORD attr = GetFileAttributes(filename.c_str());
				if(attr == INVALID_FILE_ATTRIBUTES) {
					cerr << "civetweb\tCant get attributes for '" << filename.c_str() << "' ignoring it" << endl;
					continue;
				}

				if(attr & FILE_ATTRIBUTE_DIRECTORY) {
					find(filename + "\\",name,ext);
				}

			} while(empty() && FindNextFile(hFind, &FindFileData) != 0);

			FindClose(hFind);

			return !empty();

		}

	}

 }
