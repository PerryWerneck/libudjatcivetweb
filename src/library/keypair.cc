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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/http/keypair.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/base64.h>
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

				if(RSA_generate_key_ex(key,1024,bne,nullptr) != 1) {
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

		String KeyPair::encrypt(const char *from) {

			lock_guard<mutex> lock(guard);

			if(!(str && key)) {
				throw runtime_error("Unable to encrypt data");
			}

			size_t szBuffer = RSA_size((RSA *) key);
			unsigned char to[szBuffer];
			memset(to,0,szBuffer);

			int szOut = RSA_private_encrypt(strlen(str), (unsigned char *) from, to, (RSA *) key, RSA_PKCS1_PADDING);
			if(szOut < 1) {
				throw runtime_error("Unable to encrypt data block");
			}

			return Base64::encode(to,szOut);

		}

#else

		void KeyPair::reset() {
		}

		KeyPair::~KeyPair() {
		}

#endif // HAVE_LIBSSL

	}

 }
