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

 #include <udjat/defs.h>
 #include <udjat/module/abstract.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/http/server.h>
 #include <udjat/tools/interface.h>

 #include <civetweb.h>

 namespace Udjat {

	namespace CivetWeb {

		class Interface : public Udjat::Interface {
		private:

			const char *path = nullptr;

			class Handler : public Udjat::Interface::Handler {
			private:
				HTTP::Method method;

			public:
				Handler(const XML::Node &node) 
					: Udjat::Interface::Handler{node}, method{HTTP::MethodFactory(node)} {
				}					
			};

			std::vector<Handler> handlers;

		public:
			Interface(const XML::Node &node);
			virtual ~Interface();

		};

	}

 }
