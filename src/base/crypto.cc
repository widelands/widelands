/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "base/crypto.h"

#include "third_party/libmd/include/sha1.h"

namespace crypto {

std::string MD5Checksum::str() const {
	std::string result(value.size() * 2, '?');
	for (size_t i = 0; i < value.size(); ++i) {
		snprintf(result.data() + 2 * i, 3, "%02x", value.at(i));
	}
	return result;
}

bool MD5Checksum::operator==(const MD5Checksum& other) const {
	return memcmp(value.data(), other.value.data(), value.size()) == 0;
}

MD5Checksummer::MD5Checksummer() {
	reset();
}

void MD5Checksummer::reset() {
	libmd::MD5Init(&context_);
}

void MD5Checksummer::data(const void* data, size_t len) {
	libmd::MD5Update(&context_, reinterpret_cast<const uint8_t*>(data), len);
}

std::string MD5Checksummer::finish_checksum_str() {
	std::string result(MD5_DIGEST_STRING_LENGTH - 1 /* correct for implicit null terminator */, '?');
	libmd::MD5End(&context_, result.data());
	reset();
	return result;
}

MD5Checksum MD5Checksummer::finish_checksum_raw() {
	MD5Checksum result;
	libmd::MD5Final(result.value.data(), &context_);
	reset();
	return result;
}

std::string sha1(const void* data, size_t len) {
	libmd::SHA1_CTX context;
	std::string result(
	   SHA1_DIGEST_STRING_LENGTH - 1 /* correct for implicit null terminator */, '?');
	libmd::SHA1Init(&context);
	libmd::SHA1Update(&context, reinterpret_cast<const uint8_t*>(data), len);
	libmd::SHA1End(&context, result.data());
	return result;
}

}  // namespace crypto
