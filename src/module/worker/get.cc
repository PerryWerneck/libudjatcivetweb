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
 #include "../private.h"
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/file.h>
 #include <sys/types.h>
 #include <utime.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/timestamp.h>

 namespace Udjat {

	 namespace CivetWeb {

		Udjat::String Worker::get(const std::function<bool(double current, double total)> &progress) {

			progress(0,0);

			struct mg_connection *conn = connect();

			Udjat::String response;
			try {

				const struct mg_response_info *info = mg_get_response_info(conn);

				debug("Server response was '", info->status_code, " ", info->status_text, "'");

				if(info->status_code < 200 || info->status_code > 299) {

					throw HTTP::Exception(info->status_code, url().c_str(), info->status_text);

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
					response.assign(buffer);
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

	 }

 }
