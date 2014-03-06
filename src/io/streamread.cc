/*
 * Copyright (C) 2007-2009 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "io/streamread.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>

#include "wexception.h"

StreamRead::~StreamRead() {}

StreamRead::_data_error::_data_error(char const * const fmt, ...) {
	char buffer[256];
	{
		va_list va;
		va_start(va, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, va);
		va_end(va);
	}
	m_what += buffer;
}

/**
 * Read a number of bytes from the stream.
 *
 * If the requested number of bytes couldn't be read, this function
 * fails by throwing an exception.
 */
void StreamRead::DataComplete(void * const data, const size_t size)
{
	size_t read = Data(data, size);

	if (read != size)
		throw data_error
			("Stream ended unexpectedly (%lu bytes read, %lu expected)",
			 static_cast<long unsigned int>(read),
			 static_cast<long unsigned int>(size));
}

int8_t StreamRead::Signed8() {
	int8_t x;
	DataComplete(&x, 1);
	return x;
}

uint8_t StreamRead::Unsigned8() {
	uint8_t x;
	DataComplete(&x, 1);
	return x;
}

int16_t StreamRead::Signed16() {
	int16_t x;
	DataComplete(&x, 2);
	return Little16(x);
}

uint16_t StreamRead::Unsigned16() {
	uint16_t x;
	DataComplete(&x, 2);
	return Little16(x);
}

int32_t StreamRead::Signed32() {
	int32_t x;
	DataComplete(&x, 4);
	return Little32(x);
}

uint32_t StreamRead::Unsigned32() {
	uint32_t x;
	DataComplete(&x, 4);
	return Little32(x);
}

std::string StreamRead::String()
{
	std::string x;
	char ch;

	for (;;) {
		DataComplete(&ch, 1);

		if (ch == 0)
			break;

		x += ch;
	}

	return x;
}
