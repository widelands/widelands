/*
 * Copyright (C) 2010 The Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ggzdmod.h"
#include "protocol.h"

#include <iostream>
#include <cstdarg>
#include <cstdio>

int send_debug = -1;
int debug_level = 9;

void wllog (int level, const char * fmt, ...)
{
	char buf[2048];
	va_list va;

	if (debug_level < level)
		return;

	va_start(va, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	if (send_debug > 0) {
		ggz_write_int(send_debug, op_debug_string);
		ggz_write_int(send_debug, ggzdatatype_string);
		ggz_write_string(send_debug, buf);
		ggz_write_int(send_debug, 0);
	}

	std::cout << "[WLServer]: " << buf << std::endl;
}
