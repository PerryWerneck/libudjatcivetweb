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

 #include <udjat.h>
 #include <udjat/module.h>
 #include <udjat/tools/logger.h>
 #include <udjat/worker.h>
 #include <pugixml.hpp>
 #include <unistd.h>
 #include <civetweb.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

static void test_httpd() {

	auto root_agent = Abstract::Agent::set_root(make_shared<Abstract::Agent>("root","System","Application"));

	cout << "http://localhost:8989/info/1.0/modules" << endl;
	cout << "http://localhost:8989/info/1.0/workers" << endl;
	cout << "http://localhost:8989/info/1.0/factory" << endl;
	cout << "http://localhost:8989/swagger.json" << endl;

	Udjat::run();

}

void test_http_get(const char *url) {

	char error_buffer[256] = "";

	struct mg_connection *conn =
		mg_download(
			"localhost",
			80,
			0,
			error_buffer,
			sizeof(error_buffer),
			"GET %s HTTP/1.0\r\n\r\n",
			"http://localhost"
		);

	if(!conn) {
		cout << error_buffer << endl;
		return;
	}

	const struct mg_response_info *info = mg_get_response_info(conn);

	cout << "Status: " << info->status_code << " " << info->status_text << endl;

	cout << "Length: " << info->content_length << endl;


	mg_close_connection(conn);

}

int main(int argc, char **argv) {

	//Logger::redirect();

	auto module = udjat_module_init();

	// test_httpd();
	test_http_get("http://localhost/");

	delete module;
	return 0;
}
