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
 #include <udjat/tools/response.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/value.h>
 #include <udjat/tools/http/value.h>
 #include <udjat/tools/http/layouts.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/template.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/icon.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/http/timestamp.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #include <iostream>
 #include <sstream>

 using namespace std;

 namespace Udjat {

	int HTTP::Response::status_code() const noexcept {
		return HTTP::Exception::code(Udjat::Response::status_code());
	}

	void HTTP::Response::for_each(const std::function<void(const char *header_name, const char *header_value)> &call) const noexcept {

		// https://stackoverflow.com/questions/3715981/what-s-the-best-restful-method-to-return-total-number-of-items-in-an-object
		if(range.count) {
			call("X-Total-Count",std::to_string(range.count).c_str());
		}

		if(range.total) {
			call("Content-Range",Udjat::String{"items ",range.from,"-",range.to,"/",range.total}.c_str());
		}

		/*
		if(timestamp.expires) {

		}
		*/

	}

	std::string HTTP::Response::to_string() const noexcept {

		// TODO: FIX-IT

		/*
		int code = status_code();	
		debug("Request status code is ",code);

		if(code >= 400 && code <= 599 && empty() && Config::Value<bool>("http","use-error-templates",true)) {

			try {

				HTTP::Template text{"error", (MimeType) *this};

				if(text) {

					text.expand([code,this](const char *key, std::string &value){

						if(!strcasecmp(key,"code")) {

							value = std::to_string(code);

						} else if(!strcasecmp(key,"message")) {

							value = this->status.message;

						} else if(!strcasecmp(key,"body")) {

							value = this->body();
#ifdef DEBUG
							if(!*this->body()) {
								value = "No body on this error (DEBUG)";
							}
#endif // DEBUG
						} else if(!strcasecmp(key,"syscode")) {

							value = std::to_string(this->status_code());

						} else {

							return false;

						}

						return true;

					});

					return text;

				}

			} catch(const std::exception &e) {

				Logger::String{e.what()}.error("http");

			}
		}
		*/

		try {

			if(mimetype == MimeType::svg) {

				// It's an svg
				HTTP::Icon icon;

				Udjat::Response::for_each([&icon](const char *, const Udjat::Value &value){
					if(value == Udjat::Value::Icon) {
						icon = HTTP::Icon::getInstance(value.to_string());
						return (bool) icon;
					}
					return false;

				});

				if(!icon) {
					throw system_error(ENOENT,system_category(),"No icon here");
				}

				debug("Sending icon '",icon.c_str(),"'");

				return string{File::Text{icon.c_str()}.c_str()};

			}

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error("http");
			const_cast<HTTP::Response *>(this)->failed(e);

		} catch(...) {

			Logger::String message{_( "Unexpected error processing response text" )};
			message.error("http");
			const_cast<HTTP::Response *>(this)->failed(message.c_str());

		}

		return Udjat::Response::to_string();

	}

 }
