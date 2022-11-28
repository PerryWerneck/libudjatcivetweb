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

 #include <udjat/module.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/report.h>
 #include <udjat/worker.h>
 #include <udjat/tools/url.h>
 #include <udjat/factory.h>
 #include <pugixml.hpp>
 #include <unistd.h>
 #include <civetweb.h>
 #include <udjat/civetweb.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/http/icons.h>
 #include <udjat/tools/http/handler.h>
 #include <udjat/tools/http/connection.h>
 #include <udjat/tools/http/server.h>

 using namespace std;
 using namespace Udjat;

 #pragma GCC diagnostic ignored "-Wunused-parameter"
 #pragma GCC diagnostic ignored "-Wunused-function"

//---[ Implement ]------------------------------------------------------------------------------------------

static void test_httpd() {

	static const ModuleInfo moduleinfo{"civetweb-tester"};

	class Factory : public Udjat::Factory {
	public:
		Factory() : Udjat::Factory("random",moduleinfo) {
			cout << "random agent factory was created" << endl;
			srand(time(NULL));
		}

		std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object &parent, const pugi::xml_node &node) const override {

			class RandomAgent : public Agent<unsigned int> {
			private:
				unsigned int limit = 5;

			public:
				RandomAgent(const pugi::xml_node &node) : Agent<unsigned int>(node) {
					cout << "Creating random Agent" << endl;
				}

				bool refresh() override {
					set( ((unsigned int) rand()) % limit);
					return true;
				}

				void get(const Request UDJAT_UNUSED(&request), Report &report) {

					report.start("sample","row","a","b","c",nullptr);

					for(size_t row = 0; row < 3; row++) {
						string r{"r"};
						r += std::to_string(row);

						report << (string{"["} +r + "]");
						for(size_t col = 0; col < 3;col++) {
							report << (r + "." + std::to_string(col) + "." + std::to_string(((unsigned int) rand()) % limit));
						}
					}


				}

			};

			return make_shared<RandomAgent>(node);

		}

	};

	static Factory factory;

	Udjat::reconfigure("./test.xml",true);
	auto agent = Abstract::Agent::root();

	debug("http://localhost:8989");

	if(Module::find("information")) {
		debug("http://localhost:8989/api/1.0/info/modules.xml");
		debug("http://localhost:8989/api/1.0/info/workers.xml");
		debug("http://localhost:8989/api/1.0/info/factories.xml");
	}

	debug("http://localhost:8989/icon/user-info-symbolic");

	for(auto agent : *agent) {
		debug("http://localhost:8989/api/1.0/agent/", agent->name(), ".xml");
		debug("http://localhost:8989/api/1.0/report/agent/", agent->name(), ".xml");
	}

	class HTest : public Udjat::HTTP::Handler {
	public:
		HTest() : Handler("/test/") {
		}

		int handle(const Udjat::HTTP::Connection &conn, const Udjat::HTTP::Request &request, const Udjat::MimeType mimetype) override {
			// return conn.failed(404,"Test is ok but there's no data");
			const char *ptr = strchr(request.getPath(),'/');
			if(!ptr) {
				throw runtime_error("Invalid");
			}
			return conn.send(request.as_type(),ptr+1,true);
		}
	};

	/*
	HTest test;
	if(HTTP::Server::getInstance().push_back(&test)) {
		cout << "http://localhost:8989/test/test.xml" << endl;
	}
	*/

	Udjat::MainLoop::getInstance().run();

	agent->deinit();

}

void test_http_get() {

	try {

		Udjat::URL url("http://localhost");
		cout << "Response for " << url << ": " << endl << url.get() << endl;

	} catch(const std::exception &e) {

		cerr << "Exception: " << e.what() << endl;

	}

}

void test_http_test() {

	cout << "HTTP Test result: " << URL("http://localhost").test() << endl;

}

static void test_report() {

	HTTP::Report report{"http://sample",MimeType::html};

	report.start("sample","v1","v2","v3",nullptr);

	report 	<< 1
			<< 2
			<< 3
			<< "4"
			<< "5"
			<< "6";

	cout << report.to_string() << endl;
}

int main(int argc, char **argv) {

	Logger::redirect();

	setlocale( LC_ALL, "" );

	Module * module = udjat_module_init();

	/*
	if(URL("http://127.0.0.1/~perry/test.xml").get("/tmp/localhost.html")) {
		cout << endl << endl << "File was updated!" << endl << endl;
	}
	*/

	// test_httpd();
	// test_http_get();
	test_http_test();
	// test_report();

	// cout << HTTP::Icon("document-send-symbolic") << endl;
	// cout << HTTP::Icon("dialog-password-symbolic") << endl;
	// cout << HTTP::Icon("image-missing") << endl;

	delete module;

	Module::unload();

	return 0;
}
