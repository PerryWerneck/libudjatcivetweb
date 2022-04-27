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
 #include <udjat/tools/http/value.h>
 #include <list>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_PRIVATE Report : public Udjat::Report {
		private:

			/// @brief Report contents
			std::list<HTTP::Value> values;

			void json(std::stringstream &ss) const;

		 public:
			Report(const std::string &uri, const MimeType mimetype);
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
