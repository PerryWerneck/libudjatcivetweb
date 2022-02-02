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
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/http/exception.h>
 #include <cstring>

 int webHandler(struct mg_connection *conn, function<string (const string &uri, const char *method, const MimeType mimetype)> worker) noexcept {

	const struct mg_request_info *ri = mg_get_request_info(conn);
	MimeType mimetype{MimeType::json};
	string rsp;

	try {

		const char *local_uri = ri->local_uri;

		// Extract 'API' prefix.
		if(strncasecmp(local_uri,"/api/",5) == 0) {
			local_uri += 5;
		} else {
			throw HTTP::Exception( 400, ri->local_uri, "Request must be in the format /api/version/worker/path");
		}

		// Extract version prefix.
		{
			const char *ptr = strchr(local_uri,'/');
			if(!ptr) {
				throw HTTP::Exception( 400, ri->local_uri, "Request must be in the format /api/version/worker/path");
			}
			local_uri = ptr+1;
		}

		// Extract mimetype
		string uri = local_uri;
		{
			auto ext = uri.find_last_of('.');
			if(ext != string::npos && ext > 1) {
				mimetype = MimeTypeFactory(uri.c_str()+ext+1);
				uri.resize(ext);
			}
		}

		rsp = worker(uri,ri->request_method,mimetype);

	} catch(const HTTP::Exception &error) {

		mg_send_http_error(conn, error.codes().http, error.what());
		return error.codes().http;

	} catch(const system_error &e) {

		int code = HTTP::Exception::translate(e);
		mg_send_http_error(conn, code, e.what());
		return code;

	} catch(const exception &e) {

		mg_send_http_error(conn, 500, e.what());
		return 500;

	} catch(...) {

		mg_send_http_error(conn, 500, "Unexpected error");
		return 500;

	}

#ifdef DEBUG
	cout << "Response:" << endl << rsp << endl;
#endif // DEBUG

	mg_send_http_ok(conn, to_string(mimetype), rsp.size());
	mg_write(conn, rsp.c_str(), rsp.size());

	return 200;

 }
