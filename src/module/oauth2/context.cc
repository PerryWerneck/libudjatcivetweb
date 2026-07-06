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
 #define LOG_DOMAIN "oauth"
 #include <udjat/tools/logger.h>

 #include <udjat/defs.h>
 #include <udjat/tools/http/oauth.h>
 #include <udjat/tools/http/mimetype.h>
 #include <private/oauth.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/configuration.h>
 #include <string>

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	CivetWeb::OAuthContext::OAuthContext(struct mg_connection *c) : OAuth::Context{mg_get_request_info(c)->local_uri}, conn{c} {

		// TODO: Check for 'state' parameter.

	}

	CivetWeb::OAuthContext::~OAuthContext() {
	}

	bool CivetWeb::OAuthContext::getProperty(const char *key, std::string &value) const {

		if(OAuth::Context::getProperty(key,value)) {
			return true;
		}

		// Extract options from request.
		const struct mg_request_info *req_info = mg_get_request_info(conn);
		if (req_info->query_string != NULL) {

			char buffer[4096];
			int result = mg_get_var(
							req_info->query_string, 
							strlen(req_info->query_string), 
							key, 
							buffer, 
							sizeof(buffer)
					);
					
			if(result > 0) {
				value = buffer;
				return true;
			}

		}

		return false;

	}

	int CivetWeb::OAuthContext::send_html_response(int code, const char *text) const {
		size_t szText = strlen(text);
		mg_response_header_start(conn, code);
		mg_response_header_add(conn, "Content-Type",std::to_string(MimeType::html),-1);
		mg_response_header_add(conn, "Content-Length", std::to_string(szText).c_str(), -1);
		send_header();
		mg_write(conn, text, szText);
		return code;
	}

	int CivetWeb::OAuthContext::send_redirect_response(const char *location) const {
		mg_response_header_start(conn, 303);
		mg_response_header_add(conn, "Location",location,-1);
		mg_response_header_add(conn, "Content-Length", "0", -1);
		send_header();
		return 303;
	}

	void CivetWeb::OAuthContext::send_header() const {

		time_t expires = Authentication::expires();
		int max_age = expires - time(0);

		if(Config::Value<bool>("authentication","allow-cache",true) && max_age > 0) {
			mg_response_header_add(conn, "Cache-Control", String{"private, max-age=",max_age}.c_str(),-1);
			mg_response_header_add(conn, "Expires", HTTP::TimeStamp{expires}.to_string().c_str(), -1);
		} else {
			mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
			mg_response_header_add(conn, "Expires", "0", -1);
		}

		// Setup cookie
		// String cookie{
		// 	cookie_name.c_str(),"=",
		// 	token().c_str(),
		// 	"; path=/oauth2; Expires=",
		// 	HTTP::TimeStamp::to_string(expires).c_str()
		// };
		// mg_response_header_add(conn, "Set-Cookie", cookie.c_str(),-1);

		mg_response_header_send(conn);

	}


 }
		
