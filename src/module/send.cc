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

 #include <private/module.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <udjat/version.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/intl.h>
 #include <sstream>
 #include <fcntl.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	int CivetWeb::Connection::send(const HTTP::Method method, const char *name, bool allow_index, const char *mime_type, unsigned int maxage) const {

		// Only GET and Head shoud be sent.
		if(method != HTTP::Get && method != HTTP::Head) {
			throw runtime_error(string{"Invalid HTTP request: '"} + to_string(method) + "'");
		}

		if(!(name && *name)) {
			throw system_error(ENOENT,system_category(),"Empty file path");
		}

		string filename{name};
		if(filename[filename.size()-1] == '/') {
			filename.resize(filename.size()-1);
		}

		struct stat st;
		if(stat(filename.c_str(), &st) < 0) {
			throw system_error(errno,system_category(),filename);
		}

		if(S_ISREG(st.st_mode)) {
			//
			// It's a file, send it.
			//
			mg_response_header_start(conn, 200);

			if(maxage) {
				mg_response_header_add(conn, "Cache-Control", (string{"public,max-age="} + std::to_string(maxage) + ",immutable").c_str(), -1);
				mg_response_header_add(conn, "Expires", HTTP::TimeStamp(time(0)+maxage).to_string().c_str(), -1);
			}

			mg_response_header_add(conn, "Last-Modified", HTTP::TimeStamp(st.st_mtime).to_string().c_str(), -1);

			if(method == HTTP::Get) {
				mg_response_header_add(conn, "Content-Length", std::to_string(st.st_size).c_str(), -1);
			}

			if(mime_type && *mime_type) {
				mg_response_header_add(conn, "Content-Type", mime_type, -1);
			} else {

				const char *ext = strrchr(name,'.');
				if(ext) {
					ext++;
					auto mtype = MimeTypeFactory(ext);

					debug("Detected mime-type is '",mtype,"'");
					if(mtype != MimeType::custom) {
						mg_response_header_add(conn, "Content-Type", std::to_string(mtype), -1);
					}

				}

			}

			mg_response_header_send(conn);

			if(method == HTTP::Get) {
				mg_send_file_body(conn,filename.c_str());
			}

		} else if(S_ISDIR(st.st_mode) && allow_index) {
			//
			// It's a directory
			//

			if(name[strlen(name)-1] != '/') {
				//
				// It's a directory but the URL is requesting a file.
				//
				cerr << "civetweb\tClient has requested directory '" << filename << "' without an ending '/'" << endl;

				//
				// Redirect to correct url.
				//
				// mg_response_header_start(conn, 301);
				// mg_response_header_add(conn, "Location", (string{name} + '/').c_str(), -1);
				// mg_response_header_send(conn);
				// return 301;

				mg_response_header_start(conn, 400);
				mg_response_header_send(conn);

				return 400;
			}

			//
			// Send index
			//
			mg_response_header_start(conn, 200);

			if(maxage) {
				mg_response_header_add(conn, "Cache-Control", (string{"public,max-age="} + std::to_string(maxage) + ",immutable").c_str(), -1);
				mg_response_header_add(conn, "Expires", HTTP::TimeStamp(time(0)+maxage).to_string().c_str(), -1);
			}
			mg_response_header_add(conn, "Content-Type", std::to_string(MimeType::html), -1);

			stringstream response;

			const char * basename = strrchr(name,'/');
			if(basename) {
				basename = strrchr(name,'/');
			}
			if(basename) {
				basename++;
			}

			response	<< "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"><html><head><title>Index of "
						<< basename
						<< "</title></head><body><h1>Index of "
						<< basename
						<< "</h1><hr /><pre>";

#if UDJAT_CORE_BUILD < 22122217
			File::Path::for_each(filename.c_str(),[&response](const char *name, const File::Stat &st) {

				name = strrchr(name,'/');
				if(!name) {
					return true;
				}
				name++;

				if(name[0] == '.') {
					return true;
				}

				response << "<a href=\"" << name;
				if((st.st_mode & S_IFMT) == S_IFDIR) {
					response << '/';
				}
				response	<< "\">"
							<< name
							<< "</a>" << endl;

				return true;
			});
#else
			File::Path{filename}.for_each([&response](const File::Path &file) {

				const char *name = strrchr(file.c_str(),'/');
				if(!name) {
					return true;
				}
				name++;

				response << "<a href=\"" << name;
				if(file.dir()) {
					response << '/';
				}
				response	<< "\">"
							<< name
							<< "</a>" << endl;

				return false;
			});
#endif // UDJAT_CORE_BUILD

			response << "</pre><hr /></body></html>";

			string str{response.str()};

			mg_response_header_add(conn, "Content-Length", std::to_string(str.size()).c_str(), -1);
			mg_response_header_send(conn);

			mg_write(conn, str.c_str(), str.size());

		} else {
			//
			// Invalid file type.
			//
			cerr << "civetweb\t'" << filename << "' is not allowed" << endl;
			throw system_error(ENOENT,system_category(),filename);

		}

		return 200;
	}

 }

 int send(struct mg_connection *conn, const Udjat::Abstract::Response &response) noexcept {

	const MimeType mimetype{MimeTypeFactory(conn)};
	int code = HTTP::Exception::code(response.status_code());
	string message;

	try {

		std::string text{response.to_string()};
		if(code == 200 && text.empty()) {
			code = 204;
		}

		// Build and send header
		mg_response_header_start(conn, code);

		if(code < 200 || code > 299) {

			// It's an error, log it, ignore cache and test for an alternative text output.

			const struct mg_request_info *request_info = mg_get_request_info(conn);

			Logger::String{
				request_info->remote_addr," ",
				request_info->request_method," ",
				request_info->local_uri," HTTP Error ",
				std::to_string(code)," - ",response.message()," (Error ",std::to_string(response.status_code()),")"
			}.warning("civetweb");

			mg_response_header_add(conn, "Cache-Control", "no-cache, no-store, must-revalidate, private, max-age=0", -1);
			mg_response_header_add(conn, "Expires", "0", -1);

			// Check for customized error page.
			try {

				if(Config::Value<bool>("httpd","error-templates",true) && mimetype != MimeType::custom) {

					// Try to load custom error page.
	#ifdef DEBUG
					Application::DataFile page{"./templates/error."};
	#else
					Application::DataFile page{"templates/www/error."};
	#endif // DEBUG
					page += to_string(mimetype,true);

					debug("Checking for http error template in file '",page.c_str(),"'");

					if(!access(page.c_str(),R_OK)) {

						Logger::String{"Loading error page from '",page.c_str(),"'"}.trace("civetweb");

						text = page.load().expand([&response,code](const char *key, std::string &value) {

							if(!strcasecmp(key,"code")) {
								value = std::to_string(code);
							} else if(!strcasecmp(key,"message")) {
								value = response.message();
							} else if(!strcasecmp(key,"body")) {
								value = response.body();
#ifdef DEBUG
								if(!*response.body()) {
									value = "No body on this error (DEBUG)";
								}
#endif // DEBUG
							} else if(!strcasecmp(key,"syscode")) {
								value = std::to_string(response.status_code());
							}

							return !value.empty();

						},true,true);

					}
				}

			} catch(const std::exception &e) {

				Logger::String{"Can't load custom error page: ",e.what()}.trace("civetweb");

			}

		} else {

			// It's not an error, setup cache
			time_t now = time(0);

			time_t modtime = response.last_modified();
			if(!modtime) {
				modtime = now;
			}
			mg_response_header_add(conn, "Last-Modified", HTTP::TimeStamp{modtime}.to_string().c_str(), -1);

			time_t expires = response.expires();
			if(expires && expires >= now) {

				// https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control
				mg_response_header_add(conn, "Cache-Control", String{"max-age=",(unsigned int) (now-expires),", must-revalidate, private"}.c_str(), -1);
				mg_response_header_add(conn, "Expires", HTTP::TimeStamp{expires}.to_string().c_str(), -1);

			} else {

				mg_response_header_add(conn, "Cache-Control", "no-cache, no-store, must-revalidate, private, max-age=0", -1);
				mg_response_header_add(conn, "Expires", "0", -1);

			}

		}

		mg_response_header_add(conn, "Content-Type",std::to_string(mimetype),-1);
		mg_response_header_add(conn, "Content-Length", std::to_string(text.size()).c_str(), -1);
		mg_response_header_send(conn);

		// Send response.
		mg_write(conn, text.c_str(), text.size());

		return code;

	} catch( const std::system_error &e ) {
		debug("-----> ",__FUNCTION__,": ",e.what());
		message = e.what();
		code = e.code().value();

	} catch( const std::exception &e ) {
		debug("-----> ",__FUNCTION__,": ",e.what());
		message = e.what();
		code = -1;

	} catch( ... ) {
		debug("-----> ",__FUNCTION__,": ","Unexpected error");
		message = _("Unexpected error");
		code = -1;

	}

	int http_error_code = HTTP::Exception::code(code);
	const struct mg_request_info *request_info = mg_get_request_info(conn);

	Logger::String{
		"Standard 'send' method has failed with exception '",
		message.c_str(),
		"', sending empty HTTP error ",
		http_error_code,
		" to ",
		request_info->remote_addr
	}.error("civetweb");

	if(Logger::enabled(Logger::Trace)) {
		Logger::String{
			request_info->remote_addr," ",
			request_info->request_method," ",
			request_info->local_uri," ",
			http_error_code, " - ", message.c_str()," (syserror ",code,")"
		}.trace("civetweb");
	}

	mg_response_header_start(conn, http_error_code);
	mg_response_header_add(conn, "Cache-Control", "no-cache, no-store, must-revalidate, private, max-age=0", -1);
	mg_response_header_add(conn, "Expires", "0", -1);
	mg_response_header_send(conn);

	//mg_send_http_error(conn, HTTP::Exception::code(code), message.c_str());

	debug("Returning error ",http_error_code);

	return http_error_code;
 }

