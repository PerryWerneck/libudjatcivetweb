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
  * @brief Implements the handler for icons.
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/icon.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

 using namespace Udjat;

 int HTTP::Connection::icon(const char *name) noexcept {

 	return exec([&](HTTP::Connection &connection){

	 	debug("Searching for icon",name);

		const char *path = strrchr(name,'/');
		if(path) {
			path++;
		}

		if(!(path && *path)) {
			throw HTTP::Exception(400,Logger::String{"Unable to handle icon '",name,"'"}.c_str());
		}

		debug("path='",path,"'");
		Udjat::HTTP::Icon icon = Udjat::HTTP::Icon::getInstance(path);

		if(icon.empty()) {
			throw HTTP::Exception(404,Logger::String{"Cant find icon '",name,"'"}.c_str());
		}

		return send(
			HTTP::Get,
			icon.c_str(),
			false,
			"image/svg+xml",
			Config::Value<unsigned int>("theme","icon-max-age",604800)
		);

	});

 }