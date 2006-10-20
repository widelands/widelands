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

#include "error.h"
#include "filesystem.h"
#include "filewrite.h"
#include <stdarg.h>
#include "wexception.h"

/**
 * Set the buffer to empty
 */
FileWrite::FileWrite()
{
	data = 0;
	length = 0;
	maxsize = 0;
	filepos = 0;
	counter = 0;
}

/**
 * Clear any remaining allocated data
 */
FileWrite::~FileWrite()
{
	if (data)
		Clear();
}

/**
 * Clears the object's buffer
 */
void FileWrite::Clear()
{
	if (data)
		free(data);

	data = 0;
	length = 0;
	maxsize = 0;
	filepos = 0;
	counter = 0;
}

/**
 * Reset the byte counter to zero.
 * All bytes written then are added to the byte counter
 */
void FileWrite::ResetByteCounter(void) {
	counter = 0;
}

/**
 * Returns the number of bytes written since last ResetByteCounter
 */
int FileWrite::GetByteCounter(void) {
	return counter;
}

/**
 * Actually write the file out to disk.
 * If successful, this clears the buffers. Otherwise, an exception
 * is raised but the buffer remains intact (don't worry, it will be
 * cleared by the destructor).
 */
void FileWrite::Write(FileSystem & fs, const char * const filename)
{
	fs.Write(filename, data, length);

	Clear();
}

/**
 * Same as Write, but returns falls if the write fails
 */
bool FileWrite::TryWrite(FileSystem  &fs, const char * const filename)
{
	try {
		fs.Write(filename, data, length);
	} catch(std::exception &e) {
		log("%s\n", e.what());
		return false;
	}

	Clear();
	return true;
}

/**
 * Set the file pointer to a new location. The position can be beyond
 * the current end of file.
 */
void FileWrite::SetFilePos(int pos)
{
	assert(pos >= 0);
	filepos = pos;
}

/**
 * Set the file pointer to a new location. The position can be beyond
 * the current end of file.
 */
int FileWrite::GetFilePos(void)
{
	return filepos;
}

/**
 * Write data at the given location. If pos is -1, write at the
 * file pointer and advance the file pointer.
 */
void FileWrite::Data(const void *buf, int size, int pos)
{
	int i;

	assert(data || !length);

	i = pos;
	if (pos < 0) {
		i = filepos;
		filepos += size;
	}

	if (i+size > length) {
		if (i+size > maxsize) {
			maxsize += 4096;
			if (i+size > maxsize)
				maxsize = i+size;

			data = realloc(data, maxsize);
		}

		length = i+size;
	}

	counter += size;

	memcpy((char*)data + i, buf, size);
}

/**
 * This is a perfectly normal printf (actually it isn't because it's limited
 * to a maximum string size)
 */
void FileWrite::Printf(const char *fmt, ...)
{
	char buf[2048];
	va_list va;
	int i;

	va_start(va, fmt);
	i = vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	if (i < 0)
		throw wexception("FileWrite::Printf: buffer exceeded");

	Data(buf, i, -1);
}
