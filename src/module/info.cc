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
 #include <udjat/module.h>

 int infoWebHandler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {

	const struct mg_request_info *ri = mg_get_request_info(conn);

	if(strcasecmp(ri->request_method,"get")) {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}

	Udjat::Response response;

	try {

		const char *uri = ri->local_uri;

		// Extract 'API' prefix.
		if(strncasecmp(uri,"/info/",6) == 0) {
			uri += 6;
		} else {
			mg_send_http_error(conn, 400, "Request must be in the format /info/version/name");
			return 400;
		}

		// Extract version prefix.
		{
			const char *ptr = strchr(uri,'/');
			if(!ptr) {
				mg_send_http_error(conn, 400, "Request must be in the format /info/version/name");
				return 400;
			}
			uri = ptr+1;
		}

		cout << "URI: '" << uri << "'" << endl;

		if(!strcasecmp(uri,"modules")) {

			Module::getInfo(response);

		} else if(!strcasecmp(uri,"workers")) {

			Worker::getInfo(response);

		} else if(!strcasecmp(uri,"factory")) {

			Abstract::Agent::Factory::getInfo(response);

		} else {
			mg_send_http_error(conn, 404, (string{"I don't know nothing about '"} + uri + "'").c_str());
			return 404;
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

	cout << "Response:" << endl << rsp << endl;

	mg_send_http_ok(conn, "application/json; charset=utf-8", rsp.size());
	mg_write(conn, rsp.c_str(), rsp.size());

	return 200;

 }


