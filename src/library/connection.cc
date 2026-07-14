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
 #include <stdexcept>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/template.h>
 #include <udjat/tools/intl.h>
 #include <sstream>

 using namespace std;

 namespace Udjat {

	HTTP::Connection::Connection() {
	}

	HTTP::Connection::~Connection() {
	}

	int HTTP::Connection::success(const char *mime_type, const char *response, size_t length) const noexcept {
		return send(mime_type,response,length);
	}

	int HTTP::Connection::send(const std::exception &e) {
		return send(Response::Status{e});
	}

	std::shared_ptr<HTTP::Response> HTTP::Connection::ResponseFactory() {
		return make_shared<HTTP::Response>((MimeType) *this);
	}

	int HTTP::Connection::failed(int code, const char *message, const char *body) const noexcept {
		Response::Status status{Response::Failure};
		status.message = message;
		status.body = body;
		return send(code,status);
	}

	int HTTP::Connection::send(const Udjat::HTTP::Response::Status &status) const noexcept {
		return send(
			HTTP::Exception::code(status.syscode),
			status			
		);
	}

	int HTTP::Connection::send(int code, const Udjat::HTTP::Response::Status &status) const noexcept {

		try {

			if(apicall()) {

				// API call, format using response.
				MimeType mimetype = (MimeType) *this;
				string text = status.to_string(mimetype);

				return send(
					code, 
					std::to_string(mimetype),
					text.c_str(), 
					text.size()
				);

			}

			// Send HTML formatted page.
			return send_template(code,"error",[code,&status](const char *key, std::ostream &stream) {

				if(!strcasecmp(key,"message")) {
					stream << status.message;
				}
				
				if(!strcasecmp(key,"body")) {
					stream << status.body;
				}

				if(!strcasecmp(key,"icon")) {
					stream << "/icon/computer-fail-symbolic";
				}

			});

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error();

		} catch(...) {

			Logger::String{_("Unexpected error while handling HTTP response")}.error();
			
		}

		// Exception, return 500.
		return 500;

	}

	int HTTP::Connection::exec(const std::function<int(HTTP::Connection &connection)> &call) noexcept {

		try {

			return call(*this);

		} catch(const std::exception &e) {
			return send(e);

		} catch(...) {
			Response::Status status{Response::Failure};
			status.message = _("Unexpected error while handling HTTP response");
			return send(status);

		}

	}

	int HTTP::Connection::send_template(int code, const char *tmplt, const std::function<void(const char *key, std::ostream &writer)> &callback) const {

		MimeType mimetype = (MimeType) *this;
		Template text{tmplt,mimetype};

		stringstream stream;
		text.apply(
			code,
			stream,
			callback
		);

		std::string payload = stream.str(); 
		return send(
			code, 
			std::to_string(mimetype),
			payload.c_str(),
			payload.size()
		);

	}

 }

