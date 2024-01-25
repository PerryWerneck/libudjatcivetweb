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
  * @brief Implements Request::exec().
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/report.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/worker.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	int HTTP::Request::exec(HTTP::Connection &connection) {

		Worker::ResponseType response_type = Worker::None;
		const Worker *worker = nullptr;

		Worker::for_each([&worker,&response_type,this](const Worker &w){
			response_type = w.probe(*this);
			if(response_type != Worker::None) {
				worker = &w;
				return true;
			}
			return false;
		});

		switch(response_type) {
		case Worker::None:
			return connection.send(HTTP::Response{(MimeType) connection}.failed(ENOENT));

		case Worker::Value:
			{
				if(connection == MimeType::csv) {
					throw system_error(ENOENT,system_category(),"Unsupported output format");
				}

				HTTP::Response response{(MimeType) connection};
				if(!worker->work(*this,response)) {
					Logger::String("Request has failed").trace("civetweb");
					response.failed(ENOENT);
					return connection.send(response);
				}
				return connection.send(response);
			}

		case Worker::Table:
			{
				HTTP::Report response{(MimeType) connection};
				if(!worker->work(*this,response)) {
					Logger::String("Request has failed").trace("civetweb");
					response.failed(ENOENT);
					return connection.send(response);
				}
				return connection.send(response);
			}

		case Worker::Both:
			{
				size_t output_format = Abstract::Object::getProperty("output-format","detailed").select("list","combined",nullptr);

				if(connection == MimeType::csv || output_format == 0) {

					// List
					HTTP::Report response{(MimeType) connection};
					if(!worker->work(*this,response)) {
						Logger::String("Request has failed").trace("civetweb");
						response.failed(ENOENT);
						return connection.send(response);
					}
					return connection.send(response);

				} else if(output_format == 1) {

					// Combined
					throw system_error(ENOTSUP,system_category(),"Unsupported output format");

				} else {

					// All others
					HTTP::Response response{(MimeType) connection};
					if(!worker->work(*this,response)) {
						Logger::String("Request has failed").trace("civetweb");
						response.failed(ENOENT);
						return connection.send(response);
					}
					return connection.send(response);

				}

			}

		}

		return connection.send(HTTP::Response{(MimeType) connection}.failed(ENOENT));

	}

	String HTTP::Request::cookie(const char *) const {
		return "";
	}

	/*
	String HTTP::Request::address() const {

		String proxy{getProperty("X-Forwarded-For")};
		if(!proxy.empty()) {
			// Get proxy
			auto separator = proxy.find(',');
			if(separator != string::npos) {
				proxy.resize(separator);
			}
			return proxy;
		}

		return "";
	}
	*/

 }
