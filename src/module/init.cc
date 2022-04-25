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
 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 static int log_message(const struct mg_connection *conn, const char *message);
 static int http_error( struct mg_connection *conn, int status, const char *msg );

 static const Udjat::ModuleInfo moduleinfo{ "CivetWEB " CIVETWEB_VERSION " HTTP module for " STRINGIZE_VALUE_OF(PRODUCT_NAME) };

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
	{ "http2",			MG_FEATURES_HTTP2,				false	},
	{ "domain",			MG_FEATURES_X_DOMAIN_SOCKET,	false	},
	{ "all",			MG_FEATURES_ALL,				false	},

 };

 class Module : public Udjat::Module, public MainLoop::Service {
 private:
	struct mg_context *ctx = nullptr;

	struct {
		CivetWeb::Protocol http{"http",moduleinfo};
		CivetWeb::Protocol https{"https",moduleinfo};
	} protocols;

	void setHandlers() noexcept {
		mg_set_request_handler(ctx, "/api/", apiWebHandler, 0);
		mg_set_request_handler(ctx, "/report/", reportWebHandler, 0);
		mg_set_request_handler(ctx, "/swagger.json", swaggerWebHandler, 0);
	}

	void setCallbacks(struct mg_callbacks &callbacks) noexcept {
		memset(&callbacks,0,sizeof(callbacks));
		callbacks.log_message = log_message;
		callbacks.http_error = http_error;
	}

	void initialize(const std::vector<string> &optionlist) {

		struct mg_callbacks callbacks;
		setCallbacks(callbacks);


		if(optionlist.empty()) {

			// Use default options
			cerr << "civetweb\tNo civetweb configuration, using defaults" << endl;

			static const char *options[] = {
				"listening_ports","8989",
				"request_timeout_ms","10000",
				"enable_auth_domain_check","no",
				NULL
			};

			ctx = mg_start(&callbacks, this, options);

		} else {

			// Use configured options.
			cerr << "civetweb\tFound civetweb configuration, using it" << endl;

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
			cerr << "civetweb\tCannot start: mg_start failed." << endl;
			return;
		}

		setHandlers();

	}

 public:

 	Module(const pugi::xml_node &node) : Udjat::Module("httpd",moduleinfo), MainLoop::Service(moduleinfo), ctx(NULL) {

		unsigned int init = 0;

		{
			string info{"civetweb\tFeatures:"};
			for(size_t ix = 0; ix < (sizeof(features)/sizeof(features[0]));ix++) {
				if(node.attribute(features[ix].name).as_bool(features[ix].def)) {
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

		if(optionlist.empty()) {

			clog << "civetweb\tUsing default settings" << endl;

			static const char *default_options[] = {
				"listening_ports",			"8989",
				"request_timeout_ms",		"10000",
				"enable_auth_domain_check",	"no"
			};

			for(size_t ix = 0; ix < (sizeof(default_options)/sizeof(default_options[0])); ix++) {
				optionlist.emplace_back(default_options[ix]);
			}

		} else {

			cout << "civetweb\tUsing settings from XML definition" << endl;

		}

		initialize(optionlist);

 	}

 	Module() : Udjat::Module("httpd",moduleinfo), MainLoop::Service(moduleinfo), ctx(NULL) {

 		unsigned int init = 0;

		cout << "civetweb\tUsing settings from configuration file" << endl;

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

		// https://github.com/civetweb/civetweb/blob/master/docs/api/mg_start.md
		std::vector<string> optionlist;

		Config::for_each("civetweb-options",[&optionlist](const char *key, const char *value){
			optionlist.emplace_back(key);
			optionlist.emplace_back(value);
			return true;
		});

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
			cout << "civetweb\tListening on";
			for(int ix = 0; ix < count;ix++) {
				cout << " " << ports[ix].port;
				if(ports[ix].is_ssl) {
					cout << " (ssl)";
				}
			}
			cout << endl;
		}


		// mg_check_feature()
	}

	/*
	void stop() noexcept override {
#ifdef DEBUG
		cout << "civetweb\t*** Stopping" << endl;
#endif // DEBUG
	}
	*/

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

 int http_error( struct mg_connection *conn, int status, const char *message ) {

	Udjat::MimeType mimetype = (Udjat::MimeType) 0;

	const struct mg_request_info *request_info = mg_get_request_info(conn);

	//
	// Search for mime-type on request headers.
	//
	for(int ix=0;ix<request_info->num_headers;ix++) {
		cout << request_info->http_headers[ix].name << "=" << request_info->http_headers[ix].value << endl;
	}

	if(!mimetype && request_info->local_uri_raw && *request_info->local_uri_raw) {
		//
		// Not found on headers, try by the path
		//
		const char *ptr = strrchr(request_info->local_uri_raw,'.');
		if(ptr) {
			mimetype = MimeTypeFactory(ptr+1);
		}
	}

	clog << "civetweb\t" << request_info->remote_addr << " " << status << " " << message << " (" << mimetype << ")" << endl;

	try {

		string response;

#ifdef DEBUG
		string page = "templates/error.";
		page +=  + to_string(mimetype,true);
#else
		string page = Application::DataDir("www/templates") + "error." + to_string(mimetype,true);
#endif // DEBUG

		if(!access(page.c_str(),R_OK)) {
			response = File::Text(page.c_str()).c_str();
		} else if(mimetype == Udjat::json) {
			response = "{\"error\":{\"application\":\"${application}\",\"code\":${code},\"message\":\"${message}\"}}";
		} else {
			cout << "civetweb\tNo access to '" << page << "' using default response" << endl;
		}

		if(!response.empty()) {

			Udjat::expand(response,[&status,&message](const char *key, std::string &value){

				if(!strcasecmp(key,"code")) {
					value = to_string(status);
				} else if(!strcasecmp(key,"message")) {
					value = message;
				} else if(!strcasecmp(key,"application")) {
					value = Application::Name();
				} else {
					value.clear();
				}

				return true;
			});

			mg_printf(
				conn,
				"HTTP/1.1 %d %s\r\n"
				"Content-Type: %s\r\n"
				"Content-Length: %u\r\n"
				"\r\n"
				"%s",
				status,message,
				std::to_string(mimetype),
				(unsigned int) response.size(),
				response.c_str()
			);
			return 0;

		}

	} catch(const std::exception &e) {
		cerr << "civetweb\tError '" << e.what() << "' processing error page using default" << endl;
	} catch(...) {
		cerr << "civetweb\tUnexpected error processing error page using default" << endl;
	}

	return 1;

 }

