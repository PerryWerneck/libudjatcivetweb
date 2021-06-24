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
 #include <iostream>
 #include <iomanip>

 namespace Udjat {

	void CivetWeb::Value::json(std::stringstream &ss) const {

		switch(this->type) {
		case Udjat::Value::Undefined:
			ss << "null";
			break;

		case Udjat::Value::Array:
			{
				ss << '[';

				bool sep = false;
				for(auto &child : children) {
					if(sep) {
						ss << ',';
					}
					sep = true;
					child.second->json(ss);
				}

				ss << ']';
			}
			break;

		case Udjat::Value::Object:
			{
				ss << '{';

				bool sep = false;
				for(auto &child : children) {
					if(sep) {
						ss << ',';
					}
					sep = true;
					ss << '"' << child.first << "\":";
					child.second->json(ss);
				}

				ss << '}';
			}
			break;

		case Udjat::Value::String:

			// TODO: Convert special chars.
			ss << '"' << this->value << '"';
			break;

		default:
			ss << this->value;
		}

	}

 }


