/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#ifndef BITOUTBUFFER_H
#define BITOUTBUFFER_H

#include "io/filewrite.h"
#include "compile_assert.h"

/// Wrapper around FileWrite for writing bits to a file. Call put to write a
/// bit to the buffer. Call Write to write the buffer to file. Unassigned bits
/// in the last byte will be 0.
template<uint8_t const Size> struct BitOutBuffer {
	compile_assert(Size == 1 or Size == 2 or Size == 4);
	BitOutBuffer() : buffer(0x00), mask(0xff) {}
	void put(uint8_t const item) {
		assert(item < (1 << Size));
		buffer |= item;
		mask <<= Size;
		if (mask == 0x00) {fw.Unsigned8(buffer); mask = 0xff;}
		buffer <<= Size;
	}
	void Write(FileSystem & fs, const char * const filename) {
		if (mask != 0xff)
		{while (mask <<= Size) buffer <<= Size; fw.Unsigned8(buffer);}
		fw.Write(fs, filename);
	}
private:
	FileWrite fw;
	uint8_t buffer, mask;
};

#endif
