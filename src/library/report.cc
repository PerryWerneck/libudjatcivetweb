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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/report.h>
 #include <udjat/tools/http/report.h>
 #include <udjat/tools/http/value.h>
 #include <udjat/tools/logger.h>
 #include <sstream>
 #include <udjat/tools/http/layouts.h>
 #include <udjat/tools/http/timestamp.h>

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		Report::Report(Udjat::MimeType mimetype) : Udjat::Response::Table{mimetype} {
		}

		Report::~Report() {
		}

		bool Report::empty() const {
			return values.empty();
		}

		void Report::for_each(const std::function<void(const char *header_name, const char *header_value)> &call) const noexcept {

			Abstract::Response::for_each(call);

			// https://stackoverflow.com/questions/3715981/what-s-the-best-restful-method-to-return-total-number-of-items-in-an-object
			if(total_count) {
				call("X-Total-Count",std::to_string(total_count).c_str());
			}

			if(range.total) {
				call("Content-Range",Udjat::String{"items ",range.from,"-",range.to,"/",range.total}.c_str());
			}

		}

		void Report::count(size_t value) noexcept {
			total_count = value;
		}

		void Report::content_range(size_t from, size_t to, size_t total) noexcept {
			range.from = from;
			range.to = to;
			range.total = total;
		}

		void Report::for_each(const std::function<void(const Value::Type type, const char *value)> &func) const {
			for(const Value &value : values) {
				func((Value::Type) value,value.to_string().c_str());
			}
		}

		Udjat::Response::Table & Report::push_back(const char *str, Udjat::Value::Type type) {
			values.emplace_back(str,type);
			next();
			return *this;
		}

		/*
		std::string Report::to_string() const {
			std::stringstream stream;
			save(stream);
			return stream.str();
		}
		*/

		/*
		void Report::save(std::ostream &ss) const {

			if(values.empty()) {
				return;
			}

			switch((MimeType) *this) {
			case Udjat::MimeType::xml:
				ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
				ss << "<response>";

				if(!info.caption.empty()) {
					ss << "<caption>" << info.caption << "</caption>";
				}

				if(values.empty()) {

					ss << "<contents />";

				} else {

					ss << "<contents><item>";

					auto column = columns.names.begin();
					for(auto value : values) {
						if(column == columns.names.end()) {
							ss << "</item><item>";
							column = columns.names.begin();
						}
						ss << "<" << *column << ">" << value.to_string() << "</" << *column << ">";
						column++;
					}

					ss << "</item></contents>";
				}

				ss << "</response>";
				break;

			case Udjat::MimeType::json:
				{
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
						to_json(ss,value);
						column++;

					}

					ss << "}]";

				}
				break;

			case Udjat::MimeType::html:
				{
					ss << "<table><thead>";

					if(!info.caption.empty()) {
						ss << "<caption>" << info.caption << "</caption>";
					}

					ss << "<tr>";

					for(auto column : columns.names) {
						ss << "<th>" << column << "</th>";
					}

					ss << "</tr></thead><tbody><tr>";

					auto column = columns.names.begin();
					for(auto value : values) {
						if(column == columns.names.end()) {
							ss << "</tr><tr>";
							column = columns.names.begin();
						}
						ss << "<td>" << value.to_string() << "</td>";
						column++;
					}

					ss << "</tr></tbody></table>";
				}
				break;

			case Udjat::MimeType::csv:
				{
					bool sep{false};
					for(auto column : columns.names) {
						if(sep) {
							ss << ",";
						}
						sep = true;
						ss << column;
					}
					ss << endl;

					sep = false;

					auto column = columns.names.begin();
					for(auto value : values) {
						if(column == columns.names.end()) {
							ss << endl;
							sep = false;
						}
						if(sep) {
							ss << ",";
						}
						sep = true;

						if(value == Udjat::Value::Signed || value == Udjat::Value::Unsigned || value == Udjat::Value::Real || value == Udjat::Value::Boolean || value == Udjat::Value::Fraction) {
							ss << value.to_string();
						} else {
							ss << "\"" << value.to_string() << "\"";
						}

						column++;
					}

					ss << endl;
				}
				break;

			default:
				throw system_error(ENOTSUP,system_category(),Logger::String{"No exporter for ",((MimeType) *this)});
			}
		}
		*/

	}

 }

