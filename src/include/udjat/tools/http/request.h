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

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Request : public Udjat::Request {
		public:
			Request(const std::string &url, const char *type);

			std::string pop() override;

		};

		class UDJAT_API Response : public Udjat::Response {
		private:
			Udjat::HTTP::Value *value;

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

	}

 }

