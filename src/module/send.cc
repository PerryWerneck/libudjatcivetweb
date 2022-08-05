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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/file.h>
 #include <sstream>

 using namespace std;

 namespace Udjat {

	int CivetWeb::Connection::send(const char *name, bool allow_index, const char *mime_type, unsigned int maxage) const {

		if(!(name && *name)) {
			throw system_error(ENOENT,system_category(),"Empty file path");
		}

		string filename{name};
		if(filename[filename.size()-1] == '/') {
			filename.resize(filename.size()-1);
		}

		struct stat st;
		if(stat(filename.c_str(), &st) < 0) {
			throw system_error(errno,system_category(),filename);
		}

		if(S_ISREG(st.st_mode)) {
			//
			// It's a file, send it.
			//
			mg_response_header_start(conn, 200);

			if(maxage) {
				mg_response_header_add(conn, "Cache-Control", (string{"public,max-age="} + std::to_string(maxage) + ",immutable").c_str(), -1);
				mg_response_header_add(conn, "Expires", HTTP::TimeStamp(time(0)+maxage).to_string().c_str(), -1);
			}

			mg_response_header_add(conn, "Last-Modified", HTTP::TimeStamp(st.st_mtime).to_string().c_str(), -1);
			mg_response_header_add(conn, "Content-Length", std::to_string(st.st_size).c_str(), -1);

			if(mime_type && *mime_type) {
				mg_response_header_add(conn, "Content-Type", mime_type, -1);
			}

			mg_response_header_send(conn);

			mg_send_file_body(conn,filename.c_str());

		} else if(S_ISDIR(st.st_mode) && allow_index) {
			//
			// It's a directory
			//

			if(name[strlen(name)-1] != '/') {
				//
				// It's a directory but the URL is requesting a file.
				//
				cerr << "civetweb\tClient has requested directory '" << filename << "' without an ending '/'" << endl;

				/*
				//
				// Redirect to correct url.
				//
				mg_response_header_start(conn, 301);
				mg_response_header_add(conn, "Location", (string{name} + '/').c_str(), -1);
				mg_response_header_send(conn);

				return 301;
				*/

				mg_response_header_start(conn, 400);
				mg_response_header_send(conn);

				return 400;
			}

			//
			// Send index
			//
			mg_response_header_start(conn, 200);

			if(maxage) {
				mg_response_header_add(conn, "Cache-Control", (string{"public,max-age="} + std::to_string(maxage) + ",immutable").c_str(), -1);
				mg_response_header_add(conn, "Expires", HTTP::TimeStamp(time(0)+maxage).to_string().c_str(), -1);
			}
			mg_response_header_add(conn, "Content-Type", std::to_string(MimeType::html), -1);

			stringstream response;

			response	<< "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"><html><head><title>Index of "
						<< filename
						<< "</title></head><body><h1>Index of "
						<< filename
						<< "</h1><hr /><pre>";

			File::Path::for_each(filename.c_str(),[&response](const char *name, bool is_dir) {

				name = strrchr(name,'/');
				if(!name) {
					return true;
				}
				name++;

				if(name[0] == '.') {
					return true;
				}

				response << "<a href=\"" << name;
				if(is_dir) {
					response << "/";
				}
				response	<< "\">"
							<< name
							<< "</a>" << endl;

				return true;
			});

			response << "</pre><hr /></body></html>";

			string str{response.str()};

			mg_response_header_add(conn, "Content-Length", std::to_string(str.size()).c_str(), -1);
			mg_response_header_send(conn);

			mg_write(conn, str.c_str(), str.size());

		} else {
			//
			// Invalid file type.
			//
			cerr << "civetweb\t'" << filename << "' is not allowed" << endl;
			throw system_error(ENOENT,system_category(),filename);

		}

		return 200;
	}

 }

