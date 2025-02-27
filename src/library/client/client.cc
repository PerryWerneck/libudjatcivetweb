/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/defs.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/url/handler.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/socket.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/method.h>

 #include <private/client.h>

 #include <errno.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <system_error>

 #if defined(HAVE_JSON_C)
	#include <json.h>
 #endif // HAVE_JSON_C

 using namespace std;

 namespace Udjat {

	CivetWeb::Client::Factory::Factory(const char *name) : Udjat::URL::Handler::Factory{name} {
	}

	CivetWeb::Client::Factory::~Factory() {
	}

	std::shared_ptr<Udjat::URL::Handler> CivetWeb::Client::Factory::HandlerFactory(const URL &url) const {
		return std::make_shared<CivetWeb::Client>(url);
	}

	CivetWeb::Client::Client(const URL &u) : url{u} {
		headers.request["Connection"] = "close";
		headers.request["host"] = url.hostname();
	}

	CivetWeb::Client::~Client() {
	}

	const char * CivetWeb::Client::c_str() const noexcept {
		return url.c_str();
	}

	CivetWeb::Client::Connection CivetWeb::Client::connect(void) {

		char buffer[256];
		memset(buffer,0,sizeof(buffer));

		Logger::String{"Connecting to ",url.c_str()}.trace();

		struct mg_connection *conn = mg_connect_client(
			url.hostname().c_str(),
			url.port(),
			strcasecmp(url.scheme().c_str(),"https") == 0,
			buffer,
			sizeof(buffer)-1
		);

		if(!conn) {
			throw runtime_error(String{url.c_str(),": ",buffer});
		}

		mg_set_user_connection_data(conn,this);

		return Connection{conn,mg_close_connection};

	}

	URL::Handler & CivetWeb::Client::header(const char *name, const char *value) {
		headers.request[name] = value;
		return *this;
	}

	void CivetWeb::Client::send_headers(Connection &cli, const HTTP::Method method, const char *payload) {

		debug(to_string(method)," ",url.path().c_str());
		mg_printf(cli.get(), "%s %s HTTP/1.1\r\n", to_string(method),url.path().c_str());
		for(const auto & [name,value]: headers.request) {
			mg_printf(cli.get(), "%s: %s\r\n", name.c_str(), value.c_str());
		}

		mg_printf(cli.get(), "\r\n");

		// TODO: Send payload.
		
	}

	int CivetWeb::Client::test(const HTTP::Method method, const char *payload) {

		char buffer[4096];

		try {

			Connection cli = connect();
			send_headers(cli, method, payload);

			int ret = mg_get_response(
					cli.get(),
					buffer,
					sizeof(buffer)-1,
					(Config::Value<time_t>("http","timeout",10) * 1000)
			);

			if (ret < 0) {
				return -1;
			}

			const struct mg_response_info *info = mg_get_response_info(cli.get());

			debug("length=",info->content_length);

			return info->status_code;

		} catch(const exception &e) {
			Logger::String{"Error testing ",url.c_str(),": ",e.what()}.error();
			return -1;
		}


	}

	int CivetWeb::Client::perform(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total, const void *data, size_t len)> &progress) {

		char buffer[4096];

		Connection cli = connect();
		send_headers(cli, method, payload);

		int ret = mg_get_response(
				cli.get(),
				buffer,
				sizeof(buffer)-1,
				(Config::Value<time_t>("http","timeout",10) * 1000)
		);
		if (ret < 0) {
			throw runtime_error(buffer);
		}

		const struct mg_response_info *info = mg_get_response_info(cli.get());

		debug("ret=",ret," status=",info->status_code," message=",info->status_text);
		except(info->status_code,info->status_text);

		if(info->content_length <= 0) {
			progress(0,0,nullptr,0);
			return info->status_code;
		} 

		progress(0,info->content_length,nullptr,0);

		long long current = 0;
		while(current < info->content_length) {

			int szRead = mg_read(cli.get(), (void *) buffer, 4096);

			if(szRead == 0) {
				throw system_error(ENOTCONN,system_category(),"Connection closed while downloading file");
			} else if(szRead < 0) {
				throw runtime_error("Download error");
			} else if(progress(current,info->content_length,buffer,(size_t) szRead)) {
				throw system_error(ECANCELED,system_category());
			}
			current += (uint64_t) szRead;

		}

		return info->status_code;

	}

#if defined(HAVE_JSON_C)

	static void load(Udjat::Value &value, struct json_object *jobj) {

		switch(json_object_get_type(jobj)) {
		case json_type_null:
			break;

		case json_type_boolean:
			value = json_object_get_boolean(jobj);
			break;

		case json_type_double:
			value = json_object_get_double(jobj);
			break;

		case json_type_int:
			value = json_object_get_int(jobj);

			break;

		case json_type_object:
			{
				value.set(Value::Object);
				json_object_object_foreach(jobj, key, val) {
					load(value[(const char *) key],val);
				}
			}
			break;

		case json_type_array:
			{
				value.set(Value::Array);
				int arraylen = json_object_array_length(jobj);
				for (int i = 0; i < arraylen; i++) {
					struct json_object *elem = json_object_array_get_idx(jobj, i);
					load(value[i], elem);
				}
			}
			break;

		case json_type_string:
			value = json_object_get_string(jobj);
			break;

		default:
			value = json_object_get_string(jobj);

		}

	}

	bool HTTP::Handler::get(Udjat::Value &value, const HTTP::Method method, const char *payload) {

		URL::Handler::set(MimeType::json);

		String response{URL::Handler::get(method,payload)};

		if(response.empty()) {
			throw system_error(ENODATA,system_category(),String{"Empty response from ", c_str()});
		}

		struct json_object *jobj = json_tokener_parse(response.c_str());
		if(!jobj) {
			throw runtime_error(String{"Error parsing response from ",c_str()});
		}

		json_object_object_foreach(jobj, key, val) {
			load(value[(const char *) key],val);
		}
		
		json_object_put(jobj);

		return true;
	}
#endif // HAVE_JSON_C

 }

