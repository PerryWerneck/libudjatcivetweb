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
  * @brief Implements the pubkey output.
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <udjat/tools/http/keypair.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <string>

 using namespace std;

 int keyWebHandler(struct mg_connection *conn, void *) {

#ifdef HAVE_LIBSSL

	try {

		HTTP::KeyPair &keys = HTTP::KeyPair::getInstance();

		if(keys) {

			String result{keys.to_string()};
			size_t length = result.size();

			mg_response_header_start(conn, 200);
			mg_response_header_add(conn, "Content-Type",std::to_string(MimeType::pem),-1);
			mg_response_header_add(conn, "Content-Length", std::to_string(length).c_str(), -1);
			mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
			mg_response_header_add(conn, "Expires", "0", -1);

 			mg_response_header_send(conn);
			mg_write(conn, result.c_str(), length);

			return 200;

		}

		mg_send_http_error(conn, 404, _("No pubkey"));
		return 404;


	} catch(const exception &e) {

		mg_send_http_error(conn, 500, "%s", e.what());
		return 500;

	} catch(...) {

		mg_send_http_error(conn, 500, "%s", _("Unexpected error"));
		return 500;

	}


#else

	mg_send_http_error(conn, 404, _("Unsupported method"));

#endif // HAVE_LIBSSL

	mg_send_http_error(conn, 404, _("Not available"));
	return 404;

 }
