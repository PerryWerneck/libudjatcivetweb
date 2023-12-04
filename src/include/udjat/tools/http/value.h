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
 #include <udjat/tools/value.h>
 #include <udjat/tools/http/mimetype.h>
 #include <map>
 #include <ostream>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_PRIVATE Value : public Udjat::Value {
		private:
			Udjat::Value::Type type;
			std::string value;
			std::map<std::string,Value *> children;

		public:

			Value(Udjat::Value::Type t = Udjat::Value::Undefined);
			Value(const char *value, Udjat::Value::Type t = Udjat::Value::Undefined);

			virtual ~Value();

			bool empty() const noexcept override;
			bool isNull() const override;

			operator Type() const noexcept override;

			bool for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const override;

			void to_json(std::ostream &ss) const;
			void to_xml(std::ostream &ss) const;
			void to_html(std::ostream &ss) const;
			void to_yaml(std::ostream &ss, size_t left_margin = 0) const;

			const Udjat::Value & get(std::string &value) const override;

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

	}

 }
