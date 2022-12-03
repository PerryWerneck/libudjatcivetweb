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
 #include <udjat/defs.h>
 #include <udjat/module.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/expander.h>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/http/handler.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 static int log_message(const struct mg_connection *conn, const char *message);

 const Udjat::ModuleInfo udjat_module_info{ "CivetWEB " CIVETWEB_VERSION " HTTP module for " STRINGIZE_VALUE_OF(PRODUCT_NAME) };

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


 class Module : public Udjat::Module, public MainLoop::Service, public HTTP::Server {
 private:
	struct mg_context *ctx = nullptr;

	struct {
		CivetWeb::Protocol http{"http",udjat_module_info};
		CivetWeb::Protocol https{"https",udjat_module_info};
	} protocols;

	void setHandlers() noexcept {
		mg_set_request_handler(ctx, "/api/", apiWebHandler, 0);
		mg_set_request_handler(ctx, "/icon/", iconWebHandler, 0);
//		mg_set_request_handler(ctx, "/report/", reportWebHandler, 0);
		mg_set_request_handler(ctx, "/swagger.json", swaggerWebHandler, 0);

		{
			mg_set_request_handler(ctx, "/", rootWebHandler, 0);

		}

	}

	void setCallbacks(struct mg_callbacks &callbacks) noexcept {
		memset(&callbacks,0,sizeof(callbacks));
		callbacks.log_message = log_message;
		callbacks.http_error = http_error;
	}

	void initialize(std::vector<string> &optionlist) {

		struct mg_callbacks callbacks;
		setCallbacks(callbacks);

		if(optionlist.empty()) {

			Config::for_each("civetweb-options",[&optionlist](const char *key, const char *value){
				optionlist.emplace_back(key);
				optionlist.emplace_back(value);
				return true;
			});

		}

		// https://github.com/civetweb/civetweb/blob/master/docs/api/mg_start.md

		if(optionlist.empty()) {

			// Use default options
			cerr << "civetweb\tNo civetweb configuration, using defaults" << endl;

			static const char *options[] = {
				"listening_ports","localhost:8989",
				"request_timeout_ms","10000",
				"enable_auth_domain_check","no",
				NULL
			};

			ctx = mg_start(&callbacks, this, options);

		} else {

			// Use configured options.
			clog << "civetweb\tFound civetweb configuration, using it" << endl;

			for(size_t ix = 0; ix < optionlist.size(); ix++) {
				if(!strcasecmp(optionlist[ix].c_str(),"listening_ports")) {
					break;
				}
			}

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
			Udjat::Module::error() << "Cannot start: mg_start failed." << endl;
			return;
		}

		setHandlers();

	}

 public:

 	Module(const pugi::xml_node &node) : Udjat::Module("httpd",udjat_module_info), MainLoop::Service(udjat_module_info), ctx(NULL) {

		unsigned int init = 0;

		{
			string info{"civetweb\tFeatures:"};
			for(size_t ix = 0; ix < (sizeof(features)/sizeof(features[0]));ix++) {

				pugi::xml_attribute attr = node.attribute(features[ix].name);
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
			cout << info << endl;
		}

 		mg_init_library(init);

		// https://github.com/civetweb/civetweb/blob/master/docs/api/mg_start.md
		std::vector<string> optionlist;

		options(node, [&optionlist](const char *name, const char *value){
			optionlist.emplace_back(name);
			optionlist.emplace_back(value);
		});

		initialize(optionlist);

 	}

 	Module() : Udjat::Module("httpd",udjat_module_info), MainLoop::Service(udjat_module_info), ctx(NULL) {

 		unsigned int init = 0;

		{
			string info{"civetweb\tFeatures:"};
			for(size_t ix = 0; ix < (sizeof(features)/sizeof(features[0]));ix++) {
				if(Config::Value<bool>("civetweb-features",features[ix].name,features[ix].def)) {
					init |= features[ix].flag;
					info += " ";
					info += features[ix].name;
				}
			}
			cout << info << endl;
		}

		mg_init_library(init);

		std::vector<string> optionlist;
		initialize(optionlist);

	};

 	virtual ~Module() {

		cout << "civetweb\tStopping service" << endl;

		if(ctx) {
			mg_stop(ctx);
		}

		mg_exit_library();

 	}

	void start() noexcept override {

		struct mg_server_port ports[10];

		int count = mg_get_server_ports(ctx,10,ports);

		if(count) {
			for(int ix = 0; ix < count;ix++) {
				Logger::String(
					"Listening on ",
					(ports[ix].is_ssl ? "https" : "http"),
					"://",
					(ports[ix].protocol == 1 ? "127.0.0.1" : "localhost"),
					":",
					ports[ix].port
				).write(Logger::Trace,"civetweb");
			}
			if(Logger::enabled(Logger::Debug)) {

				Logger::String(
					"Application state available on ",
					(ports[0].is_ssl ? "https" : "http"),
					"://",
					(ports[0].protocol == 1 ? "127.0.0.1" : "localhost"),
					":",
					ports[0].port,
					"/api/1.0/agent.html"
				).write(Logger::Debug,"civetweb");

				auto module = Module::find("information");
				String options;
				if(module && module->getProperty("options",options)) {
					for(const std::string &option : options.split(",")) {
						Logger::String(
							"Service info available on ",
							(ports[0].is_ssl ? "https" : "http"),
							"://",
							(ports[0].protocol == 1 ? "127.0.0.1" : "localhost"),
							":",
							ports[0].port,
							"/api/1.0/info/",
							option.c_str(),
							".html"
						).write(Logger::Debug,"civetweb");
					}
				}

			}
		}


		// mg_check_feature()
	}

	bool push_back(HTTP::Handler *handler) override {
		if(HTTP::Server::push_back(handler)) {

			string uri{handler->c_str()};

			if(uri[uri.size()-1] == '/') {
				uri.resize(uri.size()-1);
			}

			mg_set_request_handler(ctx, uri.c_str(), customWebHandler, handler);

			if(Logger::enabled(Logger::Debug)) {

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
					}.write(Logger::Debug,"civetweb");

				} else {
					Logger::String{"Request handler for '",uri,"' was activated"}.write(Logger::Debug,"civetweb");
				}

			}

			return true;
		}
		return false;
	}

	bool remove(HTTP::Handler *handler) override {
		if(HTTP::Server::remove(handler)) {
			cout << "civetweb\tRemoving http handle '" << handler->c_str() << "'" << endl;
			mg_set_request_handler(ctx, handler->c_str(), NULL, NULL);
			return true;
		}
		return false;
	}

 };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {
 	return new ::Module();
 }

 Udjat::Module * udjat_module_init_from_xml(const pugi::xml_node &node) {
	return new ::Module(node);
 }

 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wunused-parameter"
 int log_message(const struct mg_connection *conn, const char *message) {
	clog << "civetweb\t" << message << endl;
	return 1;
 }
 #pragma GCC diagnostic pop


