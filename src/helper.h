/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#ifndef __S__HELPER_H
#define __S__HELPER_H

// TODO: This _has_ to be rearranged, and those functions aren't used anyway
// mmh, i use them. this should go somewhere into widelands.h, don't you agree? // Holger 

// helper.cc define some helper functions which are
// either usefull or sometimes strangely defined (for example
// some functions are cool but only available on windows or 
// with another syntax on unix), so we either define a wrapper in os.h
// or we make our own new funtion in helper.cc

void zmem(void* mem, const ulong size);
void to_lower(char* str);

#endif 
