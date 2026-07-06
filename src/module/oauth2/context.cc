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
 #include <udjat/authentication.h>
 #include <private/oauth.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/memory.h>
 #include <udjat/tools/url.h>
 #include <string>
 #include <private/client.h>
 #include <sstream>

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	CivetWeb::OAuthContext::OAuthContext(struct mg_connection *c) : OAuth::Context{mg_get_request_info(c)->local_uri}, conn{c} {

		auto query_string = mg_get_request_info(conn)->query_string;
		if (query_string) {
			char buffer[4096];

			memset(buffer,0,4096);
			if(mg_get_var(query_string,strlen(query_string), "code", buffer, sizeof(buffer)-1) >= 0) {
				this->code = buffer;
				debug("Code=",this->code.c_str());
			}

			memset(buffer,0,4096);
			if(mg_get_var(query_string,strlen(query_string), "state", buffer, sizeof(buffer)-1) >= 0) {
				char decoded[4096];
				memset(decoded,0,sizeof(decoded));

				size_t sz = decrypt(buffer,decoded,sizeof(buffer)-1);
				decoded[sz] = 0;

				sequencial = *((uint16_t *) decoded);

				char *ptr = (buffer+sizeof(uint16_t));
				debug("Decripted state ----> '",ptr,"'");
			}

		}
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
		send_header(false);
		return 303;
	}

	void CivetWeb::OAuthContext::send_header(bool cookie) const {

		time_t expires = Authentication::expires();
		int max_age = expires - time(0);

		if(Config::Value<bool>("authentication","allow-cache",true) && max_age > 0) {
			mg_response_header_add(conn, "Cache-Control", String{"private, max-age=",max_age}.c_str(),-1);
			mg_response_header_add(conn, "Expires", HTTP::TimeStamp{expires}.to_string().c_str(), -1);
		} else {
			mg_response_header_add(conn, "Cache-Control","no-cache, no-store, must-revalidate, private, max-age=0",-1);
			mg_response_header_add(conn, "Expires", "0", -1);
		}

		if(cookie) {
			// Setup cookie
			String str{
				cookie_name().c_str(),"=",
				token().c_str(),
				"; path=/oauth2; Expires=",
				HTTP::TimeStamp::to_string(expires).c_str()
			};
			mg_response_header_add(conn, "Set-Cookie", str.c_str(),-1);
		}

		mg_response_header_send(conn);

	}

	String CivetWeb::OAuthContext::post(const char *url, const char *payload) const {

		CivetWeb::Client client{URL{url}};
		stringstream response;

		auto code = client.perform(
			HTTP::Post, 
			payload, 
			[&response](uint64_t, uint64_t, const void *data, size_t len){
				response.write((const char *) data,len);
				return false;
			}
		);

		debug("code=",code);
		debug("Response=",response.str().c_str());

		if(code != 200) {
			throw runtime_error(String{"HTTP error",code," posting to ",url});
		}

		return response.str();

	}

 }
		
