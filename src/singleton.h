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

#ifndef __S__SINGLETON_H
#define __S__SINGLETON_H

/* Referenz: Game Programming Gems I, P. 38 
 * Excellent code!!
 */

template <typename T> class Singleton {
		  static T* ms;

		  public:
		  Singleton(void) {
					 assert (!ms) ;
					 int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
					 ms=(T*)((int)this+offset);
		  }
		  ~Singleton(void) { assert(ms); ms=0; }
		  static inline T& get_singleton(void) { assert (ms); return *ms; }
		  static inline T* get_ptsingleton(void) { assert(ms); return ms; }
};

template <typename T> T* Singleton <T>::ms=0;


#endif
