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
 #include <unistd.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/file.h>
 #include <cstring>
 #include <udjat/tools/configuration.h>
 #include <utime.h>

 using namespace Udjat;
 using namespace std;

 bool CivetWeb::Protocol::get(const URL &url, const char *filename, const std::function<bool(double current, double total)> &progress) const {

	//
	// Get file status.
	//
	struct stat st;

	if(stat(filename,&st) < 0) {

		if(errno != ENOENT) {
			throw system_error(errno,system_category(),Logger::Message("Can't stat '{}'",filename));
		}

		memset(&st,0,sizeof(st));
		st.st_mode = 0644;

	}

	cout << "civetweb\tDownloading '" << filename << "'" << endl;

	//
	// Create header
	//
	URL::Components components = url.ComponentsFactory();

	string header{	"Connection:close\r\n"
					"User-Agent:" STRINGIZE_VALUE_OF(PRODUCT_NAME) "\r\n"
					"Host:"
				};

	header += components.hostname;
	header += "\r\n";

	if(st.st_mtime) {
		header += "If-Modified-Since:";
		header += HTTP::TimeStamp(st.st_mtime).to_string();
		header += "\r\n";
	}

	Config::for_each(
		"download-headers",
		[&header](const char *key, const char *value) {
			header += key;
			header += ":";
			header += value;
			header += "\r\n";
			return true;
		}
	);

	if(!strstr(header.c_str(),"\nCache-Control:")) {
		clog << "civetweb\tWarning: No 'Cache-Control' in http header" << endl;
	}

#ifdef DEBUG
	cout << header << endl;
#endif // DEBUG

	progress(0,0);

	//
	// Create connection
	//
	char error_buffer[256] = "";
 	struct mg_connection *conn =
		mg_download(
			components.hostname.c_str(),
			components.portnumber(),
			use_ssl,
			error_buffer,
			sizeof(error_buffer),
			"%s %s HTTP/1.0\r\n%s\r\n",
			"GET",
			(components.path.empty() ? "/" : components.path.c_str()),
			header.c_str()
		);

	if(!conn) {
		throw runtime_error(error_buffer);
	}

	File::Temporary response{filename};
	bool status = false;

	try {

		const struct mg_response_info *info = mg_get_response_info(conn);

		if(info->status_code == 304) {

			// Not modified.
			cout << "civetweb\tServer response was '" << info->status_code << " " << info->status_text
					<< "' keeping '" << filename << "'" << endl;
			status = false;

		} else if(info->status_code >= 200 && info->status_code <= 299) {

			cout << "civetweb\tServer response was '" << info->status_code << " " << info->status_text
					<< "' updating '" << filename << "'" << endl;

			{
				long long loaded = 0;
				char buffer[4096];

				while(loaded < info->content_length) {

					int szRead = mg_read(conn, (void *) buffer, 4096);

					if(szRead == 0) {
						throw system_error(ENOTCONN,system_category(),"Connection closed while downloading file");
					} else if(szRead < 0) {
						throw runtime_error("Download error");
					} else {
						loaded += (size_t) szRead;
						response.write((void *) buffer, szRead);
						if(!progress((double) loaded, (double) info->content_length)) {
							throw system_error(ECANCELED,system_category());
						}
					}

				}
			}
			status = true;
			response.link(filename);

			// Set filetimestamp
			utimbuf ub;
			ub.actime = time(0);
			ub.modtime = 0;

			for(int header = 0; header < info->num_headers; header++) {

#ifdef DEBUG
				cout << info->http_headers[header].name << "= '" << info->http_headers[header].value << "'" << endl;
#endif // DEBUG

				if(!strcasecmp(info->http_headers[header].name,"Last-Modified")) {
					ub.modtime = (time_t) HTTP::TimeStamp(info->http_headers[header].value);
				}

			}

			if(ub.modtime == 0) {
				cerr << "civetweb\tNo cache information in the response header" << endl;
			} else if(utime(filename,&ub) == -1) {
				cerr << "civetweb\tError '" << strerror(errno) << "' setting file timestamp" << endl;
			} else {
				cout << "civetweb\t'" << filename << "' time set to " << TimeStamp(ub.modtime) << endl;
			}

		} else {

			cout << "civetweb\tServer response was '" << info->status_code << " " << info->status_text << "'" << endl;

			throw HTTP::Exception(info->status_code, url.c_str(), info->status_text);

		}

	} catch(...) {

		mg_close_connection(conn);
		throw;
	}

	mg_close_connection(conn);

	return status;

 }


