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

 #include "private.h"
 #include <udjat/worker.h>
 #include <udjat/agent.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/http/exception.h>

 int reportWebHandler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {

	return webHandler(conn,[](const string &uri, const char *method, const MimeType mimetype){

		if(strcasecmp(method,"get")) {
			throw HTTP::Exception(405, uri.c_str(), "Method Not Allowed");
		}

		if(mimetype != MimeType::json) {
			throw HTTP::Exception(501, uri.c_str(), "Mimetype Not Supported");
		}

		HTTP::Report response;

		// Run report.
		Abstract::Agent::root()->find(uri.c_str())->get(Request(""),response);

		return response.to_string();

	});

 }


