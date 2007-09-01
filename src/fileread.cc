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


FileRead::FileRead() : data(0), length(0)
{}

FileRead::~FileRead()
{
	if (data)
		Close();
}

/// \todo error handling
void FileRead::Open(FileSystem & fs, const char * const filename)
{
	assert(!data);

	data = fs.Load(filename, length);
	filepos = 0;
}

bool FileRead::TryOpen(FileSystem & fs, const char * const filename) {
	try {
		Open(fs, filename);
	}
	catch (const std::exception & e) {
		return false;
	}

	return true;
}

void FileRead::Close()
{
	assert(data);

	free(data);
	data = 0;
}

void FileRead::SetFilePos(const Pos pos) {
	assert(data);

	if (pos >= length) throw File_Boundary_Exceeded();

	filepos = pos;
}

char * FileRead::CString(const Pos pos) {
	char *string, *p;

	assert(data);

	Pos i = pos == NoPos() ? filepos : pos;
	if (i >= length) throw File_Boundary_Exceeded();

	string = (char *)data + i;
	for (p = string; *p; p++, i++) ;
	i++; // beyond the NUL

	if (i > length) throw File_Boundary_Exceeded();

	if (pos == NoPos()) {prevpos = filepos; filepos = i;}

	return string;
}

/** FileRead::ReadLine(char *buf, int buflen)
 *
 * Reads a line from the file into the buffer.
 * The '\\r', '\\n' are consumed, but not stored in buf
 *
 * Returns true on EOF condition.
 */
bool FileRead::ReadLine(char * buf, const char * const buf_end) {
	assert(buf < buf_end);
	assert(data);

	if (filepos >= length)
		return false;

	for (;;) {
		const char c = static_cast<char *>(data)[filepos];
		++filepos;

		if (c == '\r') // not perfectly correct, but it should work
			continue;
		if (c == '\n') {
			*buf = 0;
			break;
		}

		*buf = c;
		++buf;
		if (buf == buf_end) {
			buf[-1] = 0;
			throw Buffer_Overflow();
		}
		if (filepos == length) {
			*buf = 0;
			break;
		}
	};
	return true;
}
