/*
 * Copyright (C) 2002 by Holger Rapp 
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

#ifndef __S__OS_H
#define __S__OS_H

#ifndef NULL
	#define NULL 0
#endif

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#define SEP			"\\"
	#define CSEP		'\\'
	#ifdef _DEBUG			//chk ths plz. Acked by Holger
		#define DEBUG
	#endif //_DEBUG
#else //WIN32
	#include <unistd.h>
	#define strcmpi strcasecmp
	#define strncmpi strncasecmp
	#define SEP			"/"
	#define CSEP		'/'
#endif //WIN32

#endif /* __S__OS_H */
