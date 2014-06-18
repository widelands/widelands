/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include <windows.h> // included for version info constants

A ICON MOVEABLE PURE LOADONCALL DISCARDABLE "@CMAKE_CURRENT_SOURCE_DIR@/../utils/win32/WL.ico"

//
// TO CHANGE VERSION INFORMATION, EDIT  OPTIONS BELOW...
//
1 VERSIONINFO
FILEVERSION 0,18,0,1
PRODUCTVERSION 0,18,0,1
FILETYPE VFT_APP
{
  BLOCK "StringFileInfo"
	 {
		 BLOCK "080904E4"
		 {
			 VALUE "CompanyName", "The Widelands Development Team"
			 VALUE "FileVersion", "@WL_VERSION@"
			 VALUE "FileDescription", "Widelands - realtime strategy game"
			 VALUE "InternalName", "WL"
			 VALUE "LegalCopyright", "GPL v2"
			 VALUE "WWW", "http://www.widelands.org"
			 VALUE "License", "Gnu GPL v2"
			 VALUE "OriginalFilename", "widelands"
			 VALUE "ProductName", "Widelands - an Open-Source realtime strategy game"
			 VALUE "ProductVersion", "@WL_VERSION@"
		 }
	 }
  BLOCK "VarFileInfo"
	 {
		 VALUE "Translation", 0x0809, 1252
	 }
}

