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

#ifndef __S__OUTPUT_H
#define __S__OUTPUT_H

#include <fstream.h>

extern ofstream out;
extern ofstream err;

/** void inline tell_user(const char* str) 
 *
 * This functions makes sure that the user gets informed of what's going on 
 * It's used since this information should reach the user directly 
 * and not been redirected to stderr. 
 *
 * Args: str	String to give to user
 * Returns: Nothing
 */
void inline tell_user(const char* str) {
#ifdef WIN32
//#error 	tell_user function not defined for windows yet!! Use messagebox
// FEAR!! this will probably not work. directx applications do not allow
// message boxes to paint over the primary surface. sdl hides features that
// would make this possible. Florian
// This shouldn't be a problem: This function is just used to inform the user
// of critical errors, before the game is really started: this includes not finding 
// the resources or having some memory problems. The real game will define it's own message
// boxes.
#ifdef _MSC_VER		// i want to use a msvc pragma
#pragma message("warning: tell_user function will probably not work")
#endif
	MessageBox(NULL, str, "Wide Lands", MB_ICONINFORMATION);
#else
		  cout << str << endl;
#endif
		  
}

#endif /* __S__OUTPUT_H */
