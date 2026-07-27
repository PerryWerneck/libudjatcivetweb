/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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

 #define LOG_DOMAIN "civetweb"

 #include <udjat/tools/civetweb/service.h>

 #include <udjat/defs.h>
 #include <udjat/module.h>
 #include <private/module.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/civetweb/connection.h>
 #include <udjat/tools/http/status.h>
 #include <string>
 #include <udjat/authentication.h>

 #include <civetweb.h>

 #include <private/module.h>
 #include <private/request.h>

 using namespace std;

 static const struct {
	const char *name;
	unsigned int flag;
	bool def;
	const char *message;
 } features[] = {

	// https://github.com/civetweb/civetweb/blob/master/docs/api/mg_init_library.md

	{ "files",			MG_FEATURES_FILES,				false,	"This server is able to serve files."			},
	{ "tls", 			MG_FEATURES_TLS,				true,	"Support for HTTPS is active."					},
	{ "cgi", 			MG_FEATURES_CGI,				false,	"CGI scripts can be called by this webserver."	},
	{ "ipv6", 			MG_FEATURES_IPV6,				true,	"Support IPv6 is active."	},
	{ "websocket",		MG_FEATURES_WEBSOCKET,			false,	"Supporting web sockets."	},
	{ "lua",			MG_FEATURES_LUA,				false,	"Support for Lua scripts and Lua server pages is active."	},
	{ "ssjs",			MG_FEATURES_SSJS,				false,	"Support for server side JavaScript."	},
	{ "cache",			MG_FEATURES_CACHE,				true,	"Support for caching is enabled."	},
	{ "stats",			MG_FEATURES_STATS,				false,	"This web server will collect data for server statistics."	},
	{ "compression",	MG_FEATURES_COMPRESSION,		true,	"This web server may use ZLIB for on the fly data compression."	},
#ifdef MG_FEATURES_HTTP2
	{ "http2",			MG_FEATURES_HTTP2,				false,	"This web server will accept HTTP/2 connections over HTTPS."	},
#endif // MG_FEATURES_HTTP2
#ifdef MG_FEATURES_X_DOMAIN_SOCKET
	{ "domain",			MG_FEATURES_X_DOMAIN_SOCKET,	false,	"This web server will allow to bind to domain sockets, in addition to TCP sockets."	},
#endif // MG_FEATURES_X_DOMAIN_SOCKET
	{ "all",			MG_FEATURES_ALL,				false,	nullptr	},

 };

 static int log_message(const struct mg_connection *conn, const char *message);
 static int http_error(struct mg_connection *conn, int code, const char *message) noexcept;

 namespace Udjat {

 	CivetWeb::Service::Service(const Udjat::Properties &props) 
		: Udjat::Service{props.get("name","http").as_quark(),props.get("description","CivetWEB " CIVETWEB_VERSION " HTTP module for " STRINGIZE_VALUE_OF(PRODUCT_NAME)).as_quark()} {

		// Init library
		{
			unsigned int init = 0;
			Logger::String info{"CivetWeb Features: "};
			for(const auto feature : features) {

				if(props.contains(feature.name)) {
					if(props.get(feature.name,feature.def)) {
						init |= feature.flag;
						info += " ";
						info += feature.name;
					}
				} else if(Config::Value<bool>("civetweb-features",feature.name,feature.def)) {
					init |= feature.flag;
					info += " ";
					info += feature.name;
				}

			}
			auto enabled_features = mg_init_library(init);	
			for(const auto feature : features) {
				if((feature.flag != MG_FEATURES_ALL) && (enabled_features & feature.flag)) {
					if(feature.message) {
						Logger::String{feature.message}.info(name());
					} else {
						Logger::String{"Feature '",feature.name,"' is active"}.info(name());
					}
				}
			}
	
		}

		// Start service
		{

			// https://github.com/civetweb/civetweb/blob/master/docs/api/mg_start.md
			std::vector<string> optionlist;

			props.for_each_child("option",[this,&optionlist](const Properties &property){

				auto name = property["name"];
				auto value = property["value"];

				Logger::String{"Option ",name.c_str(),"='",value.c_str(),"'"}.trace(this->name());

				if(name.empty() || value.empty()) {
					Logger::String{"Invalid option on '",property.path(),"'"}.warning(this->name());
				} else {
					optionlist.emplace_back(name);
					optionlist.emplace_back(value);
				}

				return false;
			});

			struct mg_callbacks callbacks;
			memset(&callbacks,0,sizeof(callbacks));
			callbacks.log_message = log_message;
			callbacks.http_error = http_error;

			if(optionlist.empty()) {

				// Use default options
				Logger::String{"No civetweb configuration, using defaults"}.info(name());

				static const char *options[] = {
					"listening_ports","localhost:8989",
					"request_timeout_ms","10000",
					"enable_auth_domain_check","no",
					NULL
				};

				ctx = mg_start(&callbacks, this, options);

			} else {

				// Use options
				Logger::String{"Found civetweb configuration, using it"}.trace(name());

				const char **options = new const char *[optionlist.size()+1];
				size_t ix = 0;
				for(const string & option : optionlist) {
					options[ix++] = option.c_str();
				}
				options[ix] = NULL;

				ctx = mg_start(&callbacks, this, options);
				delete[] options;

			}

			if (ctx == NULL) {
				throw runtime_error("mg_start failed.");
			}

			// mg_set_request_handler(ctx, "/api/", (mg_request_handler) api_handler, this);

			// TODO: Refactor as interfaces.
			mg_set_request_handler(ctx, "/icon/", (mg_request_handler) icon_handler, this);
			mg_set_request_handler(ctx, "/theme/", (mg_request_handler) theme_handler, this);
			// mg_set_request_handler(ctx, "/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/", (mg_request_handler) product_handler, this);
			mg_set_request_handler(ctx, "/image/", (mg_request_handler) image_handler, this);
			mg_set_request_handler(ctx, "/favicon.ico", (mg_request_handler) favicon_handler, this);

			if(Authentication::available()) {
				mg_set_request_handler(ctx, "/oauth2", (mg_request_handler) auth_handler, this);
			}

			mg_set_request_handler(ctx, "/user", (mg_request_handler) user_handler, this);

			mg_set_request_handler(ctx, "/", (mg_request_handler) default_handler, this);

		}

	}

 	CivetWeb::Service::~Service() {

		Logger::String{"Stopping service"}.trace(name());

		if(ctx) {
			mg_stop(ctx);
		}

		mg_exit_library();

 	}

	void CivetWeb::Service::start() noexcept {

		struct mg_server_port ports[10];

		int count = mg_get_server_ports(ctx,10,ports);
		if(count > 0) {

			for(int ix = 0; ix < count;ix++) {

				if(ports[ix].port <= 0) {
					continue;
				}

				String baseref{
					(ports[ix].is_ssl ? "https" : "http"),
					"://",
					(ports[ix].protocol == 1 ? "127.0.0.1" : "localhost"),
					":",
					ports[ix].port
				};

				Logger::String{"Listening on ",baseref.c_str()}.info(name());

				if(Logger::enabled(Logger::Trace)) {

					if(Authentication::available() && !strcasecmp(Config::Value<string>{"authentication","engine","undefined"}.c_str(),"internal")) {
						Logger::String{"Authentication service available on ",baseref,"/oauth2"}.trace();
					}

				}

			}

		} else {

			Logger::String{"No input ports (mg_get_server_ports has returned ",count,")"}.error();

		}

		Udjat::Service::start();

	}

	void CivetWeb::Service::stop() noexcept {

		Udjat::Service::stop();
	}

 }

 int log_message(const struct mg_connection *, const char *message) {
	Logger::String{message}.info("civetweb");
	return 1;
 }

 int http_error(struct mg_connection *conn, int code, const char *message) noexcept {

	debug("Callback ",__FUNCTION__," called, formatting output");
	
	CivetWeb::Connection connection{conn};
	connection.error((HTTP::StatusCode) code, message);

	HTTP::Status status{connection.mimetype()};
	status.assign(
		(HTTP::StatusCode) code,
		message
	);

	connection.send(
		status,
		strncasecmp("/api/",mg_get_request_info(conn)->local_uri,5) == 0
	);

	return 0;
 }

