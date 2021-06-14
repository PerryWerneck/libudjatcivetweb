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

 #include <response.h>
 #include <cstdarg>
 #include <iostream>

 using namespace std;

 Report::Report() : Udjat::Report() {
 }

 Report::~Report() {
 }

 std::string Report::to_string() const {
  	std::stringstream ss;
 	this->json(ss);
 	return ss.str();
 }

 void Report::json(std::stringstream &ss) const {

	bool sep = false;
	auto column = columns.names.begin();

	ss << "[{";
	for(auto value : values) {

		if(column == columns.names.end()) {
			ss << "},{";
			column = columns.names.begin();
			sep = false;
		}

		if(sep) {
			ss << ',';
		}
		sep = true;

		ss << "\"" << column->c_str() << "\":";
		value.json(ss);

		column++;

	}

	ss << "}]";

 }

 Udjat::Report & Report::push_back(const char *value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const std::string &value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const short value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const unsigned short value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const int value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const unsigned int value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const long value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const unsigned long value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const Udjat::TimeStamp value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const bool value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const float value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }

 Udjat::Report & Report::push_back(const double value) {
	Value v;
	v << value;
	values.push_back(v);
	return *this;
 }


