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
 #include <udjat/tools/http/timestamp.h>
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
		clear();
		token(b64);
	}

	std::string HTTP::Authentication::cookie_name() noexcept {
		return String {
			Application::Name().c_str(),
			"-session"
		};
	}

	bool HTTP::Authentication::token(const char *b64) {
		
		debug(__FUNCTION__,"(",b64,")");

		if(!(b64 && *b64)) {
			debug("Ignoring empty token");
			clear();
			return false;
		}

		try {

			char buffer[4096];
			memset(buffer,0,4096);

			auto len = decrypt(b64, buffer, 4095);
			buffer[len] = 0;

			if(len < sizeof(Token)) {
				clear();
				Logger::String{"The authentication token is too small"}.error();
				return false;
			}

			Token *token = (Token *) buffer;

			if(time(0) > token->expiration_time) {
				clear();
				Logger::String{"The authentication token is expired"}.trace();
				return false;
			}

			this->current_status = token->status;
			this->expiration_time = token->expiration_time;
			
			role(token->role);

			debug("Role=",std::to_string(role()));

			{
				char *ptr = (char *) (token+1);

				// Get user name
				{
					debug("Username: '",ptr,"'");					
					name(ptr);
					ptr += (strlen(ptr)+1);
				}

				// Get e-mail
				{
					debug("E-Mail: '",ptr,"'");					
					email(ptr);
					ptr += (strlen(ptr)+1);
				}

				// Get avatar URL
				{
					debug("Avatar URL: '",ptr,"'");
					avatar_url = ptr;
				}

			}


#ifdef DEBUG
			Logger::String{"Authentication expires on ",TimeStamp(this->expiration_time).to_string().c_str()}.info();
#endif
			return true;

		} catch(const std::exception &e) {

			clear();
			Logger::String{e.what()}.trace();

		}

		return false;

	}

	void HTTP::Authentication::clear() noexcept {
		Udjat::Authentication::clear();
		current_status = Undefined;
		avatar_url = "/icon/avatar-default";
		expiration_time = time(0) + Config::Value<time_t>("authentication","expiration-time",86400);
	}

	std::string HTTP::Authentication::token() const {

		debug("---- Encoding token");

		const char *name = this->name();
		const char *email = this->email();
		size_t szBuffer = sizeof(Token) + avatar_url.size() + strlen(email) + strlen(name) + 3;

		uint8_t buffer[szBuffer+1];
		memset(buffer,0,szBuffer+1);

		Token *token = (Token *) buffer;

		debug(
			"User role: ", std::to_string(role()),
			" Username: '",name,"'"
		);

		token->status = this->current_status;
		token->expiration_time = expiration_time;
		token->role = role();

		char *ptr = (char *) (token+1);

		// Append user name
		{
			size_t length = strlen(name);
			memcpy(ptr,name,length);
			ptr[length] = 0;
			ptr += (length+1);
		}

		// Append email
		{
			size_t length = strlen(email);
			memcpy(ptr,email,length);
			ptr[length] = 0;
			ptr += (length+1);
		}

		// Append avatar URL
		{
			size_t length = avatar_url.size();
			memcpy(ptr,avatar_url.c_str(),length);
			ptr[length] = 0;
		}

		return Authentication::encrypt(token,szBuffer);

	}

	void HTTP::Authentication::http_headers(const std::function<void(const char *name, const char *value)> &callback) const noexcept {

		callback(
			"Set-Cookie",
			String{
				cookie_name().c_str(),"=",
				token().c_str(),
				"; path=/; Expires=",
				HTTP::TimeStamp::to_string(expires()).c_str()
			}.c_str()
		);

	}

 }
