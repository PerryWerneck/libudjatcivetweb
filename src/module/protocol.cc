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

 using namespace Udjat;

 CivetWeb::Protocol::Protocol(const char *name, const ModuleInfo &info, int ssl) : Udjat::Protocol(name,info), use_ssl(ssl) {
 }

 CivetWeb::Protocol::~Protocol() {
 }

 Udjat::String CivetWeb::Protocol::call(const URL &url, const HTTP::Method method, const char *payload) const {

	URL::Components components = url.ComponentsFactory();

	string header{	"Connection: close\r\n"
					"User-Agent: " STRINGIZE_VALUE_OF(PRODUCT_NAME) "\r\n"
					"Host: "
				};

	header += components.hostname;
	header += "\r\n";

	char error_buffer[256] = "";
 	struct mg_connection *conn =
		mg_download(
			components.hostname.c_str(),
			components.portnumber(),
			use_ssl,
			error_buffer,
			sizeof(error_buffer),
			"%s %s HTTP/1.0\r\n%s\r\n%s",
			std::to_string(method),
			(components.path.empty() ? "/" : components.path.c_str()),
			header.c_str(),
			payload
		);

	if(!conn) {
		throw runtime_error(error_buffer);
	}

	Udjat::String response;

	try {

		const struct mg_response_info *info = mg_get_response_info(conn);

		// cout << "civetweb\tServer response was '" << info->status_code << " " << info->status_code << "'" << endl;

		if(info->status_code < 200 || info->status_code > 299) {

			throw HTTP::Exception(info->status_code, url.c_str(), info->status_text);

		} else if((unsigned int) info->content_length >= response.max_size()) {

			throw system_error(E2BIG,system_category(),"The response is too big for current implementation");

		} else if(info->content_length > 0) {

			response.reserve(info->content_length+1);

			// Has data, read it.
			char * buffer = new char [info->content_length + 1];

			int szRead = mg_read(conn, (void *) buffer, info->content_length);

			if(szRead != info->content_length) {
				delete[] buffer;
				throw runtime_error(string{"Got '"} + to_string(szRead) + "' when expecting '" + to_string(info->content_length) + "'");
			}

			buffer[info->content_length] = 0;
			response.assign(buffer);
			delete[] buffer;

		}

	} catch(...) {

		mg_close_connection(conn);
		throw;
	}

	mg_close_connection(conn);

	return response;
 }

 /*
 Protocol::Protocol(const char *name, const char *portname, const ModuleInfo *info, int s) : Udjat::URL::Protocol(name,portname,info), use_ssl(s) {
 }

 Protocol::~Protocol() {
 }

 std::shared_ptr<URL::Response> Protocol::call(const URL &url, const URL::Method method, const char *mimetype, const char *payload) {

	if(!method.equal(URL::Method::Get)) {
		throw system_error(ENOTSUP,system_category(),"Unsupported URL method");
	}

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
	// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,-/-;q=0.8
	// Accept-Language: en-US,en;q=0.5
	// Accept-Encoding: gzip, deflate, br
	// Referer: https://developer.mozilla.org/testpage.html
	// Connection: keep-alive
	// Upgrade-Insecure-Requests: 1
	// If-Modified-Since: Mon, 18 Jul 2016 02:36:04 GMT
	// If-None-Match: "c561c68d0ba92bbeb8b0fff2a9199f722e3a621a"
	// Cache-Control: max-age=0
	//
	if(this->maxage) {

			rsp     << "Cache-Control: " << (auth.isAuthenticated() ? "private" : "public") << ", max-age=" << string().set("%u",(unsigned int) maxage) << "\r\n"
					<< "Expires: " << getTimeString(this->timestamp + maxage) << "\r\n"
					<< "Vary: Accept-Encoding\r\n";

	} else {

			rsp     << "Cache-Control: no-cache, no-store\r\n"
					<< "Pragma: no-cache\r\n"
					<< "Expires: 0\r\n";
	}

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
*/
