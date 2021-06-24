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
 #include <list>

 namespace Udjat {

 	namespace CivetWeb {

		class UDJAT_API Value : public Udjat::Value {
		private:
			Udjat::Value::Type type;
			std::string value;
			std::map<std::string,Value *> children;

		public:

			Value(Udjat::Value::Type t = Udjat::Value::Undefined);
			virtual ~Value();

			bool isNull() const override;

			inline Udjat::Value::Type getType() const noexcept {
				return this->type;
			}

			void json(std::stringstream &ss) const;
			void xml(std::stringstream &ss) const;

			std::string to_string() const;

			Udjat::Value & operator[](const char *name) override;

			Udjat::Value & append(const Type type) override;

			Udjat::Value & reset(const Type type) override;

			Udjat::Value & set(const Udjat::Value &value) override;

			Udjat::Value & set(const char *value, const Type type) override;

			Udjat::Value & set(const Udjat::TimeStamp value) override;

			Udjat::Value & setFraction(const float fraction);

			Udjat::Value & set(const float value);

			Udjat::Value & set(const double value);

		};

		class UDJAT_API Request : public Udjat::Request {
		private:
			std::string path;

		public:
			Request(const std::string &uri, const char *method);

			std::string pop() override;

		};

		class UDJAT_API Response : public Udjat::Response {
		private:
			CivetWeb::Value *value;

		public:
			Response(Udjat::MimeType type);
			virtual ~Response();

			bool isNull() const override;

			std::string to_string() const;

			Udjat::Value & operator[](const char *name) override;

			Udjat::Value & append(const Type type) override;

			Udjat::Value & reset(const Type type) override;

			Udjat::Value & set(const Value &value) override;

			Udjat::Value & set(const char *value, const Type type) override;

		};

		class UDJAT_API Report : public Udjat::Report {
		private:

			/// @brief Report contents
			std::list<Value> values;

			void json(std::stringstream &ss) const;

		 public:
			Report();
			virtual ~Report();

			std::string to_string() const;

			Udjat::Report & push_back(const char *str) override;

			Udjat::Report & push_back(const std::string &value) override;

			Udjat::Report & push_back(const short value) override;
			Udjat::Report & push_back(const unsigned short value) override;

			Udjat::Report & push_back(const int value) override;
			Udjat::Report & push_back(const unsigned int value) override;

			Udjat::Report & push_back(const long value) override;
			Udjat::Report & push_back(const unsigned long value) override;

			Udjat::Report & push_back(const Udjat::TimeStamp value) override;
			Udjat::Report & push_back(const bool value) override;

			Udjat::Report & push_back(const float value) override;
			Udjat::Report & push_back(const double value) override;

		};


 	}

 }

