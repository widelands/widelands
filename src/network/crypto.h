/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_NETWORK_CRYPTO_H
#define WL_NETWORK_CRYPTO_H

#include <string>

#include <boost/version.hpp>

namespace crypto {

/**
 Hashes the given input string with SHA-1 and returns the hash.
 @param input A string to calculate the hash of.
 @return The hash as hex-string.
 */
std::string sha1(const std::string& input);
}  // namespace crypto

#endif  // end of include guard: WL_NETWORK_CRYPTO_H
