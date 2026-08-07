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

 #include <config.h>
 #include <udjat/tools/civetweb/connection.h>
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/intl.h>
 #include <string>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	HTTP::StatusCode CivetWeb::Connection::send(const char *name, time_t maxage, const MimeType mimetype) noexcept {

		HTTP::Status status{HTTP::Ok,mimetype};

		if(!(name && *name)) {

			status.assign(HTTP::SystemError,_("Empty filename on request"));
			return send(status,false);

		}

		string filename{name};
		if(filename[filename.size()-1] == '/') {
			filename.resize(filename.size()-1);
		}

		status.mimetype = MimeTypeFactory(filename.c_str());

		struct stat st;
		if(stat(filename.c_str(), &st) < 0) {

			Logger::String{filename.c_str(),": ",strerror(errno)}.error();
			status.assign(HTTP::NotFound);
			return send(status,false);

		}

		if(!S_ISREG(st.st_mode)) {
		
			status.assign(HTTP::NotFound);
			error(status.code,String{filename.c_str()," is not a regular file"}.c_str());
			return send(status,false);

		}

		status.timestamp.modification = st.st_mtime;

		//
		// It's a file, send it.
		//
		info(HTTP::Ok,filename.c_str());

		mg_response_header_start(conn, HTTP::Ok);
		mg_response_header_add(conn, "Content-Length", std::to_string(st.st_size).c_str(), -1);

		// https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Repr-Digest
		// calculate md5
		// mg_response_header_add(conn, "Repr-Digest", String{"md5=",md5.c_str()}.c_str(), -1);

		status.timestamp.expiration = time(0)+maxage;
		status.http_headers([this](const char *name, const char *value){
			mg_response_header_add(conn, name, value, -1);
		});

		auth.http_headers([this](const char *name, const char *value){
			mg_response_header_add(conn, name, value, -1);
		});

		mg_response_header_send(conn);

		mg_send_file_body(conn,filename.c_str());

		return HTTP::Ok;
	}

	HTTP::StatusCode CivetWeb::Connection::redirect(const char *location) const {

		if(Logger::enabled(Logger::Debug)) {
			const struct mg_request_info *request_info = mg_get_request_info(conn);
			info(
				HTTP::SeeOther,
				String{
					request_info->local_uri,
					" redirected to ",
					location
				}.c_str()
			);
		}

		mg_response_header_start(conn, (int) HTTP::SeeOther);
		mg_response_header_add(conn, "Location",location,-1);
		mg_response_header_add(conn, "Content-Length", "0", -1);

		auth.http_headers([this](const char *name, const char *value){
			mg_response_header_add(conn, name, value, -1);
		});

		mg_response_header_send(conn);

		return HTTP::SeeOther;
	}

 }

