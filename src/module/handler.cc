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

 /*
 #include <private/module.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/logger.h>
 #include <cstring>

 int webHandler(const CivetWeb::Connection &connection, function<string (const CivetWeb::Connection &connection, const char *path, const char *method, const MimeType mimetype)> worker) noexcept {

	const struct mg_request_info *ri = connection.request_info();
	MimeType mimetype{MimeType::json};
	string rsp;

	try {

		const char *local_uri = ri->local_uri;

		debug("local_uri='",local_uri,"'");

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

		rsp = worker(connection,uri.c_str(),ri->request_method,mimetype);

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

	debug("Response:\n",rsp);

	return connection.success(to_string(mimetype),rsp.c_str(),rsp.size());

 }
 */
