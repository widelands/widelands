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

#include "input.h"
#include "cursor.h"
#include "graphic.h"
#include "ui.h"
#include "fileloc.h"
#include "font.h"
#include "setup.h"
#include "counter.h"
#include "output.h"
#include "singlepmenue.h"
#include "menuecommon.h"

#ifndef VERSION
#include "config.h"
#endif /* VERSION */

#include <string.h>

/** void main_menue(void);
 * 
 * This functions runs the main menu. There, you can select
 * between different playmodes, exit and so on.
 *
 * Args: None
 * Returns: Nothing
 */
void main_menue(void) {
		  bool* doexit = new bool(false);
		  bool* dosingle_player = new bool(false);
		  bool* domulti_player = new bool(false);
		  bool* dooptions = new bool(false);
		  bool* doreadme = new bool(false);
		  bool* doabout = new bool(false);
		  
		  // Set to MENU_XRESxMENU_YRES so that we know on what we are and the pictures
		  // look good
		  //uint lx=g_gr.get_xres();
		  //uint ly=g_gr.get_yres();
		  g_gr.set_mode(MENU_XRES, MENU_YRES, g_gr.get_mode());
		  g_ip.set_max_cords(MENU_YRES-g_cur.get_w(), MENU_YRES-g_cur.get_h());

		  // make the background window, fill it with the splash screen
		  Window* win=g_ui.create_window(0, 0, g_gr.get_xres(), g_gr.get_yres(), Window::FLAT);
		  Pic* p = new Pic;
		  const char* str=g_fileloc.locate_file("splash.bmp", TYPE_PIC);
		  assert(str);
		  p->load(str);
		  assert(p);
		  
		  win->set_new_bg(p);		 
		  // Create the different areas
		  char buf[100];
		  strcpy(buf, "Version ");
		  strcat(buf, VERSION);
		  win->create_textarea(MENU_XRES, MENU_YRES, buf, Textarea::RIGHTA);
		  win->create_textarea(0, 480, "(C) 2002 by the Widelands Development Team"); 

		  // Create the buttons
		  Button* b;
		  b=win->create_button(60, 150, 170, 20, 1);
		  b->register_func(menue_butclick_func, dosingle_player);
		  b->set_pic(g_fh.get_string("Single Player", 0));
		  
		  b=win->create_button(60, 190, 170, 20, 1);
		  b->register_func(menue_butclick_func, domulti_player);
		  b->set_pic(g_fh.get_string("Multi Player", 0));
		  
		  b=win->create_button(60, 230, 170, 20, 1);
		  b->register_func(menue_butclick_func, dooptions);
		  b->set_pic(g_fh.get_string("Options", 0));
		 
		  b=win->create_button(60, 270, 170, 20, 1);
		  b->register_func(menue_butclick_func, doreadme);
		  b->set_pic(g_fh.get_string("View Readme", 0));
		  
		  b=win->create_button(60, 310, 170, 20, 1);
		  b->register_func(menue_butclick_func, doabout);
		  b->set_pic(g_fh.get_string("About", 0));
		  

		  
		  b=win->create_button(60, 370, 170, 20, 0);
		  b->register_func(menue_butclick_func, doexit);
		  b->set_pic(g_fh.get_string("Exit Game", 0));
		  
		  // Register the resposible mouse funtions
		  g_ip.register_mcf(menue_lclick, Input::BUT1);
		  g_ip.register_mcf(menue_rclick, Input::BUT2);
	     g_ip.register_mmf(menue_mmf);
		
		  while(!g_ip.should_die() && !*doexit && !*dosingle_player &&  
								!*domulti_player && !*dooptions &&  !*doreadme && !*doabout) {
					 menue_loop();
		  }
		  

		  
		  if(*doexit) {
					 g_ui.delete_window(win);
					 return;
		  }
		  else if(*dosingle_player) { 
					 g_ui.delete_window(win);
					 single_player_menue();
		  }
		  else if(*domulti_player) {
					 g_ui.delete_window(win);
					 // Todo
		  }
		  else if(*dooptions) {
					 g_ui.delete_window(win);
					 // Todo
		  }
		  else if(*doreadme) {
					 g_ui.delete_window(win);
					 // Todo
		  }
		  else if(*doabout) {
					 g_ui.delete_window(win);
					 // Todo
		  }

		  // everything done.
		  // we return, this will end the game
		  return;
}
