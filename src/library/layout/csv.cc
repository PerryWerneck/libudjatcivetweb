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
 #include <udjat/defs.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/http/layouts.h>
 #include <udjat/tools/logger.h>
 #include <iostream>
 #include <vector>

 using namespace std;

 namespace Udjat {

	/*
	void HTTP::to_csv(std::ostream &ss, const Udjat::Value &value, const char delimiter) {

		if(value == Udjat::Value::Array && !value.empty()) {

			// Get column names;
			vector<string> colnames;
			value.for_each([&ss,&colnames,delimiter](const char *, const Value &row){
				bool sep = false;
				row.for_each([&ss,&colnames,&sep,delimiter](const char *name, const Value &){
					colnames.push_back(name);
					if(sep) {
						ss << delimiter;
					}
					ss << name;
					return false;
				});
				return true;
			});

			ss << endl;

			// Get contents
			value.for_each([&ss,&colnames,delimiter](const char *, const Udjat::Value &row){
				bool sep = false;
				for(const string &name : colnames) {
					if(sep) {
						ss << delimiter;
					}
					string field{row[name.c_str()].to_string()};
					for(char *ptr = (char *) field.c_str();*ptr;ptr++) {
						if(*ptr == delimiter) {
							*ptr = ' ';
						}
					}
					ss << field;
				}
				ss << endl;
				return false;
			});

		}

	}
	*/

 }

