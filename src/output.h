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
	MessageBox(NULL, str, "Wide Lands", MB_ICONINFORMATION);
#else
		  cout << str << endl;
#endif
		  
}

#endif /* __S__OUTPUT_H */
