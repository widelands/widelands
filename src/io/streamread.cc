/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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

#include "io/streamread.h"

#include <cstdarg>

#include "base/wexception.h"
#include "io/machdep.h"

StreamRead::DataError::DataError(char const* const fmt, ...) {
	char buffer[256];
	{
		va_list va;
		va_start(va, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, va);
		va_end(va);
	}
	what_ += buffer;
}

/**
 * Read a number of bytes from the stream.
 *
 * If the requested number of bytes couldn't be read, this function
 * fails by throwing an exception.
 */
void StreamRead::data_complete(void* const read_data, const size_t size) {
	size_t read = data(read_data, size);

	if (read != size) {
		throw data_error(
		   "Stream ended unexpectedly (%" PRIuS " bytes read, %" PRIuS " expected)", read, size);
	}
}

int8_t StreamRead::signed_8() {
	int8_t x;
	data_complete(&x, 1);
	return x;
}

uint8_t StreamRead::unsigned_8() {
	uint8_t x;
	data_complete(&x, 1);
	return x;
}

int16_t StreamRead::signed_16() {
	int16_t x;
	data_complete(&x, 2);
	return little_16(x);
}

uint16_t StreamRead::unsigned_16() {
	uint16_t x;
	data_complete(&x, 2);
	return little_16(x);
}

int32_t StreamRead::signed_32() {
	int32_t x;
	data_complete(&x, 4);
	return little_32(x);
}

uint32_t StreamRead::unsigned_32() {
	uint32_t x;
	data_complete(&x, 4);
	return little_32(x);
}

float StreamRead::float_32() {
	uint32_t x;
	data_complete(&x, 4);
	x = little_32(x);
	float rv;
	memcpy(&rv, &x, 4);
	return rv;
}

std::string StreamRead::string() {
	std::string x;
	char ch;

	for (;;) {
		data_complete(&ch, 1);

		if (ch == 0) {
			break;
		}

		x += ch;
	}

	return x;
}
