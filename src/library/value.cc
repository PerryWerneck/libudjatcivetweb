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
 #include <udjat/civetweb.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/http/value.h>
 #include <udjat/tools/logger.h>
 #include <iostream>
 #include <iomanip>

 using namespace std;

 namespace Udjat {

	HTTP::Value::Value(Udjat::Value::Type t) : type(t) {
	}

	HTTP::Value::~Value() {
		reset(Udjat::Value::Undefined);
	}

	bool HTTP::Value::empty() const noexcept {
		return children.empty();
	}

	HTTP::Value::operator Value::Type() const noexcept {
		return this->type;
	}

	bool HTTP::Value::for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const {

		for(const auto [key, value] : children)	{
			if(call(key.c_str(),*value)) {
				return true;
			}
		}

		return false;
	}

	void HTTP::Value::to_json(std::ostream &ss) const {
	}

	void HTTP::Value::to_xml(std::ostream &ss) const {
	}

	void HTTP::Value::to_html(std::ostream &ss) const {
	}

	void HTTP::Value::to_yaml(std::ostream &ss, size_t left_margin) const {
	}

	/*
	void HTTP::Value::dump(std::stringstream &ss, const MimeType mimetype) const {

		switch(mimetype) {
		case Udjat::MimeType::xml:
			// Format as XML
			ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
			ss << "<response>";
			this->xml(ss);
			ss << "</response>";
			break;

		case Udjat::MimeType::json:
			// Format as JSON
			this->json(ss);
			break;

		case Udjat::MimeType::html:
			// Format as HTML.
			this->html(ss);
			break;

		case Udjat::MimeType::yaml:
			// Format as yaml.
			this->yaml(ss);
			break;

		case Udjat::MimeType::sh:

			// Format as shell script (only first level)

			for_each([&ss](const char *key, const Udjat::Value &value){

				switch((Value::Type) value) {
				case Udjat::Value::Undefined:
				case Udjat::Value::Array:
					break;

				case Udjat::Value::Object:
					// TODO: Check for 'summary' ou 'value' objects.
					break;

				case Udjat::Value::Signed:
				case Udjat::Value::Unsigned:
				case Udjat::Value::Real:
				case Udjat::Value::Boolean:
				case Udjat::Value::Fraction:
					ss << key << "=" << value << endl;
					break;

				default:
					ss << key << "=\"" << value << "\"" << endl;
				}

				return false;
			});

			break;

		default:
			throw system_error(ENOTSUP,system_category(),Logger::String{"No exporter for ",mimetype});

		}

	}
	*/

	Value & HTTP::Value::reset(const Udjat::Value::Type type) {

		if(type != this->type) {

			// Reset value.
			this->type = type;
			this->value.clear();

			// cleanup children.
			for(auto child : children) {
				delete child.second;
			}

			children.clear();

		}

		return *this;

	}

	bool HTTP::Value::isNull() const {
		return this->type == Udjat::Value::Undefined;
	}

	Value & HTTP::Value::operator[](const char *name) {

		reset(Udjat::Value::Object);

		auto search = children.find(name);
		if(search != children.end()) {
			return *search->second;
		}

		Value * rc = new Value();

		children[name] = rc;

		return *rc;
	}

	Value & HTTP::Value::append(const Type type) {
		reset(Udjat::Value::Array);

		Value * rc = new Value(type);
		children[std::to_string((int) children.size()).c_str()] = rc;

		return *rc;
	}

	Value & HTTP::Value::set(const char *value, const Type type) {
		reset(type);
		this->value = value;
		return *this;
	}

	Value & HTTP::Value::set(const Udjat::Value UDJAT_UNUSED(&value)) {
		throw runtime_error("Not implemented");
		return *this;
	}

	Value & HTTP::Value::set(const Udjat::TimeStamp value) {
		if(value)
			return this->set(value.to_string(TIMESTAMP_FORMAT_JSON).c_str(),Udjat::Value::String);
		return this->set("false",Value::Type::Boolean);
	}

	Udjat::Value & HTTP::Value::setFraction(const float fraction) {
		std::stringstream out;
		out.imbue(std::locale("C"));
		out << std::fixed << std::setprecision(2) << (fraction *100);
		return Udjat::Value::set(out.str(),Value::Fraction);
	}

	Value & HTTP::Value::set(const float value) {
		std::stringstream out;
		out.imbue(std::locale("C"));
		out << value;
		return Udjat::Value::set(out.str(),Value::Real);
	}

	Value & HTTP::Value::set(const double value) {
		std::stringstream out;
		out.imbue(std::locale("C"));
		out << value;
		return Udjat::Value::set(out.str(),Value::Real);
	}

	const Udjat::Value & HTTP::Value::get(std::string &value) const {

		if(!children.empty()) {

			// Has children, check standard names.
			static const char *names[] = {
				"summary",
				"value",
				"name"
			};

			for(size_t ix = 0; ix < (sizeof(names)/sizeof(names[0]));ix++) {

				auto child = children.find(names[ix]);
				if(child != children.end()) {
					child->second->get(value);
					if(!value.empty()) {
						return *this;
					}
				}

			}

		}

		value = this->value;
		return *this;
	}


 }
