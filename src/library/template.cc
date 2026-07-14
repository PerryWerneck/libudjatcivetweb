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

 /**
  * @brief Implement template pages.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/template.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <fcntl.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	HTTP::Template::Template(const char *name, const MimeType mimetype) {

		if(mimetype == MimeType::none) {
			throw runtime_error("Template parsing requires a MIME type.");
		}

		filepath = Config::Value<string>{"httpd","template-path"};

		if(filepath.empty()) {
			filepath = Config::Value<string>{"httpd","root-path"};
			if(!filepath.empty()) {
				filepath += "templates/";
			}
		}

		if(filepath.empty()) {
			filepath = Application::DataFile{"templates/www/"};
		}

		filepath.append(
			name,".",std::to_string(mimetype,true)
		);

	}

	void HTTP::Template::apply(int code, std::ostream &stream, const std::function<void(const char *key, std::ostream &stream)> &callback) {

		if(!filepath) {
			Logger::Message{"The file '{}' is unavailable within the selected theme",filepath.c_str()}.error();
			throw runtime_error(
				_("A required file is unavailable within the selected theme. Please contact the system administrator for assistance.")
			);
		}
		
		string text = filepath.load();

		// Build response.
		const char *ptr = text.c_str();

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

					stream << Application::Name();

				} else if(!strcasecmp(key.c_str(),"css-path")) {

					stream 
						<< Config::Value<std::string>{"theme","http-root","/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/"}.c_str()
						<< "css/style.css";

				} else if(!strcasecmp(key.c_str(),"code")) {

					Logger::String{"Using obsolete '%{code}' on template"}.warning();
					stream << code;

				} else if(!strcasecmp(key.c_str(),"status-code")) {

					stream << code;

				} else {

					callback(key.c_str(),stream);

				}

			} else {
				size_t len = strlen(ptr);
				stream.write(ptr,len);
				ptr += len;
			}

		}

	}

	// HTTP::Template::Template(const char *name, const MimeType mimetype) {

	// 	debug(__FUNCTION__,"(",name,",",std::to_string(mimetype),")");

	// 	if(mimetype == MimeType::none) {
	// 		return;
	// 	}

	// 	File::Path filename = Config::Value<String>{"httpd","template-path"};
	// 	if(filename.empty()) {
	// 		filename = Config::Value<String>{"httpd","root-path"};
	// 		if(!filename.empty()) {
	// 			filename += "templates/";
	// 		}
	// 	}
	// 	if(filename.empty()) {
	// 		filename = Application::DataFile{"templates/www/"};
	// 	}

	// 	filename += name;
	// 	filename += ".";
	// 	filename += std::to_string(mimetype,true);

	// 	if(!filename) {
	// 		Logger::String{"Cant find template '",filename.c_str(),"' (",std::to_string(mimetype),")"}.trace("http");
	// 		return;
	// 	}

	// 	Logger::String{"Loading template from '",filename.c_str(),"'"}.trace("http");
	// 	assign(filename.load());

    //     expand([](const char *key, std::string &value) {

 	// 		if(!strcasecmp(key,"app-name")) {
	// 			value = Application::Name();
	// 			return true;
 	// 		}

	// 		if(!strcasecmp(key,"css-path")) {

	// 			value = String {
	// 				Config::Value<std::string>{"theme","http-root","/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/"}.c_str(),
	// 				"css/style.css"
	// 			};

	// 			return true;
	// 		}

	// 		return false;

    //     },false,false);


	// }

 }
