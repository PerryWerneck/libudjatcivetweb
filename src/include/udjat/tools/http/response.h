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
 #include <udjat/tools/value.h>
 #include <udjat/tools/http/value.h>
 #include <ostream>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Response : public Udjat::Response::Value {
		private:

			Value::Type type = Value::Object;
			std::map<std::string,HTTP::Value> children;

			/// @brief Value for X-Total-Count header.
			size_t total_count = 0;

			/// @brief Values for Content-Range header.
			struct {
				size_t from = 0;
				size_t to = 0;
				size_t total = 0;
			} range;

		public:
			Response(Udjat::MimeType mimetype);
			virtual ~Response();

			int status_code() const noexcept;

			operator Type() const noexcept override;

			bool empty() const noexcept override;

			/// @brief Get text for response, parse errors, add custom page if necessary.
			/// @param response Response to.
			/// @param mimetype The mimetype for response.
			/// @return true if the string
			static std::string to_string(const Udjat::Abstract::Response &response, const MimeType mimetype);

			std::string to_string() const override;

			/// @brief Enumerate headers.
			void for_each(const std::function<void(const char *header_name, const char *header_value)> &call) const noexcept;

			bool for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const override;
			Udjat::Value & operator[](const char *name) override;

			Udjat::Value & append(const Type type = Object) override;
			Udjat::Value & reset(const Udjat::Value::Type type) override;
			Udjat::Value & set(const char *value, const Type type = String) override;

			/// @brief Set item count for this response.
			/// @param value The item count (for X-Total-Count http header).
			void count(size_t value) noexcept override;

			/// @brief Set range for this response (Content-Range http header).
			/// @param from First item.
			/// @param to Last item.
			/// @param total Item count.
			void content_range(size_t from, size_t to, size_t total) noexcept override;

		};

	}

 }

