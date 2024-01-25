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
 #include <udjat/tools/response.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/value.h>
 #include <udjat/tools/http/value.h>
 #include <udjat/tools/http/layouts.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/mimetype.h>
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

	HTTP::Response::Response(Udjat::MimeType mimetype) : Udjat::Response::Value{mimetype}{

		debug("Building HTTP response for ",std::to_string(mimetype));

	}

	HTTP::Response::~Response() {
	}

	int HTTP::Response::status_code() const noexcept {
		return HTTP::Exception::code(Abstract::Response::status_code());
	}

	bool HTTP::Response::empty() const noexcept {
		return children.empty();
	}

	HTTP::Response::operator Type() const noexcept {
		return this->type;
	}

	Udjat::Value & HTTP::Response::set(const char *value, const Type type) {
		Udjat::Value &child{(*this)["value"]};
		child.set(value,type);
		debug("Response value set to '",child.to_string(),"'");
		return child;
	}

	void HTTP::Response::for_each(const std::function<void(const char *header_name, const char *header_value)> &call) const noexcept {

		Abstract::Response::for_each(call);

		// https://stackoverflow.com/questions/3715981/what-s-the-best-restful-method-to-return-total-number-of-items-in-an-object
		if(total_count) {
			call("X-Total-Count",std::to_string(total_count).c_str());
		}

		if(range.total) {
			call("Content-Range",Udjat::String{"items ",range.from,"-",range.to,"/",range.total}.c_str());
		}

	}

	void HTTP::Response::count(size_t value) noexcept {
		total_count = value;
	}

	void HTTP::Response::content_range(size_t from, size_t to, size_t total) noexcept {
		range.from = from;
		range.to = to;
		range.total = total;
	}

	Udjat::Value & HTTP::Response::append(const Type) {
		reset(Value::Array);
		return children[std::to_string((int) children.size()).c_str()];
	}

	Udjat::Value & HTTP::Response::reset(const Udjat::Value::Type type) {

		if(type != Value::Array && type != Value::Object) {
			throw runtime_error(Logger::String{"Cant handle '",std::to_string(type),"' at this level"});
		}

		if(type != this->type) {
			debug("Response reset to '",std::to_string(type),"'");

			this->type = type;
			children.clear();
		}

		return *this;
	}

	bool HTTP::Response::for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const {

		for(const auto& [name, value] : children)	{
			if(call(name.c_str(),(Udjat::Value &) value)) {
				return true;
			}
		}

		return false;
	}

	Udjat::Value & HTTP::Response::operator[](const char *name) {
		return children[name];
	}

	std::string HTTP::Response::to_string() const noexcept {

		try {

			if(mimetype == MimeType::svg) {

				// It's an svg
				HTTP::Icon icon;

				for_each([&icon](const char *, const Udjat::Value &value){
					if(value == Value::Icon) {
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

			return e.what();

		} catch(...) {

			return _( "Unexpected error converting value" );

		}

		return Udjat::Response::Value::to_string();
	}

	std::string HTTP::Response::to_string(const Udjat::Abstract::Response &response, const MimeType mimetype) {

		int code = HTTP::Exception::code(response.status_code());
		std::string text{response.to_string()};

		if(code < 200 || code > 299) {

			// It's an error, check for customized error page.
			try {

				if(Config::Value<bool>("http","use-error-templates",true) && mimetype != MimeType::custom) {

					// Try to load custom error page.
#ifdef DEBUG
					Application::DataFile page{"./templates/error."};
#else
					Application::DataFile page{"templates/www/error."};
#endif // DEBUG
					page += std::to_string(mimetype,true);

					debug("Checking for http error template in file '",page.c_str(),"'");

					if(!access(page.c_str(),R_OK)) {

						Logger::String{"Loading error page from '",page.c_str(),"'"}.trace("civetweb");

						text = page.load().expand([&response,code](const char *key, std::string &value) {

							if(!strcasecmp(key,"code")) {
								value = std::to_string(code);
							} else if(!strcasecmp(key,"message")) {
								value = response.message();
							} else if(!strcasecmp(key,"body")) {
								value = response.body();
#ifdef DEBUG
								if(!*response.body()) {
									value = "No body on this error (DEBUG)";
								}
#endif // DEBUG
							} else if(!strcasecmp(key,"syscode")) {
								value = std::to_string(response.status_code());
							}

							return !value.empty();

						},true,true);

					}
				}

			} catch(const std::exception &e) {

				Logger::String{"Can't load custom error page: ",e.what()}.trace("civetweb");

			}

		}

		// TODO: If text.empty() && mimetype == json format standard json error.
		// https://github.com/omniti-labs/jsend

		return text;
	}

 }
