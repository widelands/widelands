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

#ifndef __S__DIRENT_H
#define __S__DIRENT_H

#ifndef WIN32
#include <dirent.h>
#include <sys/types.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct dirent
{
	char d_name[MAX_PATH];
};

// ich gehe davon aus, das DIR in der orginal-dirent.h ein handle ist, dessen
// inhalt niemanden interessiert und das es deshalb auch niemanden stört, wenn
// da ganz was anderes drinsteht.

typedef class Dir DIR;
class Dir
{
	friend DIR* opendir(const char*);
	friend dirent* readdir(DIR*);
	friend void closedir(DIR*);

// ich weiss nicht, wie das im original geregelt ist. der fileloc.cpp entnehme
// ich, dass der rückgabepointer von readfile nicht gelöscht werden muss; da
// ein globaler "dirent ret;" ziemlich dumm wäre (mit dem DIR-handle kann man
// mehrere Verzeichnisse parallel durchsuchen, mit einem globalen ret aber nur
// eine Datei abfragen), löse ich das erstmal so: man kann für jeden verzeich-
// nisdurchlauf je eine datei betrachten.
	dirent ret;
	char* name;
	HANDLE find;
	
	Dir(const char* n);
	~Dir();
};

DIR* opendir(const char* name);
dirent* readdir(DIR* dir);
void closedir(DIR* dir);

#endif //WIN32

#endif // __S__DIRENT_H 
