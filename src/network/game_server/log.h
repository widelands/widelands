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

#ifndef __LOG_H__
#define __LOG_H__

extern int send_debug;
extern int debug_level;

#define DL_FATAL 1
#define DL_ERROR 2
#define DL_WARN 3
#define DL_INFO 4
#define DL_DEBUG 5
#define DL_DUMP 6

#define DL_DUMPDATA 9

void wllog (int level, const char * fmt, ...);

#endif // __LOG_H__
