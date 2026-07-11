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
 #include <sstream>

 using namespace std;

 namespace Udjat {

	int HTTP::Connection::send_template(int code, const char *tmplt, const std::function<void(const char *key, std::ostream &writer)> &callback) const {

		MimeType mimetype{(MimeType) *this};
		if(mimetype == MimeType::none) {
			throw runtime_error("Unexpected connection mimetype");
		}

		// Load template
		string text;

		{
			File::Path filename = Config::Value<string>{"httpd","template-path"};
			if(filename.empty()) {
				filename = Config::Value<string>{"httpd","root-path"};
				if(!filename.empty()) {
					filename += "templates/";
				}
			}
			if(filename.empty()) {
				filename = Application::DataFile{"templates/www/"};
			}

			filename += tmplt;
			filename += ".";
			filename += std::to_string(mimetype,true);

			if(!filename) {
				throw runtime_error(String{"Cant find template '",filename.c_str(),"' (",std::to_string(mimetype),")"});
			}

			Logger::String{"Loading template from '",filename.c_str(),"'"}.trace();
			text.assign(filename.load());
		}

		// Build response.
		const char *ptr = text.c_str();
		stringstream stream;

		while(*ptr) {

			const char *mark = strstr(ptr,"${");
			if(mark) {

				size_t len = mark - ptr;
				stream.write(ptr,len);
				ptr += len;

				mark += 2;
				ptr = strstr(mark,"}");
				if(!ptr) {
					throw runtime_error("Malformed variable definition due to a missing '}' bracket");
				}
				std::string key{mark,(size_t) (ptr-mark)};
				ptr++;

				if(!strcasecmp(key.c_str(),"app-name")) {

					stream 
						<< Application::Name();

				} else if(!strcasecmp(key.c_str(),"css-path")) {

					stream 
						<< Config::Value<std::string>{"theme","http-root","/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/"}.c_str()
						<< "css/style.css";

				} else {

					callback(key.c_str(),stream);

				}


			} else {
				size_t len = strlen(ptr);
				stream.write(ptr,len);
				ptr += len;
			}

		}

		std::string payload = stream.str(); 
		return send(
			code, 
			std::to_string(mimetype),
			payload.c_str(),
			payload.size()
		);

	}


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