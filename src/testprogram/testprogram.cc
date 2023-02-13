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
 #include <udjat/tools/logger.h>
 #include <udjat/tools/application.h>
 #include <udjat/moduleinfo.h>
 #include <udjat/module.h>
 #include <udjat/tools/logger.h>
 #include <udjat/factory.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

 static const ModuleInfo moduleinfo{"civetweb-tester"};

 class RandomFactory : public Udjat::Factory {
 public:
	RandomFactory() : Udjat::Factory("random",moduleinfo) {
		cout << "random agent factory was created" << endl;
		srand(time(NULL));
	}

	std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object &, const pugi::xml_node &node) const override {

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

 int main(int argc, char **argv) {

	Logger::verbosity(9);
	Logger::redirect();

	RandomFactory factory;

	udjat_module_init();
	auto rc = Application{}.run(argc,argv,"./test.xml");

	debug("Application exits with rc=",rc);

	return rc;

 }
