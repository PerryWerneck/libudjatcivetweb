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
 #include <udjat/tools/logger.h>
 #include <sstream>
 #include <udjat/tools/http/layouts.h>

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		Report::Report(Udjat::MimeType mimetype) : Udjat::Response::Table{mimetype} {
		}

		Report::~Report() {
		}

		Udjat::Response::Table & Report::push_back(const char *str, Udjat::Value::Type type) {
			debug("column(",columns.current->c_str(),")='",str,"'");
			values.emplace_back(str,type);
			next();
			return *this;
		}

		std::string Report::to_string() const {
			std::stringstream stream;
			save(stream);
			return stream.str();
		}

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

	}

 }

 /*
 namespace Udjat {

	namespace HTTP {

		Report::Report() : mimetype(MimeType::json) {
		}

		Report::Report(const char *uri, const MimeType m) : Udjat::Report(), mimetype(m) {

			if(mimetype != MimeType::json && mimetype != MimeType::html && mimetype != MimeType::xml) {
				throw HTTP::Exception(501, uri, "Mimetype Not Supported");
			}

		}

		Report::~Report() {
		}

		std::string Report::to_string() const {
			std::stringstream ss;

			if(mimetype == MimeType::html) {
				this->to_html(ss);
			} else if(mimetype == MimeType::json) {
				this->to_json(ss);
			} else if(mimetype == MimeType::xml) {
				this->to_xml(ss);
			}
			return ss.str();
		}

		void Report::to_json(std::stringstream &ss) const {

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
				value.json(ss);

				column++;

			}

			ss << "}]";

		}

		void Report::to_html(std::stringstream &ss) const {

			ss << "<table><thead>";

			// ss << "<caption>" << this->title << "</caption>"

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

		void Report::to_xml(std::stringstream &ss) const {

			ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

			if(values.empty()) {
				ss << "</report>";
			} else {

				ss << "<report><item>";

				auto column = columns.names.begin();
				for(auto value : values) {
					if(column == columns.names.end()) {
						ss << "</item><item>";
						column = columns.names.begin();
					}
					ss << "<" << *column << ">" << value.to_string() << "</" << *column << ">";
					column++;
				}

				ss << "</item></report>";
			}

		}

		Udjat::Report & Report::push_back(const char *value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const std::string &value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const short value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const unsigned short value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const int value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const unsigned int value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const long value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const unsigned long value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const Udjat::TimeStamp value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const bool value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const float value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

		Udjat::Report & Report::push_back(const double value) {
			HTTP::Value v;
			v << value;
			values.push_back(v);
			return *this;
		}

	}

 }
 */
