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
#include "cardselectmenue.h"
#include "singlepmenue.h"
#include "graphic.h"
#include "input.h"
#include "ui.h"
#include "fileloc.h"
#include "cursor.h"
#include "counter.h"
#include "font.h"
#include "mainmenue.h"
#include "menuecommon.h"
#include "game.h"
#include "criterr.h"

/** void single_player_menue(void) 
 *
 * This takes care for the single player menue, chosing campaign or
 * single player card and so on, so on.
 *
 * Args:	None
 * Returns:	nothing
 */
void single_player_menue(void) {
		  bool* brunmm = new bool(false);
		  bool* brunc = new bool(false);
		  bool* brunsc = new bool(false);
		  bool* bloadg = new bool(false);
		  
		  // We do not need to care for the resolution, since this menue just gets called by other menues
		  // So we know for sure, we've got 640x480 resolution
		  
		  // make the background window, fill it with the splash screen
		  Window* win=g_ui.create_window(0, 0, g_gr.get_xres(), g_gr.get_yres(), Window::FLAT);
		  Pic* p = new Pic;
		  const char* str=g_fileloc.locate_file("splash.bmp", TYPE_PIC);
		  if(!str) {
					 critical_error("splash.bmp:  File not found. Check your installation.");
		  }
		  p->load(str);

		  win->set_new_bg(p);		 
		  // Create the different areas
		  win->create_textarea(0, 150, 640, Textarea::CENTER)->set_text("Single Player Menu");

		  // Create the buttons
		  Button* b;
		  b=win->create_button(60, 170, 170, 20, 1);
		  b->register_func(menue_butclick_func, brunsc);
		  b->set_pic(g_fh.get_string("Single Card", 0));

		  b=win->create_button(60, 210, 170, 20, 1);
		  b->register_func(menue_butclick_func, brunc);
		  b->set_pic(g_fh.get_string("Campaign", 0));

		  b=win->create_button(60, 250, 170, 20, 1);
		  b->register_func(menue_butclick_func, bloadg);
		  b->set_pic(g_fh.get_string("Load Game", 0));

		  b=win->create_button(60, 370, 170, 20, 0);
		  b->register_func(menue_butclick_func, brunmm);
		  b->set_pic(g_fh.get_string("Back", 0));

		  // Register the resposible mouse funtions
		  g_ip.register_mcf(menue_lclick, Input::BUT1);
		  g_ip.register_mcf(menue_rclick, Input::BUT2);
	     g_ip.register_mmf(menue_mmf);

		  while(!g_ip.should_die() && !*brunmm && !*brunsc && !*bloadg && !*brunc) {
					 menue_loop();
		  }		

		  if(*brunmm) {
					 g_ui.delete_window(win);
					 main_menue();
		  }
		  else if(*brunsc) {
					 g_ui.delete_window(win);
					 card_select_menue();
		  }
		  else if(*brunc) {
					 g_ui.delete_window(win);
					 // TODO
					 critical_error("This is not yet supported. You can savly click on continue.");
					 single_player_menue();
		  }
		  else if(*bloadg) {
					 g_ui.delete_window(win);
					 // TODO
					 critical_error("This is not yet supported. You can savly click on continue.");
					 single_player_menue();
		  }
		  
		  
		  return;
}
