/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#ifndef SYSTEM_H
#define SYSTEM_H

#include "graphic.h"
#include <libintl.h>
#include <locale.h>
#include <string>
#include "types.h"

void init_double_game ();

// basic initialization etc..
void Sys_Init();
void Sys_Shutdown();

void Sys_SetRecordFile(const char *filename);
void Sys_SetPlaybackFile(const char *filename);

#endif
