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
#include "criterr.h"
#include "graphic.h"
#include "input.h"
#include "ui.h"
#include "cursor.h"
#include "font.h"
#include "menuecommon.h"

/** void critical_error(const char* str) 
 *
 * this is a critical error in the program. It will quite surely die.
 *
 * Args:	str			String to give to the user
 * Returns:	nothing, but may die (through exit())
 */
void critical_error(const char* str) {
		  bool* bquit = new bool(false);
		  bool* breturn = new bool(false);
		  
		  // make the background window, fill it with the splash screen
		  Window* win=g_ui.create_window(0, 0, g_gr.get_xres(), g_gr.get_yres(), Window::FLAT);
		  Pic* p = new Pic;
		  p->set_size(g_gr.get_xres(), g_gr.get_yres());
		  
/*		  p->set_pixel(g_gr.get_xres()-1, g_gr.get_yres()-1, 0, 0, 0);
		  for(uint i=g_gr.get_xres()*g_gr.get_yres()-2; i--; ) 
					 p->set_ppixel(0,0,0);*/
		  p->clear_all();
		  win->set_new_bg(p);		 

		  // Create the different areas
		  win->create_textarea(0, 150, g_gr.get_xres(), Textarea::CENTER)->set_text("!! CRITICAL ERROR !!");

		  win->create_textarea(0, 200, g_gr.get_xres(), Textarea::CENTER)->set_text(str);
		  // Create the buttons
		  Button* b;
		  b=win->create_button((g_gr.get_xres()>>1)-85, g_gr.get_yres()-200, 170, 20, 1);
		  b->register_func(menue_butclick_func, bquit);
		  b->set_pic(g_fh.get_string("Exit", 0));

		  b=win->create_button((g_gr.get_xres()>>1)-85, g_gr.get_yres()-250, 170, 20, 1);
		  b->register_func(menue_butclick_func, breturn);
		  b->set_pic(g_fh.get_string(" !! Continue execution !! ", 0));

		  // Register the resposible mouse funtions
		  // They should already be registered. or else backup the move functions and restore them
		  // in case of a return. But this shouldn't be needed in the game
//		  g_ip.register_mcf(menue_lclick, Input::BUT1);
//		  g_ip.register_mcf(menue_rclick, Input::BUT2);
//	     g_ip.register_mmf(menue_mmf);

		  while(!g_ip.should_die() && !*bquit && !*breturn) {
					 menue_loop();
		  }		

		  if(*bquit) {
					 g_ui.delete_all_windows();
					 exit(-1);
		  }
		  else if(*breturn) {
					 g_ui.delete_all_windows();
					 // we return, though this might kill the app
					 return;
		  }
		  
		  return;
}
