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

 static const Udjat::ModuleInfo moduleinfo{"CivetWEB " CIVETWEB_VERSION " HTTP module for " STRINGIZE_VALUE_OF(PRODUCT_NAME) };

 class Module : public Udjat::Module, public MainLoop::Service {
 private:
	struct mg_context *ctx;

	struct {
		CivetWeb::Protocol http{"http",moduleinfo,0};
		CivetWeb::Protocol https{"https",moduleinfo,1};
	} protocols;

 public:

 	Module() : Udjat::Module("httpd",moduleinfo), MainLoop::Service(moduleinfo), ctx(NULL) {

		mg_init_library(0);

 	};

 	virtual ~Module() {

		mg_exit_library();

 	}

	void start() noexcept override {

		cout << "civetweb\tStarting service" << endl;

		if(!ctx) {

			// Start
			// https://github.com/civetweb/civetweb/blob/master/docs/api/mg_start.md
			{
				std::vector<string> optionlist;

				Config::for_each("civetweb-options",[&optionlist](const char *key, const char *value){
					optionlist.emplace_back(key);
					optionlist.emplace_back(value);
#ifdef DEBUG
					cout << "civetweb\t" << key << "= '" << value << "'" << endl;
#endif // DEBUG
					return true;
				});

				if(optionlist.empty()) {
					cerr << "civetweb\tNo civetweb configuration" << endl;
					return;
				}

				const char **options = new const char *[optionlist.size()+1];
				size_t ix = 0;
				for(string & option : optionlist) {
					options[ix++] = option.c_str();
				}
				options[ix] = NULL;

				struct mg_callbacks callbacks;
				memset(&callbacks,0,sizeof(callbacks));
				callbacks.log_message = log_message;
				callbacks.http_error = http_error;

				ctx = mg_start(&callbacks, this, options);

				delete[] options;

				if (ctx == NULL) {
					cerr << "civetweb\tCannot start: mg_start failed." << endl;
					return;
				}
			}

			mg_set_request_handler(ctx, "/api/", apiWebHandler, 0);
			mg_set_request_handler(ctx, "/report/", reportWebHandler, 0);
			mg_set_request_handler(ctx, "/swagger.json", swaggerWebHandler, 0);

			// cout << "civetweb\tListening on port " << options[1] << endl;

		}

	}

	void stop() noexcept override {

		cout << "civetweb\tStopping service" << endl;

		if(ctx) {
			mg_stop(ctx);
		}

		ctx = NULL;
	}

 };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {
	return new ::Module();
 }

 bool udjat_module_deinit() {

	// Can't unload this module because of the http/https protocol modules.
	return false;

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

