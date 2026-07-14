/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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

 #undef LOG_DOMAIN
 #define LOG_DOMAIN "auth"

 #include <udjat/authentication.h>
 #include <udjat/tools/http/authentication.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h> 
 #include <udjat/tools/application.h>
 #include <stdexcept>
 #include <string>
 #include <ctime>

 using namespace std; 

 namespace Udjat {

	#pragma pack(1)
	struct Token {
		HTTP::Authentication::Status status;
		Authentication::Role role;
		time_t expiration_time;
	};
	#pragma pack()

	HTTP::Authentication::Authentication(const char *b64) {
		token(b64);
	}

	std::string HTTP::Authentication::cookie_name() noexcept {
		return String {
			Application::Name().c_str(),
			"-session"
		};
	}

	void HTTP::Authentication::token(const char *b64) {
		
		debug(__FUNCTION__,"(",b64,")");

		if(!(b64 && *b64)) {
			debug("Ignoring empty token");
			clear();
			return;
		}

		try {

			char buffer[4096];
			memset(buffer,0,4096);

			auto len = decrypt(b64, buffer, 4095);
			buffer[len] = 0;

			if(len < sizeof(Token)) {
				throw runtime_error("The authentication token is too small");
			}

			Token *token = (Token *) buffer;

			if(time(0) < token->expiration_time) {
				throw runtime_error("The authentication token is expired");
			}

			this->current_status = token->status;
			this->role = token->role;
			this->expiration_time = token->expiration_time;

		} catch(const std::exception &e) {

			clear();
			Logger::String{e.what()}.trace();

		}

	}

	void HTTP::Authentication::clear() noexcept {
		Udjat::Authentication::clear();
		current_status = Undefined;
		role = None;
		avatar_url.clear();
		expiration_time = time(0) + Config::Value<time_t>("authentication","expiration-time",86400);
	}

	std::string HTTP::Authentication::token() const {

		size_t szBuffer = sizeof(Token);

		uint8_t buffer[szBuffer+1];
		memset(buffer,0,sizeof(szBuffer+1));

		Token *token = (Token *) buffer;

		token->status = this->current_status;
		token->role = role;
		token->expiration_time = expiration_time;

		return Authentication::encrypt(token,szBuffer);

	}

 }
