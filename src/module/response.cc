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
 #include <sys/mman.h>

 ::Response::Response(struct mg_connection *conn) {

	const struct mg_response_info *info = mg_get_response_info(conn);

	status.code = info->status_code;
	status.text = info->status_text;

#ifdef DEBUG
	cout 	<< "Status: " << status.code << " " << status.text << endl
			<< "Length: " << info->content_length << endl;
#endif // DEBUG

	if(info->content_length < 0) {
		response.length = 0;
		return;
	}

	response.length = (size_t) info->content_length;

#ifdef DEBUG
	cout << "Response.length=" << response.length << endl;
#endif // DEBUG

	if(!response.length)
		return;

	// Map as memory
	response.payload = (char *) mmap(NULL, response.length, PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	if(response.payload == MAP_FAILED) {
		if(errno == ENODEV) {
			throw runtime_error("The underlying filesystem does not support memory mapping.");
		}
		throw system_error(errno, system_category(), "Cant map response block");
	}

	int szRead = mg_read(conn, response.payload, response.length);
	if(szRead != (int) response.length) {
		munmap(response.payload,response.length);
		throw runtime_error(string{"Got '"} + to_string(szRead) + "' when expecting '" + to_string(response.length) + "'");
	}

 }

 bool ::Response::isValid() const noexcept {
	return status.code == 200 && URL::Response::isValid();
 }

 ::Response::~Response() {
	munmap(response.payload,response.length);
 }
