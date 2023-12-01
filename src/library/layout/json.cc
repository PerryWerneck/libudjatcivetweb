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
 #include <iostream>

 namespace Udjat {

	void HTTP::to_json(std::ostream &output, const Udjat::Value &value) {

		switch((Value::Type) value) {
		case Udjat::Value::Undefined:
			output << "null";
			break;

		case Udjat::Value::Array:
			{
				output << '[';
				bool sep = false;
				value.for_each([&output,&sep](const char *, const Value &value){
					if(sep) {
						output << ',';
					}
					sep = true;
					to_json(output,value);
					return false;
				});
				output << ']';
			}
			break;

		case Udjat::Value::Object:
			{
				output << '{';
				bool sep = false;
				value.for_each([&output,&sep](const char *name, const Value &value){
					if(sep) {
						output << ',';
					}
					sep = true;
					output << '"' << name << "\":";
					to_json(output,value);
					return false;
				});
				output << '}';
			}
			break;

		case Udjat::Value::Signed:
		case Udjat::Value::Unsigned:
		case Udjat::Value::Real:
		case Udjat::Value::Boolean:
		case Udjat::Value::Fraction:
			output << value;
			break;

		default:
			// TODO: Convert special chars.
			output << '"' << value << '"';

		}

	}

 }

 /*
 #include <udjat/civetweb.h>
 #include <udjat/tools/http/value.h>
 #include <iostream>
 #include <iomanip>

 namespace Udjat {

	void HTTP::Value::json(std::stringstream &ss) const {

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

		case Udjat::Value::Signed:
		case Udjat::Value::Unsigned:
		case Udjat::Value::Real:
		case Udjat::Value::Boolean:
		case Udjat::Value::Fraction:
			ss << this->value;
			break;

		default:
			// TODO: Convert special chars.
			ss << '"' << this->value << '"';
		}

	}

 }
 */


