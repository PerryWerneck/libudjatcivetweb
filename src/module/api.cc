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
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/report.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/logger.h>

 static std::string report(const CivetWeb::Connection &connection, const char *path, const char *method, const MimeType mimetype) {

	// It's a report
	if(strcasecmp(method,"get")) {
		throw HTTP::Exception(405, connection.request_uri(), "Method Not Allowed");
	}

	HTTP::Report  response{path, mimetype};
	HTTP::Request request(path,method);

	// Run report.
	if(!Worker::work(request.getMethod(),request,response)) {
		throw HTTP::Exception(405, connection.request_uri(), "Method Not Allowed");
	}

	return response.to_string();

 }

 int apiWebHandler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {

	return webHandler(CivetWeb::Connection(conn),[](const CivetWeb::Connection &connection, const char *path, const char *method, const MimeType mimetype){

		debug("*** path='",path,"'");

		if(mimetype == MimeType::csv) {

			return report(connection,path,method,mimetype);

		} else if(!strncasecmp(path,"report/",7)) {

			return report(connection,path+7,method,mimetype);

		} else {

			// It's a 'normal' API request.
			HTTP::Response response(mimetype);
			HTTP::Request request(path,method);

			if(!Worker::work(request.getMethod(),request,response)) {
				throw HTTP::Exception(405, connection.request_uri(), "Method Not Allowed");
			}

			return response.to_string();

		}

	});

 }


