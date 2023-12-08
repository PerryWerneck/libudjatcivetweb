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
 #include <udjat/tools/response.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/http/value.h>
 #include <udjat/tools/http/value.h>
 #include <udjat/tools/http/layouts.h>
 #include <udjat/tools/logger.h>
 #include <iostream>
 #include <sstream>

 using namespace std;

 namespace Udjat {

	HTTP::Response::Response(Udjat::MimeType mimetype) : Udjat::Response::Value{mimetype}{

		debug("Building HTTP response for ",std::to_string(mimetype));

	}

	HTTP::Response::~Response() {
	}

	bool HTTP::Response::empty() const noexcept {
		return children.empty();
	}

	HTTP::Response::operator Type() const noexcept {
		return this->type;
	}

	Udjat::Value & HTTP::Response::append(const Type type) {

		reset(Value::Array);
		return children[std::to_string((int) children.size()).c_str()];

	}

	Udjat::Value & HTTP::Response::reset(const Udjat::Value::Type type) {

		if(type != Value::Array && type != Value::Object) {
			throw runtime_error(Logger::String{"Cant handle '",std::to_string(type),"' at this level"});
		}

		if(type != this->type) {
			debug("Response reset to '",std::to_string(type),"'");

			this->type = type;
			children.clear();
		}

		return *this;
	}

	bool HTTP::Response::for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const {

		for(const auto& [name, value] : children)	{
			if(call(name.c_str(),(Udjat::Value &) value)) {
				return true;
			}
		}

		return false;
	}

	Udjat::Value & HTTP::Response::operator[](const char *name) {
		return children[name];
	}

	std::string HTTP::Response::to_string() const {
		std::stringstream stream;
		save(stream);
		return stream.str();
	}

	void HTTP::Response::save(std::ostream &stream) const {

		switch((MimeType) *this) {
		case Udjat::MimeType::xml:
			// Format as XML
			stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
			stream << "<response>";
			to_xml(stream,*this);
			stream << "</response>";
			break;

		case Udjat::MimeType::json:
			// Format as JSON
			to_json(stream,*this);
			break;

		case Udjat::MimeType::html:
			// Format as HTML.
			to_html(stream,*this);
			break;

		case Udjat::MimeType::yaml:
			// Format as yaml.
			to_yaml(stream,*this);
			break;

		case Udjat::MimeType::csv:
			for_each([&stream](const char *, const Udjat::Value &value){
				if(value == Value::Type::Array) {
					to_csv(stream,value);
					return true;
				}
				return false;
			});
			break;

		case Udjat::MimeType::sh:

			// Format as shell script (only first level)
			for_each([&stream](const char *key, const Udjat::Value &value){

				switch((Value::Type) value) {
				case Udjat::Value::Undefined:
				case Udjat::Value::Array:
					break;

				case Udjat::Value::Object:
					break;

				case Udjat::Value::Signed:
				case Udjat::Value::Unsigned:
				case Udjat::Value::Real:
				case Udjat::Value::Boolean:
				case Udjat::Value::Fraction:
					stream << key << "=" << value << endl;
					break;

				default:
					stream << key << "=\"" << value << "\"" << endl;
				}

				return false;
			});

			break;

		default:
			throw system_error(ENOTSUP,system_category(),Logger::String{"No exporter for ",((MimeType) *this)});

		}

	}

 }
