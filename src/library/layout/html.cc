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
 #include <udjat/civetweb.h>
 #include <udjat/tools/http/value.h>
 #include <iostream>
 #include <iomanip>
 #include <vector>

 using namespace std;

 namespace Udjat {

	void HTTP::Value::html(std::stringstream &ss) const {

		switch(this->type) {
		case Udjat::Value::Object:
			{
				ss << "<ul>";
				for(auto &child : children) {
					ss << "<li><label>" << child.first << ":&nbsp;";
					if(child.second->type == Udjat::Value::Object || child.second->type == Udjat::Value::Array ) {
						child.second->html(ss);
					} else {
						ss << "<strong>" << child.second->value << "</strong>";
					}

					ss << "</label></li>";
				}
				ss << "</ul>";
			}
			break;

		case Udjat::Value::Array:
			{
				ss << "<table>";

				// Get column names.
				vector<string> colnames;
				{
					const Value &first = *children.begin()->second;
					if(!first.children.empty()) {
						ss << "<thead><tr>";
						for(auto &col : first.children) {
							ss << "<th>";
							colnames.push_back(col.first);
							ss << col.first;
							ss << "</th>";
						}
						ss << "</tr></thead>";
					}
				}

				// Get column contents.
				{
					for(auto &row : children) {
						if(!row.second->children.empty()) {
							ss << "<tr>";
							for(string &colname : colnames) {
								auto value = row.second->children.find(colname.c_str());
								ss << "<td>";
								if(value != row.second->children.end()) {
									ss << value->second->value;
								}
								ss << "</td>";
							}
							ss << "</tr>";
						}
					}
				}

				ss << "<tbody>";


				ss << "</tbody></table>";
			}
			break;

		}

	}

 }


