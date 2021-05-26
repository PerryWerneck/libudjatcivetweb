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
 #include <udjat/url.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <tools.h>

 using namespace Udjat;
 using namespace std;

 static int log_message(const struct mg_connection *conn, const char *message);

 static const Udjat::ModuleInfo moduleinfo{
	PACKAGE_NAME,									// The module name.
	"CivetWEB " CIVETWEB_VERSION " HTTP exporter", 	// The module description.
	PACKAGE_VERSION, 								// The module version.
	PACKAGE_URL, 									// The package URL.
	PACKAGE_BUGREPORT 								// The bugreport address.
 };

 class Module : public Udjat::Module, public MainLoop::Service {
 private:
	struct mg_context *ctx;

 public:

 	Module() : Udjat::Module("civetweb",&moduleinfo), MainLoop::Service(&moduleinfo), ctx(NULL) {

		mg_init_library(0);

		Udjat::URL::insert(make_shared<::Protocol>("http","80",&moduleinfo,0));
		Udjat::URL::insert(make_shared<::Protocol>("https","443",&moduleinfo,1));

 	};

 	virtual ~Module() {
		mg_exit_library();
 	}

	void start() noexcept override {

		cout << "civetweb\tStarting service" << endl;

		if(!ctx) {

			Config::Value<std::string> listening_ports{"civetweb","listening_ports","8989"};
			Config::Value<std::string> request_timeout_ms{"civetweb","request_timeout_ms","10000"};
			Config::Value<std::string> error_log_file{"civetweb","error_log_file","error.log"};
			Config::Value<std::string> enable_auth_domain_check{"civetweb","enable_auth_domain_check","no"};

			const char *options[] = {
				"listening_ports", 			listening_ports.c_str(),
				"request_timeout_ms",		request_timeout_ms.c_str(),
				"error_log_file",			error_log_file.c_str(),
				"enable_auth_domain_check",	enable_auth_domain_check.c_str(),
				NULL
			};

			// https://github.com/civetweb/civetweb/blob/master/docs/api/mg_start.md
			struct mg_callbacks callbacks;
			memset(&callbacks,0,sizeof(callbacks));
			callbacks.log_message = log_message;

			ctx = mg_start(&callbacks, 0, options);
			if (ctx == NULL) {
				cerr << "civetweb\tCannot start: mg_start failed." << endl;
				return;
			}

			mg_set_request_handler(ctx, "/api/", apiWebHandler, 0);
			mg_set_request_handler(ctx, "/report/", reportWebHandler, 0);
			mg_set_request_handler(ctx, "/swagger.json", swaggerWebHandler, 0);

			cout << "civetweb\tListening on port " << options[1] << endl;

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

