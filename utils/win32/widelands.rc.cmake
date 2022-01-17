/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

// To change version information, edit FILEVERSION and PRODUCTVERSION. Keep these two identical.
// Version "A,B,C,D" translates to version "A.B.C.D",
// e.g. for version "1.2" we need "1,2,0,0".
// https://docs.microsoft.com/en-us/windows/win32/menurc/versioninfo-resource
1 VERSIONINFO
FILEVERSION 0,21,0,1
PRODUCTVERSION 0,21,0,1
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
			 VALUE "WWW", "https://www.widelands.org"
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
