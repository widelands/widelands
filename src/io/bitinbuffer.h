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

#ifndef BITINBUFFER_H
#define BITINBUFFER_H

#include "fileread.h"

#include "compile_assert.h"

/// Wrapper around FileRead for reading groups of Size bits from a file, where
/// Size can be a factor of 8. Call get to read Size bits from the buffer.
/// Attempts to read more bits than have been written to the BitOutBuffer when
/// the file was created will not fail unless it causes another byte to be read
/// from the file.
template<uint8_t const Size> struct BitInBuffer {
	compile_assert(Size == 1 or Size == 2 or Size == 4);
	BitInBuffer() : mask(0x00) {}
	void Open(FileSystem & fs, const char * const filename)
	{fr.Open(fs, filename);}
	size_t GetSize() const throw () {return fr.GetSize  ();}
	bool EndOfFile() const throw () {return fr.EndOfFile();}
	size_t GetPos () const throw () {return fr.GetPos   ();}
	uint8_t get() {
		if (mask == 0x00) {buffer = fr.Unsigned8(); mask = 0xff;}
		uint8_t const result = buffer >> (8 - Size);
		buffer <<= Size;
		mask   <<= Size;
		assert(result < (1 << Size));
		return result;
	}
private:
	FileRead fr;
	uint8_t buffer, mask;
};

#endif
