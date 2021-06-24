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
 #include "private.h"
 #include <udjat/request.h>

 namespace Udjat {

	CivetWeb::Request::Request(const string &u, const char *m)
		: Udjat::Request(m), path(u) {

		this->name = pop();

	}

	std::string CivetWeb::Request::pop() {

		if(path.empty()) {
			throw system_error(ENODATA,system_category(),"No more arguments");
		}

		size_t pos = path.find('/');
		if(pos == string::npos) {
			string rc = path;
			path.clear();
			return rc;
		}

		string rc{path.c_str(),pos};
		path.erase(0,pos+1);

		return rc;
	}

 }
