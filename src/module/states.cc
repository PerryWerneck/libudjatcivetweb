/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements the swagger.json output.
  *
  * References:
  *
  * https://samanthaneilen.github.io/2018/12/08/Using-and-extending-swagger.json-for-API-documentation.html
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/agent/state.h>
 #include <cstring>

 #include "private.h"

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

 int stateWebHandler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {

	try {

		MimeType mimetype{MimeType::json};
		string uri{mg_get_request_info(conn)->local_uri};
		{
			auto ext = uri.find_last_of('.');
			if(ext != string::npos && ext > 1) {
				mimetype = MimeTypeFactory(uri.c_str()+ext+1);
				uri.resize(ext);
			}
		}

		if(!strncasecmp(uri.c_str(),"/state",6)) {
			uri.erase(0,6);
		}

		debug("PATH=[",uri.c_str(),"] mymetype=[",std::to_string(mimetype),"]");

		HTTP::Response response{mimetype};
		if(!Abstract::State::getProperties(uri.c_str(), response)) {
			mg_send_http_error(conn, 404, strerror(ENOENT));
		}

		string rsp{response.to_string()};
		return CivetWeb::Connection{conn}.success(to_string(mimetype),rsp.c_str(),rsp.size());


	} catch(const HTTP::Exception &error) {

		mg_send_http_error(conn, error.codes().http, "%s", error.what());
		return error.codes().http;

	} catch(const system_error &e) {

		int code = HTTP::Exception::translate(e);
		mg_send_http_error(conn, code, "%s", e.what());
		return code;

	} catch(const exception &e) {

		mg_send_http_error(conn, 500, "%s",  e.what());
		return 500;

	} catch(...) {

		mg_send_http_error(conn, 500, "Unexpected error");
		return 500;

	}

	mg_send_http_error(conn, 404, strerror(ENOENT));
	return 404;

 }
