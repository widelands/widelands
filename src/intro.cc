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
#include "intro.h"
#include "graphic.h"
#include "cursor.h"

static bool end_run; // it's better than making it private static

// 
// class Intro
//
AutoPic Intro::splash("splash.bmp", 640, 480);

Intro::Intro(void) {
   end_run=false;
};

Intro::~Intro(void) {
}
               
void Intro::run(void) {
	g_gr.needs_fs_update();
	while(!end_run)
	{
		static System::InputCallback icb = {
			&Intro::mclick,
		   0,
         0
      };
	
		g_sys.handle_input(&icb);

		if (g_gr.does_need_update()) {
			copy_pic(g_gr.get_screenbmp(), &splash, 0, 0, 0, 0, splash.get_w(), splash.get_h());
			g_gr.update();
		}
	
      g_gr.needs_fs_update();
	}
	// Done
	
}

void Intro::mclick(bool b, int but, uint buts, int x, int y) {
   if(b)
      end_run=true;
}
