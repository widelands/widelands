/*
 * Copyright (C) 2001 by Holger Rapp 
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

#ifndef __S__COUNTER_H
#define __S__COUNTER_H

#include <SDL/SDL.h>
#include "mytypes.h"

/** class Counter
 *
 * This class is a simple counter for counting ticks
 * 
 * Depends: Nothing, but SDL
 */
class Counter {
		  Counter(const Counter&);
		  Counter& operator=(const Counter&);

		  public:
					 Counter(void);
					 ~Counter(void);
					 
					 void start(void) { ticks=SDL_GetTicks(); is_running=1; }
					 void pause(const bool b) { is_running=b; }
					 
					 inline ulong get_ticks(void) { 
								if(is_running) return SDL_GetTicks()-ticks;
								return ticks; 
					 }
		  
		  private:
					 bool is_running;
					 ulong ticks;
};

#endif /* __S__COUNTER_H */
