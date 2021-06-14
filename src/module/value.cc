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

 using namespace std;

 Value::Value(Udjat::Value::Type t) : type(t) {
 }

 Value::~Value() {
 	reset(Udjat::Value::Undefined);
 }

 Udjat::Value & Value::reset(const Udjat::Value::Type type) {

	if(type != this->type) {

		// Reset value.
		this->type = type;
		this->value.clear();

		// cleanup children.
		for(auto child : children) {
			delete child.second;
		}

		children.clear();

	}

	return *this;

 }

 bool Value::isNull() const {
	return this->type == Udjat::Value::Undefined;
 }

 Udjat::Value & Value::operator[](const char *name) {

	reset(Udjat::Value::Object);

	auto search = children.find(name);
	if(search != children.end()) {
		return *search->second;
	}

	Value * rc = new Value();

	children[name] = rc;

	return *rc;
 }

 Udjat::Value & Value::append(const Type type) {
	reset(Udjat::Value::Array);

	Value * rc = new Value(type);
	children[std::to_string((int) children.size()).c_str()] = rc;

	return *rc;
 }

 Udjat::Value & Value::set(const char *value, const Type type) {
	reset(type);
	this->value = value;
	return *this;
 }

 Udjat::Value & Value::set(const Udjat::Value &value) {
 	throw runtime_error("Not implemented");
	return *this;
 }

 Udjat::Value & Value::set(const Udjat::TimeStamp value) {
	this->set(value.to_string(TIMESTAMP_FORMAT_JSON).c_str(),Udjat::Value::String);
 }

 std::string Value::to_string() const {
	return this->value;
 }

 void Value::json(std::stringstream &ss) const {

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

