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

 /*
 class Response : Udjat::Response {
 public:
 	Response() = default;

 };
 */

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
