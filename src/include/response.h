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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/request.h>
 #include <udjat/tools/value.h>
 #include <sstream>
 #include <map>

 class UDJAT_API Value : public Udjat::Value {
 private:
	Udjat::Value::Type type;
	std::string value;
	std::map<std::string,Value *> children;

 public:

 	Value(const Value &src) = delete;
 	Value(const Value *src) = delete;

 	Value(Udjat::Value::Type t = Udjat::Value::Undefined);
 	virtual ~Value();

	bool isNull() const override;

	void json(std::stringstream &ss) const;

	std::string to_string() const override;

	Udjat::Value & operator[](const char *name) override;

	Udjat::Value & append(const Type type) override;

	Udjat::Value & reset(const Type type) override;

	Udjat::Value & set(const Udjat::Value &value) override;

	Udjat::Value & set(const char *value, const Type type) override;

 };

 class UDJAT_API Response : public Udjat::Response {
 private:
	::Value *value;

 public:
 	Response();
 	virtual ~Response();

	bool isNull() const override;

	std::string to_string() const override;

	Udjat::Value & operator[](const char *name) override;

	Udjat::Value & append(const Type type) override;

	Udjat::Value & reset(const Type type) override;

	Udjat::Value & set(const Value &value) override;

	Udjat::Value & set(const char *value, const Type type) override;

 };

	/*
 namespace Reports {

	class UDJAT_API JSON : public Udjat::Report {
	private:

		/// @brief Report contents (Json Array)
		Json::Value report;

		/// @brief Current row (Json Object)
		Json::Value row;

	public:
		JSON();
		virtual ~JSON();

		bool open() override;
		bool close() override;

		std::string to_string() override;

		Udjat::Report & push_back(const char *str) override;
		Udjat::Report & push_back(const bool value) override;
		Udjat::Report & push_back(const int8_t value) override;
		Udjat::Report & push_back(const int16_t value) override;
		Udjat::Report & push_back(const int32_t value) override;
		Udjat::Report & push_back(const uint8_t value) override;
		Udjat::Report & push_back(const uint16_t value) override;
		Udjat::Report & push_back(const uint32_t value) override;

	};
 }
	*/
