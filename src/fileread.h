/*
 * Copyright (C) 2006 by the Widelands Development Team
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

#ifndef FILEREAD_H
#define FILEREAD_H

#include "error.h"
#include "machdep.h"
#include "wexception.h"

#include <string>

struct FileSystem;

/**
 * FileRead can be used to read a file.
 * It works quite naively by reading the entire file into memory.
 * Convenience functions are available for endian-safe access of common data types
 */
class FileRead {
	public:
		void	*data;
		int	filepos;
		int	length;

	public:
		FileRead();
		~FileRead();

		void Open(FileSystem *fs, std::string fname);
		bool TryOpen(FileSystem *fs, std::string fname);
		void Close();

		inline int GetSize() const { return length; }
		inline bool IsEOF() const { if(filepos>=length) return true; return false; }
		void SetFilePos(int pos);
		inline int GetFilePos(void) { return filepos; }

		inline char Signed8(int pos = -1) { return Deref8(Data(1, pos)); }
		inline uchar Unsigned8(int pos = -1) { return (uchar)Deref8(Data(1, pos)); }
		inline short Signed16(int pos = -1) { return Little16(Deref16(Data(2, pos))); }
		inline ushort Unsigned16(int pos = -1) { return (ushort)Little16(Deref16(Data(2, pos))); }
		inline int Signed32(int pos = -1) { return Little32(Deref32(Data(4, pos))); }
		inline uint Unsigned32(int pos = -1) { return (uint)Little32(Deref32(Data(4, pos))); }
		inline float Float(int pos = -1) { return LittleFloat(DerefFloat(Data(4, pos))); }
		char *CString(int pos = -1);
		bool ReadLine(char *buf, int buflen);

		void *Data(int bytes, int pos = -1) {
			int i;

			assert(data);

			i = pos;
			if (pos < 0) {
				i = filepos;
				filepos += bytes;
			}
			if (i+bytes > length)
				throw wexception("File boundary exceeded");

			return (char*)data + i;
		}
};

#endif
