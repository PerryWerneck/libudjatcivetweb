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

 Protocol::Protocol(const Quark &name, const Quark &portname, int s) : Udjat::URL::Protocol(name,portname), use_ssl(s) {
 }

 Protocol::~Protocol() {
 }

 std::shared_ptr<URL::Response> Protocol::call(const URL &url, const URL::Method method, const char *mimetype, const char *payload) {

	std::shared_ptr<URL::Response> response;

	char error_buffer[256] = "";
	const char *filename = url.getFileName();

	string header{	"Connection: close\r\n"
					"User-Agent: " STRINGIZE_VALUE_OF(PRODUCT_NAME) "\r\n"
					"Host: "
				};

	header += url.getDomainName();
	header += "\r\n";

	if(mimetype && *mimetype) {
		header += "Accept: ";
		header += mimetype;
		header += "\r\n";
	}


#ifdef DEBUG
	cout << header << endl;
#endif // DEBUG

	//
	// GET /home.html HTTP/1.1
	// Host: developer.mozilla.org
	// User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:50.0) Gecko/20100101 Firefox/50.0
	// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
	// Accept-Language: en-US,en;q=0.5
	// Accept-Encoding: gzip, deflate, br
	// Referer: https://developer.mozilla.org/testpage.html
	// Connection: keep-alive
	// Upgrade-Insecure-Requests: 1
	// If-Modified-Since: Mon, 18 Jul 2016 02:36:04 GMT
	// If-None-Match: "c561c68d0ba92bbeb8b0fff2a9199f722e3a621a"
	// Cache-Control: max-age=0
	//
	/*
	if(this->maxage) {

			rsp     << "Cache-Control: " << (auth.isAuthenticated() ? "private" : "public") << ", max-age=" << string().set("%u",(unsigned int) maxage) << "\r\n"
					<< "Expires: " << getTimeString(this->timestamp + maxage) << "\r\n"
					<< "Vary: Accept-Encoding\r\n";

	} else {

			rsp     << "Cache-Control: no-cache, no-store\r\n"
					<< "Pragma: no-cache\r\n"
					<< "Expires: 0\r\n";
	}
	*/

 	struct mg_connection *conn =
		mg_download(
			url.getDomainName(),
			url.getPortNumber(),
			use_ssl,
			error_buffer,
			sizeof(error_buffer),
			"GET %s HTTP/1.0\r\n%s\r\n",
			(filename && *filename ? filename : "/"),
			header.c_str()
		);

	if(!conn) {
		throw runtime_error(error_buffer);
	}

	try {

		response = make_shared<::URLResponse>(conn);

	} catch(...) {

		mg_close_connection(conn);
		throw;
	}

	mg_close_connection(conn);

	return response;
 }
