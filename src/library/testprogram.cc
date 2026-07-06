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
 #include <udjat/tools/unit-test.h>
 #include <string>
 
 #include <private/client.h>
 
 using namespace Udjat;
 using namespace std;

 #ifdef DEBUG 

 UDJAT_API void enum_udjat_unit_tests(Udjat::UnitTests &tests) noexcept {

	debug(__FUNCTION__," begin -> ",tests.size());

	tests.append(
		UnitTests::Worker{
			"httpclient", "Test HTTP client",
			[]() {

				Udjat::URL url{"http://127.0.0.1/udjat/css/style.css"};
				auto handler = CivetWeb::Client::Factory{
									"http",
									"CivetWEB " CIVETWEB_VERSION " HTTP module for " STRINGIZE_VALUE_OF(PRODUCT_NAME)
								}.HandlerFactory(url);

				auto response = handler->get("/tmp/style.css");

				cout << "-----" << endl << response << endl << "-----" << endl;

				return true;
			}
		}
	);
 }


#endif // DEBUG
