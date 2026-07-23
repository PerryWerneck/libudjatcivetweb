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
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/icon.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <string>
 #include <fcntl.h> 

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	HTTP::StatusCode HTTP::Connection::theme(const char *name) noexcept {

		while(*name && *name == '/') {
			name++;
		}

		{
			const char *ptr = strchr(name,'/');
			if(ptr) {
				name = ptr+1;
			}
		}

		debug("------------------ ",__FUNCTION__,"(",name,") --------------------------");
		if(strstr(name,"..")) {
			return send(
				HTTP::Status{HTTP::BadRequest,MimeType::html}
			);
		}

		Config::Value<unsigned int> maxage{"theme","theme-max-age",604800};

		String filename{
			Config::Value<string>{"theme","rootdir","/srv/www/htdocs/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/theme/default/"}.c_str(),
			name
		};

		debug("Filename='",filename.c_str(),"'");

		return send(
			filename.c_str(), 
			(time_t) maxage, 
			MimeTypeFactory(filename.c_str(),MimeType::html)
		);

	}

 }
