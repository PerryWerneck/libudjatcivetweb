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

 #include <udjat/defs.h>
 #include <udjat/module.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/configuration.h>
 #include <civetweb.h>

 using namespace Udjat;
 using namespace std;

 static int log_message(const struct mg_connection *conn, const char *message);


 class Module : public Udjat::Module {
 private:
	struct mg_context *ctx;

 public:

 	Module(void *handle) : Udjat::Module(Quark::getFromStatic("civetweb"),handle), ctx(NULL) {
		mg_init_library(0);
 	};

 	virtual ~Module() {
		stop();
		mg_exit_library();
 	}

	void start() override {

		if(!ctx) {

			const char *options[] = {
				"listening_ports", 			Config::Value<std::string>("civetweb","listening_ports","8989").c_str(),
				"request_timeout_ms",		Config::Value<std::string>("civetweb","request_timeout_ms","10000").c_str(),
				"error_log_file",			Config::Value<std::string>("civetweb","error_log_file","error.log").c_str(),
				"enable_auth_domain_check",	Config::Value<std::string>("civetweb","enable_auth_domain_check","no").c_str(),
				NULL
			};

			// https://github.com/civetweb/civetweb/blob/master/docs/api/mg_start.md
			struct mg_callbacks callbacks;
			memset(&callbacks,0,sizeof(callbacks));
			callbacks.log_message = log_message;

			ctx = mg_start(&callbacks, 0, options);
			if (ctx == NULL) {
				throw runtime_error("Cannot start CivetWeb - mg_start failed.");
			}

		}

	}

	void stop() override {

		if(ctx) {
			mg_stop(ctx);
		}

		ctx = NULL;
	}

 };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init(void *handle) {
	return new ::Module(handle);
 }

 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wunused-parameter"
 int log_message(const struct mg_connection *conn, const char *message) {
	clog << "civetweb\t" << message << endl;
	return 1;
 }
 #pragma GCC diagnostic pop

