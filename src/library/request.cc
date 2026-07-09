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
 #include <udjat/tools/request.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/http/request.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/http/template.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/string.h>
 #include <cstring>

 #include <stdexcept>

 using namespace std; 

 namespace Udjat {

	HTTP::Request::~Request() {
	}

	bool HTTP::Request::getProperty(const char *key, std::string &value) const {

		if(!strcasecmp(key,"client-address")) {
			value = address();
			return true;
		}

		if(!strcasecmp(key,"client-id")) {
			value = Config::Value<string>{"authentication","client-id"};
			if(value.empty()) {
				throw logic_error("The required client-id for authentication is empty");
			}
			return true;
		}

		if(!strcasecmp(key,"redirect-uri")) {
			value = uri();
			return true;
		}

		if(!strcasecmp(key,"redirect-uri")) {
			Config::Value<Udjat::String> uri{"authentication","redirect-uri"};
			if(value.empty()) {
				throw logic_error("The required redirect-uri for authentication is empty");
			}
			value = uri.escape();
			return true;
		}
	
		
		return Udjat::Request::getProperty(key,value);
	}

	String HTTP::Request::cookie(const char *) const {
		return "";
	}

	Udjat::String HTTP::Request::session_cookie() const {
		return cookie(Udjat::String{Application::Name().c_str(),"-session"}.c_str());
	}

	// int HTTP::Request::send(int code, const char *text) const {
	// 	return code;
	// }

	bool HTTP::Request::for_each(const std::function<bool(const char *name, const char *value)> &call) const {

		if(call("client-address",address().c_str())) {
			return true;
		}

		return Udjat::Request::for_each(call);
	}

	bool HTTP::Request::cached(const Udjat::TimeStamp &timestamp) const {

		HTTP::TimeStamp	reqtime{header("If-Modified-Since")};

		if(reqtime && ((time_t) reqtime) >= ((time_t) timestamp)) {
			return true;
		}

		return Udjat::Request::cached(timestamp);
	}

	bool HTTP::Request::html() const noexcept {
		return !api_call && mimetype() == MimeType::html;		
	}

	MimeType HTTP::Request::mimetype() const noexcept {

		// Legacy header.
		const char *remote_request = header("X-RemoteRequest");
		if(remote_request && *remote_request) {
			auto mime = MimeTypeFactory(remote_request);
			if(mime != MimeType::none) {
				return mime;
			}
		}

		// Use 'accept' header to identify response type.
		for(const Udjat::String &value : Udjat::String{header("accept")}.split(",")) {
			auto mime = MimeTypeFactory(value.c_str(),MimeType::none);
			if(mime != MimeType::none) {
				return mime;
			}
		}

		return MimeType::none;
	}

	void HTTP::Request::parse_query(const char *query) {

		if(!(query && *query)) {
			return;
		}

		debug("Parsing query '",query,"'");

		for(const Udjat::String &value : Udjat::String{query}.unescape().split("&")) {
			debug(value.c_str());
			const char *ptr = strchr(value.c_str(),'=');
			if(ptr) {
				debug("request[",string{value.c_str(),(size_t) (ptr-value.c_str())}.c_str(),"]='",(ptr+1),"'");
				(*this)[string{value.c_str(),(size_t) (ptr-value.c_str())}.c_str()] = (const char *) (ptr+1);
			} else {
				debug("request[",value.c_str(),"]='true'");
				(*this)[value.c_str()] = true;
			}
		}

	}

	int HTTP::Request::failed(int code, const char *message, const char *body) const {

		if(api_call || !Config::Value<bool>("http","use-error-templates",true)) {

			// Format API call response.
			HTTP::Response response{mimetype()};
			response.failed(
				Logger::Message{_("HTTP Error {}"),code}.c_str(),
				message,
				body
			);

			return send(code,response.to_string().c_str());

		}

		Template response{"error",mimetype()};
		if(response.empty()) {

			// Empty template, Format API call response.
			HTTP::Response response{mimetype()};
			response.failed(
				Logger::Message{_("HTTP Error {}"),code}.c_str(),
				message,
				body
			);

			return send(code,response.to_string().c_str());

		}

		response.expand([&](const char *key, std::string &value){

			if(!(strcasecmp(key,"code") && strcasecmp(key,"error-code"))) {

				value = std::to_string(code);

			} else if(!strcasecmp(key,"message")) {

				value = message;

			} else if(!strcasecmp(key,"body")) {

				value = body;

			} else if(!strcasecmp(key,"syscode")) {

				value = body;

			} else {

				return false;

			}

			return true;
		});

		response.expand(*this);

		return send(code,response.c_str());

	}

 }
