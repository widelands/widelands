/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#include "streamwrite.h"
#include "wexception.h"

#include <stdarg.h>
#include <stdint.h>

StreamWrite::~StreamWrite()
{
}

void StreamWrite::Flush()
{
	// no-op as default implementation
}

void StreamWrite::Printf(const char *fmt, ...)
{
	// Try to do formatting on the stack first, but fallback
	// to heap allocations to accomodate strings of arbitrary length
	char buffer[2048];
	va_list va;

	va_start(va, fmt);
	int i = vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if ((uint32_t)i < sizeof(buffer)) {
		Data(buffer, i);
	} else {
		uint32_t size = sizeof(buffer);
		char* heapbuf = 0;

		do {
			if (i < 0)
				size = 2*size; // old vsnprintf
			else
				size = i+1; // C99-compatible vsnprintf

			delete[] heapbuf;
			heapbuf = new char[size];

			va_start(va, fmt);
			i = vsnprintf(heapbuf, i+1, fmt, va);
			va_end(va);
		} while((uint32_t)i >= size);

		Data(heapbuf, i);

		delete[] heapbuf;
	}
}

