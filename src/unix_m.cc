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

// ich weiss, dass meine winmain hier nicht reingehoert.
// ich bin boese. ich werd mir ein z in den namen schreiben.
// FlorianZZZ

#include "os.h"
#include "main.h"
#include "singleton.h"

// let sdl wrap main into its own main
#include <SDL/SDL.h>


#if !defined(WIN32) || (defined(WIN32) && defined(_CONSOLE))
/** This is the unix main function 
 * It's a simple wrapper for g_main
 */
int main(int argn, char** argc) {
			     return g_main(argn, argc);
}

#else // WIN32, !_CONSOLE

/** This is the win32 gui main function 
 * Again, a simple wrapper for g_main
 */
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdLine, int)
{
// TODO
#ifdef _MSC_VER
#pragma message("warning: WinMain does not parse command line arguments")
#else
#error "warning: WinMain does not parse command line arguments"
#endif
	char* args[1];
	args[0] = cmdLine;
	int argcount = 1;

	return g_main(argcount, args);
}

#endif
