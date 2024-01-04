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

 #include <config.h>
 #include <udjat/civetweb.h>
 #include <udjat/tools/http/handler.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/intl.h>
 #include <private/module.h>
 #include <udjat/tools/logger.h>

 using namespace Udjat;

 int customWebHandler(struct mg_connection *conn, void *cbdata) {

	HTTP::Handler &handler = *((HTTP::Handler *) cbdata);

	CivetWeb::Connection connection{conn};

	const struct mg_request_info *ri = connection.request_info();

	debug("Using custom web handler for '",ri->local_uri,"' request");

	try {

		return handler.handle(
			connection,
			HTTP::Request{ri->local_uri,ri->request_method},
			(MimeType) connection
		);

	} catch(const HTTP::Exception &error) {

		cerr << "civetweb\t" << error.what() << endl;
		HTTP::Response response{(MimeType) connection};
		response.failed(error);
		return connection.send(response);

	} catch(const system_error &error) {

		cerr << "civetweb\t" << error.what() << endl;
		HTTP::Response response{(MimeType) connection};
		response.failed(error);
		return connection.send(response);

	} catch(const exception &error) {

		cerr << "civetweb\t" << error.what() << endl;
		HTTP::Response response{(MimeType) connection};
		response.failed(error);
		return connection.send(response);

	} catch(...) {

		cerr << "civetweb\tUnexpected error" << endl;
		HTTP::Response response{(MimeType) connection};
		response.failed(_("Unexpected error on http handler"));
		return connection.send(response);
	}

	return 500;

 }


