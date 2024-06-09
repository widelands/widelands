/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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

#ifndef WL_BASE_CRYPTO_H
#define WL_BASE_CRYPTO_H

#include <array>
#include <cstdint>
#include <cstring>
#include <string>

#include <md5.h>

namespace crypto {

struct MD5Checksum : public std::array<uint8_t, MD5_DIGEST_LENGTH> {
	[[nodiscard]] std::string str() const;
	[[nodiscard]] bool operator==(const MD5Checksum& other) const;
	[[nodiscard]] bool operator!=(const MD5Checksum& other) const {
		return !(*this == other);
	}
};

struct MD5Checksummer {
	MD5Checksummer();

	void data(const void* data, size_t len);
	[[nodiscard]] std::string finish_checksum_str();
	[[nodiscard]] MD5Checksum finish_checksum_raw();
	void reset();

private:
	MD5_CTX context_;
};

[[nodiscard]] inline MD5Checksum md5_raw(const void* data, size_t len) {
	MD5Checksummer checksummer;
	checksummer.data(data, len);
	return checksummer.finish_checksum_raw();
}

[[nodiscard]] inline std::string md5_str(const void* data, size_t len) {
	MD5Checksummer checksummer;
	checksummer.data(data, len);
	return checksummer.finish_checksum_str();
}

[[nodiscard]] inline std::string md5_str(const std::string& str) {
	return md5_str(str.c_str(), str.size());
}

[[nodiscard]] std::string sha1(const void* data, size_t len);

[[nodiscard]] inline std::string sha1(const std::string& str) {
	return sha1(str.c_str(), str.size());
}

}  // namespace crypto

#endif  // end of include guard: WL_BASE_CRYPTO_H
