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

#include "fileread.h"
#include "filesystem.h"

/**
 * Create the object with nothing to read
 */
FileRead::FileRead()
{
	data = 0;
}

/**
 * Close the file if open
 */
FileRead::~FileRead()
{
	if (data)
		Close();
}

/**
 * Loads a file into memory.
 * Reserves one additional byte which is zeroed, so that text files can
 * be handled like a normal C string.
 * Throws an exception if the file couldn't be loaded for whatever reason.
 *
 * \todo error handling
 */
void FileRead::Open(FileSystem & fs, const char * const filename)
{
	assert(!data);

	data = fs.Load(filename, &length);
	filepos = 0;
}

/**
 * Works just like Open, but returns false when the load fails.
 */
bool FileRead::TryOpen(FileSystem & fs, const char * const filename)
{
	assert(!data);

	try {
		data = fs.Load(filename, &length);
		filepos = 0;
	} catch(std::exception &e) {
		//log("%s\n", e.what());
		return false;
	}

	return true;
}

/**
 * Frees allocated memory
 */
void FileRead::Close()
{
	assert(data);

	free(data);
	data = 0;
}

/**
 * Set the file pointer to the given location.
 * Raises an exception when the pointer is out of bound
 */
void FileRead::SetFilePos(int pos)
{
	assert(data);

	if (pos < 0 || pos >= length)
		throw wexception("SetFilePos: %i out of bound", pos);

	filepos = pos;
}

/**
 * Read a zero-terminated string from the file
 */
char *FileRead::CString(int pos)
{
	char *string, *p;
	int i;

	assert(data);

	i = pos;
	if (pos < 0)
		i = filepos;
	if (i >= length)
		throw wexception("File boundary exceeded");

	string = (char *)data + i;
	for(p = string; *p; p++, i++) ;
	i++; // beyond the NUL

	if (i > length)
		throw wexception("File boundary exceeded");

	if (pos < 0)
		filepos = i;

	return string;
}

/** FileRead::ReadLine(char *buf, int buflen)
 *
 * Reads a line from the file into the buffer.
 * The '\\r', '\\n' are consumed, but not stored in buf
 *
 * Returns true on EOF condition.
 */
bool FileRead::ReadLine(char *buf, int buflen)
{
	char *dst = buf;

	assert(data);

	if (filepos >= length)
		return false;

	while(filepos < length && buflen > 0) {
		char c = ((char *)data)[filepos];
		filepos++;

		if (c == '\r') // not perfectly correct, but it should work
			continue;
		if (c == '\n') {
			*dst++ = 0;
			buflen--;
			break;
		}

		*dst++ = c;
		buflen--;
	}

	if (!buflen && *(dst-1)) {
		*(dst-1) = 0;
		throw wexception("ReadLine: buffer overflow");
	}

	return true;
}
