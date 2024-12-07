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
  * @brief Implements the handler for images.
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/image.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/configuration.h>

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

 using namespace Udjat;

 int HTTP::Connection::image(const char *name) noexcept {

 	return exec([&](HTTP::Connection &connection){

		{
			if(*name == '/') {
				name++;
			}
			const char *ptr = strchr(name,'/');
			if(ptr) {
				name = ptr+1;
			}
		}

		debug("searching for image '",name,"'");

		Udjat::HTTP::Image filename{name};

		if(!filename) {
			throw HTTP::Exception(404,Logger::String{"Cant find image '",name,"'"}.c_str());
		}

		Logger::String{"Sending static file '", filename.c_str(),"'"}.trace();
		
		return send(
			HTTP::Get,
			filename.c_str(),
			false,
			nullptr,
			Config::Value<unsigned int>("theme","image-max-age",604800)
		);

	});

 }
