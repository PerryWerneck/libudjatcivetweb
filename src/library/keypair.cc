/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Implements keypair singleton.
  */

 // TODO:
 //
 //	References: 
 //		https://blog.cloudflare.com/pt-br/the-linux-crypto-api-for-user-applications/
 //		https://github.com/smuellerDD/libkcapi
 //

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/http/keypair.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/base64.h>
 #include <udjat/tools/configuration.h>
 #include <stdexcept>

#ifdef HAVE_LIBSSL
	#include <openssl/pem.h>
	#include <openssl/err.h>
#endif // HAVE_LIBSSL

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		KeyPair::KeyPair() {
			reset();
		}

		KeyPair & KeyPair::getInstance() {
			static KeyPair instance;
			return instance;
		}

#ifdef HAVE_LIBSSL

		KeyPair::~KeyPair() {
			RSA_free((RSA *) key);
		}

		void KeyPair::reset() {

			if(this->key) {
				RSA_free((RSA *) this->key);
				this->key = nullptr;
			}

			BIGNUM * bne = BN_new();

			try {

				if(BN_set_word(bne,RSA_F4) != 1) {
					throw runtime_error(Logger::String{"Error creating RSA key pair: ",ERR_lib_error_string(ERR_get_error())});
				}

				RSA *key = RSA_new();
				if(!key) {
					throw runtime_error(Logger::String{"Error creating RSA key pair: ",ERR_lib_error_string(ERR_get_error())});
				}

				if(RSA_generate_key_ex(key,Config::Value<unsigned int>{"authenticator","key-length",1024},bne,nullptr) != 1) {
					RSA_free(key);
					throw runtime_error(Logger::String{"Error creating RSA key pair: ",ERR_lib_error_string(ERR_get_error())});
				}

				this->key = (void *) key;

			} catch(...) {

				BN_free(bne);
				throw;

			}

			BN_free(bne);

		}

		bool KeyPair::decrypt(const char *str, void *data, size_t length) {

			size_t szBuffer = RSA_size((RSA *) key);

			debug("length: ", length, " szBuffer: ",szBuffer);
			debug(str);

			unsigned char from[szBuffer+2];
			memset(from,0,szBuffer+2);

			ssize_t szData = Base64::decode((unsigned char *) str,from,szBuffer+1);
			if(szData < 1) {
				Logger::String{"Error decoding Base64"}.error("keypair");
				Logger::String{str}.trace("keypair");
				return false;
			}

			unsigned char to[szBuffer+2];
			memset(to,0,szBuffer+2);
			if(RSA_public_decrypt(szData,from,to,(RSA *) key, RSA_PKCS1_PADDING) != (ssize_t) length) {
				int errcode = ERR_get_error();
				while(errcode) {
					cerr << "ssl\t" << ERR_lib_error_string(errcode) << endl;
					cerr << "ssl\t" << ERR_func_error_string(errcode) << endl;
					cerr << "ssl\t" << ERR_reason_error_string(errcode) << endl;
					errcode = ERR_get_error();
				}
				Logger::String{"Unable to decrypt received data"}.error("keypair");
				Logger::String{str}.error("keypair");
				return false;
			}

			debug("Data block with ",length," bytes decripted");
			memcpy(data,to,length);
			return true;

		}

		String KeyPair::encrypt(const void *from, size_t length) {

			lock_guard<mutex> lock(guard);

			if(!(from && key)) {
				throw runtime_error("Unable to encrypt data");
			}

			size_t szBuffer = RSA_size((RSA *) key);

			debug("length: ", length, " szBuffer: ",szBuffer);

			unsigned char to[szBuffer];
			memset(to,0,szBuffer);

			int szOut = RSA_private_encrypt(length, (unsigned char *) from, to, (RSA *) key, RSA_PKCS1_PADDING);
			if(szOut < 1) {
				int errcode = ERR_get_error();
				while(errcode) {
					cerr << "ssl\t" << ERR_lib_error_string(errcode) << endl;
					cerr << "ssl\t" << ERR_func_error_string(errcode) << endl;
					cerr << "ssl\t" << ERR_reason_error_string(errcode) << endl;
					errcode = ERR_get_error();
				}
				throw runtime_error("Unable to encrypt data block");
			}

			return Base64::encode(to,szOut);

		}

		String KeyPair::to_string() {

			if(key) {
				BIO *mem = BIO_new(BIO_s_mem());
				PEM_write_bio_RSA_PUBKEY(mem, (RSA *) key);
				size_t len = BIO_pending(mem);
				char text[len+1];
				BIO_read(mem, text, len);
				text[len] = 0;
				BIO_free(mem);
				return text;
			}

			return "";
		}

#else

		void KeyPair::reset() {
		}

		KeyPair::~KeyPair() {
		}

		bool KeyPair::decrypt(const char *, void *, size_t) {
			throw system_error(ENOTSUP,system_category(),"No support for encrypt/decrypt in this build");
		}

		String KeyPair::encrypt(const void *, size_t) {
			throw system_error(ENOTSUP,system_category(),"No support for encrypt/decrypt in this build");
		}

#endif // HAVE_LIBSSL

	}

 }
