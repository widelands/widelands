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

#include "widelands.h"
#include "menuecommon.h"
#include "intro.h"


/*
==============================================================================

Intro IMPLEMENTATION

==============================================================================
*/

class Intro : public BaseMenu {
public:
	Intro();
	
	virtual bool handle_mouseclick(uint btn, bool down, int x, int y);
};

Intro::Intro()
	: BaseMenu("splash.bmp")
{
}

bool Intro::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (down)
		end_modal(0);
	
	return true;
}


/*
===============
intro

Show the splash screen
===============
*/
void intro()
{
	Intro i;
	i.run();
}
