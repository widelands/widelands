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

#ifndef __S__INTPLAYER_H
#define __S__INTPLAYER_H

#include "mytypes.h"
#include "mapview.h"

class Interactive_Player {
		 Interactive_Player(const Interactive_Player&);
		 Interactive_Player operator=(const Interactive_Player&);

		  public:
					 Interactive_Player(Map*);
					 ~Interactive_Player(void);
					 void interact(void);
					 void grab_input(void);

					 // some inlines
					 inline bool should_end_game(void) { return bshould_endgame; }
					 inline bool should_exit(void) { return bshould_exit; }

		  
		  friend int IP_mmf(const uint, const uint, const int, const int, const bool, const bool, void*);
		  friend int IP_lclick(const bool, const uint, const uint, void* );
		  friend int IP_rclick(const bool, const uint, const uint, void* );
		  
		  private:
					 Map_View* mv;
					 bool bshould_endgame;
					 bool bshould_exit;
};


#endif // __S__INTPLAYER_H
