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
 #include <udjat/tools/response.h>
 #include <udjat/tools/http/value.h>
 #include <ostream>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Response : public Udjat::Response::Value {
		private:
			Value::Type type = Value::Object;
			std::map<std::string,HTTP::Value> children;

		public:
			Response(Udjat::MimeType mimetype);
			virtual ~Response();

			operator Type() const noexcept override;

			bool empty() const noexcept override;

			bool for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const override;
			Udjat::Value & operator[](const char *name) override;

			Udjat::Value & append(const Type type = Object) override;
			Udjat::Value & reset(const Udjat::Value::Type type) override;

			void save(std::ostream &stream) const;
			std::string to_string() const;

		};

	}

 }

 namespace std {

	inline string to_string(const Udjat::HTTP::Response &response) noexcept {
		return response.to_string();
	}

	inline ostream & operator<< (ostream& os, const Udjat::HTTP::Response &response) {
		return os << response.to_string();
	}

 }
