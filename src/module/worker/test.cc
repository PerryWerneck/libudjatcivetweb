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
 #include <private/module.h>
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

		int Worker::test(const std::function<bool(double current, double total)> &progress) noexcept {

			progress(0,0);

			struct mg_connection *conn;
			try {

				conn = connect();

			} catch(const std::exception &e) {

				error() << url() << ":" << e.what() << endl;
				return ENOTCONN;

			} catch(...) {

				error() << url() << ": Unexpected error" << endl;
				return ENOTCONN;

			}

			int response = -1;

			const struct mg_response_info *info = mg_get_response_info(conn);

			response = info->status_code;

			if(info->content_length > 0) {

				progress((double) 0, (double) info->content_length);
				char * buffer = new char [info->content_length + 1];
				memset(buffer,0,info->content_length + 1);

				long long loaded = 0;

				char buf[1024];
				while(loaded < info->content_length && response == info->status_code) {

					int szRead = mg_read(conn, buf, 1024);

					if(szRead == 0) {

						response = ENOTCONN;

					} else if(szRead < 0) {

						response = errno;
						break;

					} else {

						loaded += (size_t) szRead;
						if(!progress((double) loaded, (double) info->content_length)) {
							response = ECANCELED;
						}

					}

				}

				progress((double) info->content_length, (double) info->content_length);

			}

			mg_close_connection(conn);

			return response;
		}

	 }

 }
