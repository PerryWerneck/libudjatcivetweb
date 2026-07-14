/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/intl.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/template.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/file/path.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {


	/// @brief Run method, handle exceptions.
	int HTTP::Connection::handle() noexcept {

		debug("--- ",__FUNCTION__," ---");

		try {

			// Find interface.

			// Prepare response.

			if(apicall()) {

				// It's an API call, call interface.
				throw runtime_error("Incomplete");

			}

			// It's not an API call.
			return send_template(200,"main",[](const char *key, std::ostream &stream){
				
				debug("Replacing ---[[[",key,"]]]---");
				if(!strcasecmp(key,"page-title")) {

					// TODO: Send the page title
					stream << "Page title"; 

				} else if(!strcasecmp(key,"navbar")) {

					// TODO: Send the navbar

				} else if(!strcasecmp(key,"page-summary")) {
					
					// TODO: Send the page summary

				} else if(!strcasecmp(key,"page-contents")) {

					// TODO: Send the page contents.

				} else {

					Logger::String{"Unexpected key '",key,"' processing template"}.error();
					throw runtime_error(_("We couldn't process your request due to a system error. Please contact the administrator."));

				}

			});

		} catch(const HTTP::Exception &e) {

			return failed(
				e.code(),
				_("We're sorry, but we encountered an error while processing your request."),
				e.what()
				);

		// } catch(const system_error &e) {

		// 	return failed(
		// 		500,
		// 		_("We're sorry, but we encountered an error while processing your request."),
		// 		e.what()
		// 	);

		} catch(const std::exception &e) {

			return failed(
				500,
				_("We're sorry, but we encountered an error while processing your request."),
				e.what()
			);

		} catch(...) {

			return failed(
				500,
				_("We're sorry, but we encountered an error while processing your request."),
				_("Unexpected error")
			);

		}

	}


 }