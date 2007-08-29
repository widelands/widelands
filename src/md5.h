/*
 * Thanks to Ulrich Drepper for the md5sum example code
 *
 * Copyright (C) 2002, 2007 by the Widelands Development Team
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

#ifndef __S__MD5_H
#define __S__MD5_H

#include <string>

#include "streamwrite.h"

/* Structure to save state of computation between the single steps.  */
struct md5_ctx {
	ulong A;
	ulong B;
	ulong C;
	ulong D;

	ulong total[2];
	ulong buflen;
	char buffer[128];
};

/**
 * One MD5 checksum is simply an array of 16 bytes.
 */
struct md5_checksum {
	Uint8 data[16];

	std::string str() const;

	bool operator==(const md5_checksum& o) const
	{
		return memcmp(data, o.data, sizeof(data)) == 0;
	}

	bool operator!=(const md5_checksum& o) const {return not (*this == o);}
};

/**
 * This class is responsible for creating a streaming md5 checksum.
 * You simply pass it the data using stream operations, and if you want
 * to read the checksum, first call FinishChecksum(), followed by
 * GetChecksum().
 *
 * Instances of this class can be copied.
 */
class MD5Checksum : public StreamWrite {
public:
	MD5Checksum();

	void Reset();

	virtual void Data(const void * data, size_t size);

	void FinishChecksum();
	const md5_checksum& GetChecksum() const;

private:
	bool can_handle_data;
	md5_checksum sum;
	md5_ctx ctx;
};

#endif /* __S__MD5_H */
