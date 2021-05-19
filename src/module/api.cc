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
 #include <tools.h>
 #include <udjat/worker.h>

 int apiWebHandler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {

	const struct mg_request_info *ri = mg_get_request_info(conn);

	if(strcasecmp(ri->request_method,"get")) {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}

	::Response response;

	try {

		const char *uri = ri->local_uri;

		// Extract 'API' prefix.
		if(strncasecmp(uri,"/api/",5) == 0) {
			uri += 5;
		} else {
			mg_send_http_error(conn, 400, "Request must be in the format /api/version/worker/path");
			return 400;
		}

		// Extract version prefix.
		{
			const char *ptr = strchr(uri,'/');
			if(!ptr) {
				mg_send_http_error(conn, 400, "Request must be in the format /api/version/worker/path");
				return 400;
			}
			uri = ptr+1;
		}

		// Get worker.
		{
			const char *ptr = strchr(uri,'/');
			string worker, path;

			if(ptr) {
				worker.assign(uri,ptr-uri);
				path = ptr+1;
			} else {
				worker = uri;
			}

#ifdef DEBUG
			cout << "Worker: '" << worker << "' Path: '" << path << "'" << endl;
#endif // DEBUG

			Request request(path);
			Worker::work(worker.c_str(),request,response);

		}

	} catch(const system_error &e) {

		int code = sysErrorToHttp(e.code().value());
		mg_send_http_error(conn, code, e.what());
		return code;

	} catch(const exception &e) {

		mg_send_http_error(conn, 500, e.what());
		return 500;

	}

	string rsp = response.toStyledString();

#ifdef DEBUG
	cout << "Response:" << endl << rsp << endl;
#endif // DEBUG

	mg_send_http_ok(conn, "application/json; charset=utf-8", rsp.size());
	mg_write(conn, rsp.c_str(), rsp.size());

	return 200;

 }


