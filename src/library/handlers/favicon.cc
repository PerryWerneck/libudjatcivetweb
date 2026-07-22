/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements the faviconn output.
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/image.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/http/icon.h>
 #include <udjat/tools/http/connection.h>
 #include <fcntl.h>
 #include <string>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	HTTP::StatusCode HTTP::Connection::favicon() noexcept {

		Config::Value<time_t> maxage{"theme","icon-max-age",604800};
#ifdef _WIN32
		Config::Value<string> filename{"theme","favicon","icons/favicon.ico"};
#else
		Config::Value<string> filename{"theme","favicon","/usr/share/pixmaps/distribution-logos/favicon.ico"};
#endif // _WIN32

		return send(
			filename.c_str(), 
			(time_t) maxage, 
			MimeType::icon
		);

	}

 }
