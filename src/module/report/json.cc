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

 namespace Reports {

	JSON::JSON() : Udjat::Response::Report() {
		report = Json::Value(Json::arrayValue);
	}

	JSON::JSON(const char *column_name, ...) : JSON() {

		va_list args;
		va_start(args, column_name);
		setColumns(column_name,args);
		va_end(args);

	}

	JSON::~JSON() {
	}

	bool JSON::open() {
		if(Udjat::Response::Report::open()) {
			row = Json::Value(Json::objectValue);
			return true;
		}
		return false;
	}

	bool JSON::close() {
		if(Udjat::Response::Report::close()) {
			report.append(row);
			return true;
		}
		return false;
	}

	std::string JSON::to_string() {
		close();
		return report.toStyledString();
	}

	Udjat::Response::Report & JSON::push_back(const char *str) {
		row[next()] = str;
		return *this;
	}

	Udjat::Response::Report & JSON::push_back(const bool value) {
		row[next().c_str()] = value;
		return *this;
	}

	Udjat::Response::Report & JSON::push_back(const int8_t value) {
		row[next().c_str()] = (int) value;
		return *this;
	}

	Udjat::Response::Report & JSON::push_back(const int16_t value) {
		row[next().c_str()] = (int) value;
		return *this;
	}

	Udjat::Response::Report & JSON::push_back(const int32_t value) {
		row[next().c_str()] = (int) value;
		return *this;
	}

	Udjat::Response::Report & JSON::push_back(const uint8_t value) {
		row[next().c_str()] = (unsigned int) value;
		return *this;
	}

	Udjat::Response::Report & JSON::push_back(const uint16_t value) {
		row[next().c_str()] = (unsigned int) value;
		return *this;
	}

	Udjat::Response::Report & JSON::push_back(const uint32_t value) {
		row[next().c_str()] = (unsigned int) value;
		return *this;
	}

 }
