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

#include "input.h"
#include "cursor.h"
#include "graphic.h"
#include "ui.h"
#include "fileloc.h"
#include "font.h"
#include "setup.h"
#include "counter.h"
#include "output.h"

void click(const bool b, void* ) {
		  exit(0);
}

/** void main_menue(void);
 * 
 * This functions runs the main menu. There, you can select
 * between different playmodes, exit and so on.
 *
 * Args: None
 * Returns: Nothing
 */
void main_menue(void) {
		  static Font_Handler f; // Global instance for the hole game
		  static User_Interface ui; // Global instance for the hole game

		  // Setup font handler and user interface for the use in widelands
		  setup_fonthandler();
		  setup_ui();

		  // make a window
		  Window* win=g_ui.create_window(0,0,320, 320);
/*		  Pic* p = new Pic;
		  p->load("/home/sirver/.widelands/pics/s2.bmp");
		  win->set_new_bg(p);
		 
		  assert(p);
		  
*/
		  g_ip.register_mcf(click, Input::BUT1);
	//	  g_ip.register_mcf(mcf1, Input::BUT2);
	//	  g_ip.register_mmf(mmf);
		 
		  Counter c;
		  c.start();
		  while(!g_ip.should_die()) {
					 g_ip.handle_pending_input(); 
					 if(c.get_ticks()> 500) {
								g_ui.draw();
								g_cur.draw(g_ip.get_mpx(), g_ip.get_mpy());
								g_gr.update_screen(); // g_gr.update_quarter();
					 }
		  }
		  return; 
}
