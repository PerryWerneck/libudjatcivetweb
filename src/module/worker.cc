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
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/file.h>
 #include <sys/types.h>
 #include <utime.h>
 #include <udjat/tools/http/timestamp.h>

 namespace Udjat {

	 namespace CivetWeb {

		Worker::Worker(const char *url, const HTTP::Method method, const char *payload) : Udjat::Protocol::Worker(url,method,payload) {

			header("Connection") = "close";
			header("User-Agent") = STRINGIZE_VALUE_OF(PRODUCT_NAME);

			Config::for_each(
				"http-default-headers",
				[this](const char *key, const char *value) {
					header(key) = value;
					return true;
				}
			);

		}

		Udjat::Protocol::Header & Worker::header(const char *name) {

			for(Header &header : headers) {
				if(header == name) {
					return header;
				}
			}

			headers.emplace_back(name);
			return headers.back();

		}

		struct mg_connection * Worker::connect() {

			URL::Components components = args.url.ComponentsFactory();
			header("Host") = components.hostname;

			string hdr;
			for(Header &header : headers) {
				hdr += header.name + ":" + header.c_str() + "\r\n";
			}

			char error_buffer[256] = "";
			struct mg_connection *conn =
				mg_download(
					components.hostname.c_str(),
					components.portnumber(),
					strcasecmp(components.scheme.c_str(),"https") == 0,
					error_buffer,
					sizeof(error_buffer),
					"%s %s HTTP/1.0\r\n%s\r\n%s",
					std::to_string(args.method),
					(components.path.empty() ? "/" : components.path.c_str()),
					hdr.c_str(),
					args.payload.c_str()
				);

			if(!conn) {
				throw runtime_error(error_buffer);
			}

			return conn;

		}

		Udjat::String Worker::get(const std::function<bool(double current, double total)> &progress) {

			progress(0,0);

			struct mg_connection *conn = connect();

			Udjat::String response;
			try {

				const struct mg_response_info *info = mg_get_response_info(conn);

#ifdef DEBUG
				cout << "civetweb\tServer response was '" << info->status_code << " " << info->status_code << "'" << endl;
#endif // DEBUG

				if(info->status_code < 200 || info->status_code > 299) {

					throw HTTP::Exception(info->status_code, args.url.c_str(), info->status_text);

				} else if((unsigned int) info->content_length >= response.max_size()) {

					throw system_error(E2BIG,system_category(),"The response is too big for current implementation");

				} else if(info->content_length > 0) {

					progress((double) 0, (double) info->content_length);
					char * buffer = new char [info->content_length + 1];
					memset(buffer,0,info->content_length + 1);

					long long loaded = 0;

					try {
						while(loaded < info->content_length) {

							int szRead = mg_read(conn, (void *) (buffer+loaded), 1024);

							if(szRead == 0) {
								throw system_error(ENOTCONN,system_category(),"Connection closed while downloading file");
							} else if(szRead < 0) {
								throw runtime_error("Download error");
							} else {
								loaded += (size_t) szRead;
								if(!progress((double) loaded, (double) info->content_length)) {
									throw system_error(ECANCELED,system_category());
								}
							}

						}

					} catch(...) {
						delete[] buffer;
						throw;
					}

					buffer[info->content_length] = 0;
					response = buffer;
					delete[] buffer;

					progress((double) info->content_length, (double) info->content_length);

				}

			} catch(...) {

				mg_close_connection(conn);
				throw;
			}

			mg_close_connection(conn);

			return response;
		}

		bool Worker::save(const char *filename, const std::function<bool(double current, double total)> &progress) {

			progress(0,0);

			struct mg_connection *conn = connect();

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

					File::Temporary response{filename};
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

					throw HTTP::Exception(info->status_code, args.url.c_str(), info->status_text);

				}

			} catch(...) {

				mg_close_connection(conn);
				throw;
			}

			mg_close_connection(conn);

			return status;

		}

	 }

 }
