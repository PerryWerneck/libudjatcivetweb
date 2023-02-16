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
 #include <udjat/tools/application.h>

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		Image::Image(const char *name) {

			Application::DataDir file{"images"};

			if(file.find((string{name} + ".svg").c_str(),true)) {
				assign(file);
				return;
			}

			if(file.find((string{name} + ".png").c_str(),true)) {
				assign(file);
				return;
			}

			throw system_error(ENOENT,system_category(),string{"Can't find image '"} + name + "'");

		}


	}

 }
