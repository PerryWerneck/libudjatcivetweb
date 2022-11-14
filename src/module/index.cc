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
  * @brief Implements the default index page.
  *
  */

 #include "private.h"
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/worker.h>
 #include <udjat/module.h>
 #include <cstring>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/string.h>
 #include <sstream>

 using namespace std;
 using namespace Udjat;

 int rootWebHandler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {
 	CivetWeb::Connection connection{conn};
	return connection.info(connection.local_uri());
 }
