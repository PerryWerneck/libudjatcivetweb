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
 #include <map>
 #include <mutex>

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

 #include <dirent.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Icon::Controller {
		private:
			map<std::string,Icon> cache;

		public:
			Icon find(const char *name) {

				static mutex guard;
				lock_guard<mutex> lock(guard);

				// First check if its on cache.
				auto search = cache.find(string{name});
				if (search != cache.end()) {
					// Found it.
#ifdef DEBUG
					cout << "Found cached '" << search->second << "'" << endl;
#endif // DEBUG
					return search->second;
				}

				// Not found, create new icon.
				auto inserted = cache.emplace(make_pair(std::string{name},Icon(name)));

				cout << "icons\tCaching " << inserted.first->second << " as " << name << endl;
				return inserted.first->second;

			}

		};

		Icon Icon::getInstance(const char *name) {

			static Controller controller;
			return controller.find(name);

		}

		bool Icon::find(const string &path, const char *name, const char *ext) {

			string filename;

			// First check file.
			filename = path + "/" + name + ext;

			if(access(filename.c_str(),F_OK) == 0) {
				assign(filename);
#ifdef DEBUG
				cout << "Found '" << *this << "'" << endl;
#endif // DEBUG
				return true;
			}
#ifdef DEBUG
			else {
				cout << "Not Found '" << filename << "'" << endl;
			}
#endif // DEBUG

			DIR *directory = opendir(path.c_str());
			if(directory) {

				struct dirent *entry;
				while((entry=readdir(directory)) != NULL && empty()) {

					if(entry->d_name[0] == '.') {
						continue;
					}

					string fpath = path + entry->d_name;
					if(entry->d_type & DT_DIR) {
						find(path + "/" + entry->d_name, name, ext);
						continue;
					}


				}

				closedir(directory);
				return !empty();
			}

			return false;
		}


	}

 }
