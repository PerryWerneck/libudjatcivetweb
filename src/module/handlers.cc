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
 #include <udjat/defs.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/response.h>
 #include <udjat/tools/civetweb/connection.h> 
 #include <udjat/tools/civetweb/service.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>
 #include <private/request.h>
 #include <private/oauth.h>

 namespace Udjat {

	int CivetWeb::Service::default_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept {
		debug("-------- ",__FUNCTION__,"(",mg_get_request_info(conn)->local_uri,") --------");
		CivetWeb::Connection client{conn};
		CivetWeb::Request request{client};
		return (int) client.handle(request);
	}

	int CivetWeb::Service::api_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept {
		debug("-------- ",__FUNCTION__,"(",mg_get_request_info(conn)->local_uri,") --------");
		CivetWeb::Connection client{conn};
		CivetWeb::Request request{client};

		class Response : public HTTP::Response {
		private:
			HTTP::Connection &conn;
		public:
			Response(HTTP::Connection &c, Udjat::MimeType mimetype) : HTTP::Response{mimetype}, conn{c} {
			}

			HTTP::Connection & connection() const override {
				return conn;
			}

		};

		Response response{client,request.mimetype()};

		return (int) client.apicall(request,response);
	}

	int CivetWeb::Service::favicon_handler(struct mg_connection *conn, CivetWeb::Service *) noexcept {
		debug("-------- ",__FUNCTION__,"(",mg_get_request_info(conn)->local_uri,") --------");
		return (int) CivetWeb::Connection{conn}.favicon();
	}

	int CivetWeb::Service::icon_handler(struct mg_connection *conn, CivetWeb::Service *) noexcept {
		debug("-------- ",__FUNCTION__,"(",mg_get_request_info(conn)->local_uri,") --------");
		return (int) CivetWeb::Connection{conn}.icon(mg_get_request_info(conn)->local_uri);
	}

	int CivetWeb::Service::image_handler(struct mg_connection *conn, CivetWeb::Service *) noexcept {
		debug("-------- ",__FUNCTION__,"(",mg_get_request_info(conn)->local_uri,") --------");
		return (int) CivetWeb::Connection{conn}.image(mg_get_request_info(conn)->local_uri);
	}

	int CivetWeb::Service::theme_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept {
		debug("-------- ",__FUNCTION__,"(",mg_get_request_info(conn)->local_uri,") --------");
		return (int) CivetWeb::Connection{conn}.theme(mg_get_request_info(conn)->local_uri);
	}

	int CivetWeb::Service::auth_handler(struct mg_connection *conn, CivetWeb::Service *) noexcept {

		debug("-------- ",__FUNCTION__,"(",mg_get_request_info(conn)->local_uri,") --------");
		CivetWeb::OAuthContext context{conn};
		try {

			return context.handle();

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error();

			return (int) context.send_html_response(
				HTTP::SystemError,
				_("An unexpected error occurred during the login process.")
			);

		}

	}

	int CivetWeb::Service::user_handler(struct mg_connection *conn, CivetWeb::Service *srvc) noexcept {

		debug("------------------ ",__FUNCTION__," ------------------");

		CivetWeb::Connection connection{conn};

		if(!connection.allow(Authentication::Guest)) {
			return CivetWeb::OAuthContext(conn).authenticate();
		}

		debug("User is authenticated");

		return HTTP::SystemError;

	}

// 	int CivetWeb::Service::product_handler(struct mg_connection *conn, CivetWeb::Service *) noexcept {

// 		CivetWeb::Connection client{conn};

// 		try {

// #ifdef _WIN32
// 			Application::DataFile htdocs = Config::Value<string>{"httpd","doc-path","www/htdocs/"}.c_str();
// #else
// 			Application::DataFile htdocs = Config::Value<string>{"httpd","doc-path","/srv/www/htdocs/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/"}.c_str();
// #endif // _WIN32

// 			String filename {
// 				htdocs.c_str(),
// 				client.local_uri()
// 			};

// 			return (int) client.send(
// 				filename.c_str(),
// 				(time_t) Config::Value<unsigned int>{"theme","file-max-age",3600},
// 				MimeTypeFactory(filename.c_str())
// 			);

// 		} catch(const std::exception &e) {

// 			return (int) client.send(e,_("Unexpected error"));

// 		} catch(...) {

// 			return (int) client.send(HTTP::SystemError,_("Unexpected error"));

// 		}

// 	}

 }


