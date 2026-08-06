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
 #include <udjat/tools/logger.h>
 #include <udjat/module.h>
 #include <udjat/tools/testsuite.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/url/handler.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/civetweb/service.h>
 #include <string>
 
 #include <private/client.h>
 
 using namespace Udjat;
 using namespace std;

 #ifdef DEBUG 

 UDJAT_API void udjat_register_tests(Udjat::TestSuite &testcases) noexcept {

	using Case = TestSuite::Case;

	testcases.add(
		// Case{
		// 	"httpserver", "Test HTTP server",
		// 	[](std::ostream &stream) {

		// 		CivetWeb::Service srvc{Properties{}};

		// 		Service::for_each([](Service &service){
		// 			service.start();
		// 			return false;
		// 		});

		// 		MainLoop::getInstance().run();

		// 		Service::for_each([](Service &service){
		// 			service.stop();
		// 			return false;
		// 		});

		// 		return "Test complete";
				
		// 	}
		// },
		Case{
			"httpclient", "Test HTTP client",
			[](std::ostream &stream) {

				Udjat::URL url{"http://127.0.0.1/udjat/css/style.css"};

				auto handler = CivetWeb::Client::Factory{
									"http",
									"CivetWEB " CIVETWEB_VERSION " HTTP module for " STRINGIZE_VALUE_OF(PRODUCT_NAME)
								}.HandlerFactory(url);

				auto response = handler->get("/tmp/style.css");

				stream << "-----" << endl << response << endl << "-----" << endl;

				return "Got response";
				
			}
		}
	);
 }


#endif // DEBUG
