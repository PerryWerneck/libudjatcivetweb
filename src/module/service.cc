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

 #define UDJAT_ENABLE_CIVETWEB
 #include <udjat/tools/civetweb/service.h>

 #include <udjat/defs.h>
 #include <udjat/module/abstract.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/worker.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/handler.h>
 #include <string>

 #include <civetweb.h>

 #include <private/module.h>

 using namespace std;

 static const struct {
	const char *name;
	unsigned int flag;
	bool def;
 } features[] = {

	{ "files",			MG_FEATURES_FILES,				false	},
	{ "tls", 			MG_FEATURES_TLS,				true	},
	{ "cgi", 			MG_FEATURES_CGI,				false	},
	{ "ipv6", 			MG_FEATURES_IPV6,				true	},
	{ "websocket",		MG_FEATURES_WEBSOCKET,			false	},
	{ "lua",			MG_FEATURES_LUA,				false	},
	{ "ssjs",			MG_FEATURES_SSJS,				false	},
	{ "cache",			MG_FEATURES_CACHE,				true	},
	{ "stats",			MG_FEATURES_STATS,				false	},
	{ "compression",	MG_FEATURES_COMPRESSION,		true	},
#ifdef MG_FEATURES_HTTP2
	{ "http2",			MG_FEATURES_HTTP2,				false	},
#endif // MG_FEATURES_HTTP2
#ifdef MG_FEATURES_X_DOMAIN_SOCKET
	{ "domain",			MG_FEATURES_X_DOMAIN_SOCKET,	false	},
#endif // MG_FEATURES_X_DOMAIN_SOCKET
	{ "all",			MG_FEATURES_ALL,				false	},

 };

 static int log_message(const struct mg_connection *conn, const char *message);
 
 namespace Udjat {

	CivetWeb::Service * CivetWeb::Service::instance = nullptr;

	CivetWeb::Service & CivetWeb::Service::get_instance() {
		if(instance) {
			throw runtime_error("HTTP server is undefined");
		}
		return *instance;
	}

 	CivetWeb::Service::Service(const ModuleInfo &info, const pugi::xml_node &node) 
		: Udjat::Service{String{node,"name",info.name}.as_quark(), info} {

		if(instance) {
			throw runtime_error("HTTP server is already defined");
		}

		// Init library from XML definitions
		{
			unsigned int init = 0;
			Logger::String info{"CivetWeb Features: "};
			for(size_t ix = 0; ix < (sizeof(features)/sizeof(features[0]));ix++) {

				XML::Attribute attr{XML::AttributeFactory(node,features[ix].name)};
				if(attr) {
					if(attr.as_bool(features[ix].def)) {
						init |= features[ix].flag;
						info += " ";
						info += features[ix].name;
					}
				} else if(Config::Value<bool>("civetweb-features",features[ix].name,features[ix].def)) {
					init |= features[ix].flag;
					info += " ";
					info += features[ix].name;
				}
			}
			mg_init_library(init);		
		}

		// Start service
		{

			// https://github.com/civetweb/civetweb/blob/master/docs/api/mg_start.md
			std::vector<string> optionlist;

			XML::options(node, [&optionlist](const char *name, const char *value){
				optionlist.emplace_back(name);
				optionlist.emplace_back(value);
			});

			struct mg_callbacks callbacks;
			memset(&callbacks,0,sizeof(callbacks));
			callbacks.log_message = log_message;
			callbacks.http_error = http_error;

			if(optionlist.empty()) {

				// Use default options
				Logger::String{"No civetweb configuration, using defaults"}.trace(Service::name());

				static const char *options[] = {
					"listening_ports","localhost:8989",
					"request_timeout_ms","10000",
					"enable_auth_domain_check","no",
					NULL
				};

				ctx = mg_start(&callbacks, this, options);

			} else {

				// Use options
				Logger::String{"Found civetweb configuration, using it"}.trace(Service::name());

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

			mg_set_request_handler(ctx, "/icon/", iconWebHandler, 0);
			mg_set_request_handler(ctx, "/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/", productWebHandler, 0);
			mg_set_request_handler(ctx, "/image/", imageWebHandler, 0);
			mg_set_request_handler(ctx, "/favicon.ico", faviconWebHandler, 0);

#ifdef HAVE_LIBSSL
			mg_set_request_handler(ctx, "/pubkey.pem", keyWebHandler, 0);
			if(Config::Value<bool>{"oauth2","enable-internal",false}) {
				mg_set_request_handler(ctx, "/oauth2", oauthWebHandler, 0);
			}
#endif // HAVE_LIBSSL

			// All other requests goes to rootwebhandler
			mg_set_request_handler(ctx, "/", rootWebHandler, 0);

		}

	}

 	CivetWeb::Service::~Service() {

		Logger::String{"Stopping service"}.trace(name());
		instance = nullptr;

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

#ifdef HAVE_LIBSSL
					Logger::String{"Public key available on ",baseref,"/pubkey.pem"}.write(Logger::Trace,"civetweb");
					if(Config::Value<bool>{"oauth2","enable-internal",false}) {
						Logger::String{"OAuth2 service available on ",baseref,"/oauth2"}.write(Logger::Trace,"civetweb");
					}
#endif // HAVE_LIBSSL

					baseref += "/api/";
					baseref += Config::Value<string>{"http","apiversion",PACKAGE_VERSION};
					baseref += "/";

					Udjat::Worker::for_each([this,&baseref](const Udjat::Worker &worker){
						if(!strcasecmp(worker.c_str(),"agent")) {
							Logger::String{"Application state available on ",baseref,"agent"}.trace(name());
							return true;
						}
						return false;
					});

					Udjat::Module::for_each([&baseref](const Udjat::Module &module){
						module.trace_paths(baseref.c_str());
						return false;
					});


				}

			}

		} else {

			Logger::String{"No input ports (mg_get_server_ports has returned ",count,")"}.error("civetweb");

		}

		Udjat::Service::start();

	}

	void CivetWeb::Service::stop() noexcept {

		Udjat::Service::stop();
	}

	bool CivetWeb::Service::push_back(HTTP::Handler *handler) {

		string uri{handler->c_str()};

		if(uri[uri.size()-1] == '/') {
			uri.resize(uri.size()-1);
		}

		mg_set_request_handler(ctx, uri.c_str(), customWebHandler, handler);

		if(Logger::enabled(Logger::Trace)) {

			struct mg_server_port ports[10];
			if(mg_get_server_ports(ctx,10,ports) > 0) {

				Logger::String{
					"New request handler was activated on ",
					(ports[0].is_ssl ? "https" : "http"),
					"://",
					(ports[0].protocol == 1 ? "127.0.0.1" : "localhost"),
					":",
					ports[0].port,
					uri
				}.write(Logger::Trace,"civetweb");

			}

		} else {
			Logger::String{"Custom handler for '",handler->c_str(),"' added"}.info("civetweb");
		}

		return true;

	}

	bool CivetWeb::Service::remove(HTTP::Handler *handler) {

		string uri{handler->c_str()};

		if(uri[uri.size()-1] == '/') {
			uri.resize(uri.size()-1);
		}

		mg_set_request_handler(ctx, uri.c_str(), NULL, NULL);
		Logger::String{"Custom handler for '",handler->c_str(),"' removed"}.info("civetweb");

		return true;

	}

 }

 int log_message(const struct mg_connection *, const char *message) {
	Logger::String{message}.info("civetweb");
	return 1;
 }

