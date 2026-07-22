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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/string.h>
 #include <cstring>
 #include <string>
 #include <stdexcept>
 #include <civetweb.h>
 #include <iostream>
 #include <list>
 #include <cstring>

 using namespace Udjat;
 using namespace std;

 /// @brief Handler for icon requests.
 UDJAT_PRIVATE int defaultWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for icon requests.
 UDJAT_PRIVATE int iconWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for product requests.
 UDJAT_PRIVATE int productWebHandler(struct mg_connection *conn, void *cbdata) noexcept;

 /// @brief Handler for image requests.
 UDJAT_PRIVATE int imageWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Authentication handler.
 UDJAT_PRIVATE int oauthWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief User information handler.
 UDJAT_PRIVATE int userWebHandler(struct mg_connection *conn, void *cbdata);

 /// @brief Handler for '/favicon.ico' request.
 UDJAT_PRIVATE int faviconWebHandler(struct mg_connection *conn, void *cbdata) noexcept;

 /// @brief Get mime-type from 'Accept' or 'Content-Type' header.
 /// @param conn Civetweb connection data.
 /// @param def The mimetype to use if connection doesnt set one.
 UDJAT_PRIVATE Udjat::MimeType MimeTypeFactory(struct mg_connection *conn, const Udjat::MimeType def = Udjat::MimeType::json) noexcept;

 