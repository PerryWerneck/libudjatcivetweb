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
 #include <udjat/civetweb.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/mimetype.h>

 int customWebHandler(struct mg_connection *conn, void *cbdata) {

	CivetWeb::Connection connection(conn);

	HTTP::Server::Handler *handler = (HTTP::Server::Handler *) cbdata;

	const struct mg_request_info *ri = connection.request_info();
	MimeType mimetype{MimeType::custom};
	string rsp;

	try {

		// Extract mimetype
		string uri = ri->local_uri;
		{
			auto ext = uri.find_last_of('.');
			if(ext != string::npos && ext > 1) {
				mimetype = MimeTypeFactory(uri.c_str()+ext+1);
			}
		}

		return handler->handle(
			connection,
			HTTP::Request(uri.c_str(),ri->request_method),
			mimetype
		);

	} catch(const HTTP::Exception &error) {

		return connection.response(error);

	} catch(const system_error &error) {

		return connection.response(error);

	} catch(const exception &error) {

		return connection.response(error);

	} catch(...) {

		connection.failed(500, "Unexpected error");
		return 500;

	}

	return 500;

 }


