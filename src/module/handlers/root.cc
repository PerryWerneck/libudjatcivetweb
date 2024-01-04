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

 /**
  * @brief Implements the default index page.
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/module.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/report.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/worker.h>
 #include <private/request.h>

 using namespace std;
 using namespace Udjat;

 int rootWebHandler(struct mg_connection *conn, void *) {

	CivetWeb::Connection connection{conn};
	CivetWeb::Request request{conn};

	size_t output_format = request.getArgument("output-format","detailed").select("detailed","list","combined",nullptr);

	if(output_format == 1 || connection ==  MimeType::csv) {

		// List
		HTTP::Report response{(MimeType) connection};

		if(!Udjat::exec(request,response)) {
			response.failed(ENOENT);
		}

		return connection.send(response);

	} else {

		// Detailed or combined.
		HTTP::Response response{(MimeType) connection};

		if(!Udjat::exec(request,response)) {
			response.failed(ENOENT);
		}

		// TODO: If not failed and output_format == 2 append report on response.

		return connection.send(response);

	}


	/*
	if(response.empty()) {

		const struct mg_request_info *request_info = mg_get_request_info(conn);

		Logger::String{
			request_info->remote_addr," ",
			request_info->request_method," ",
			request_info->local_uri," ",
			" - Empty response"
		}.warning("civetweb");

	}
	*/

	/*

 	try {

#ifdef DEBUG
		{
			auto info = mg_get_request_info(conn);
			debug("local-uri='",info->local_uri,"'");
		}
#endif // DEBUG

		MimeType mimetype = (MimeType) connection;
		string response = CivetWeb::Request{conn}.exec(mimetype);

		if(response.empty()) {

			// TODO: Send 'empty response' status.
			connection.failed(204, "Empty response");

		} else {

			return connection.success(to_string(mimetype),response.c_str(),response.size());

		}


	} catch(const HTTP::Exception &error) {

		cerr << "civetweb\t" << error.what() << endl;
		return connection.response(error);

	} catch(const system_error &error) {

		cerr << "civetweb\t" << error.what() << endl;
		return connection.response(error);

	} catch(const exception &error) {

		cerr << "civetweb\t" << error.what() << endl;
		return connection.response(error);

	} catch(...) {

		cerr << "civetweb\tUnexpected error" << endl;
		connection.failed(500, "Unexpected error");
		return 500;

	}

	return 500;
	*/
 }
