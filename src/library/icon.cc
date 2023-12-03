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
 #include <udjat/tools/http/icon.h>
 #include <map>
 #include <mutex>
 #include <udjat/tools/logger.h>

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

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
				debug("Appending icon '",name,"'");
				auto inserted = cache.emplace(make_pair(std::string{name},Icon(name)));

				if(!inserted.first->second.empty()) {
					cout << "civetweb\tCaching " << inserted.first->second << " as " << name << endl;
				}
				return inserted.first->second;

			}

		};

		Icon Icon::getInstance(const char *name) {

			static Controller controller;
			return controller.find(name);

		}


	}

 }
