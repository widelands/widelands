/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__UNICODE_H
#define __S__UNICODE_H

/* 
 * This contains some functions and definitions
 * to get unicode to run under all OSes
 */

/*
 * This function has another name under unix <-> windows
 * do not know why
 */
#ifdef WIN32
#define swprintf snwprintf
#endif

/*
 * std::string<wchar_t> doesn't seem to work under my windows version
 */
#ifdef WIN32
#define std::string<wchar_t> std::wstring 
#endif

#endif 
