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
 #include <response.h>
 #include <iostream>
 #include <iomanip>

 void Value::xml(std::stringstream &ss) const {

 	switch(this->type) {
	case Udjat::Value::Undefined:
		break;

	case Udjat::Value::Array:
		for(auto &child : children) {
			ss << "<item>";
			child.second->xml(ss);
			ss << "</item>";
		}
		break;

	case Udjat::Value::Object:
		for(auto &child : children) {
			ss << "<" << child.first << " type='" << std::to_string(child.second->getType()) << "'"<< ">";
			child.second->xml(ss);
			ss << "</" << child.first << ">";
		}
		break;

	default:
		ss << this->value;
 	}

 }
