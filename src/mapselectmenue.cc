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

#include "os.h"
#include "mapselectmenue.h"
#include "ui.h"
#include "font.h"
#include "fileloc.h"
#include "input.h"
#include "menuecommon.h"
#include "map.h"
#include "singlepmenue.h"
#include "game.h"
#include "criterr.h"
#include "mainmenue.h"

/** void map_select_menue(void) 
 *
 *	Here, you chose a map out of a given listbox
 *
 * Args:	None
 * Returns:	nothing
 */
void map_select_menue(void) {
		  bool* bexit= new bool(false);
		  bool* bselect= new bool(false);
		  
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
		  win->create_textarea(0, 150, 640, Textarea::CENTER)->set_text("Choose your map!");

		  // Create the buttons
		  Button* b;
		  b=win->create_button(400, 400, 170, 20, 0);
		  b->register_func(menue_butclick_func, bexit);
		  b->set_pic(g_fh.get_string("Back", 0));
		  b=win->create_button(400, 430, 170, 20, 2);
		  b->register_func(menue_butclick_func, bselect);
		  b->set_pic(g_fh.get_string("OK", 0));
		  
		  // Create the list area
		  Listselect* sel= win->create_listselect(20, 170, 360, 300);
		  // Fill it with the files: Widelands map files
		  g_fileloc.init_filelisting(TYPE_MAP, WLMF_SUFFIX);
		  const char* name;
		  uint i;
		  while(g_fileloc.get_state() != File_Locator::LA_NOMOREFILES) {
					 name=g_fileloc.get_next_file();
					 if(!name) continue;

					 i=strlen(name);
					 while(name[i]!= CSEP) --i;
					 ++i;
					 sel->add_entry(name+i, name);
		  }
		  g_fileloc.end_filelisting();


		  // Fill it with more files: Settlers2 map files
		  g_fileloc.init_filelisting(TYPE_MAP, S2MF_SUFFIX);
		  while(g_fileloc.get_state() != File_Locator::LA_NOMOREFILES) {
					 name=g_fileloc.get_next_file();
					 if(!name) continue;

					 i=strlen(name);
					 while(name[i]!= CSEP) --i;
					 ++i;
					 sel->add_entry(name+i, name);
		  }
		  g_fileloc.end_filelisting();

		  // Add info fields
		  win->create_textarea(380, 210, "Name:",  Textarea::CENTER);
		  Textarea* taname= win->create_textarea(460, 210, 640,  Textarea::LEFTA);
		  win->create_textarea(380, 230, "Author:",  Textarea::CENTER);
		  Textarea* taauthor= win->create_textarea(460, 230, 640,  Textarea::LEFTA);
		  win->create_textarea(380, 250, "Size:", Textarea::CENTER);
		  Textarea* tasize=win->create_textarea(460, 250, 640, Textarea::LEFTA);
		  win->create_textarea(380, 270, "World:", Textarea::CENTER);
		  Textarea* taworld=win->create_textarea(460, 270, 640, Textarea::LEFTA);
		  win->create_textarea(380, 290, "Players:", Textarea::CENTER);
		  Textarea* tanplayers=win->create_textarea(460, 290, 640, Textarea::LEFTA);
		  win->create_textarea(380, 310, "Descr:", Textarea::CENTER);
		  Multiline_Textarea* tadescr=win->create_multiline_textarea(460, 310, 180, 90, true, Multiline_Textarea::LEFTA);

		  // Register the resposible mouse funtions
		  g_ip.register_mcf(menue_lclick, Input::BUT1);
		  g_ip.register_mcf(menue_rclick, Input::BUT2);
	     g_ip.register_mmf(menue_mmf);

		  Map m;
		  char buf[10]; // 1024x1024\0
		  while(!g_ip.should_die() && !*bexit) {
					 if(sel->new_selection()) {
								if(sel->get_selection())
								{
										  if(m.load_map(sel->get_selection()) == RET_OK) {	
													 taname->set_text(m.get_name());
													 taauthor->set_text(m.get_author());
													 sprintf(buf, "%-4ix%4i", m.get_w(), m.get_h());
													 tasize->set_text(buf);
													 taworld->set_text(m.get_world());
													 sprintf(buf, "%i", m.get_nplayers());
													 tanplayers->set_text(buf);
													 tadescr->set_text(m.get_descr());
													 g_gr.register_update_rect(460, 210, 180, 190);
										  }
								}
					 }
					 if(*bselect) {
								if(sel->get_selection()) break;
								else *bselect=false;
					 }
					 menue_loop();
		  }		


		  if(*bexit) {
					 g_ui.delete_all_windows();
					 single_player_menue();
					 return;
		  }

		  if(*bselect) {
					 // a selection has been done
					 char* map = new char[strlen(sel->get_selection())+1];
					 strcpy(map, sel->get_selection());
					 g_ui.delete_all_windows();
					 Game *g=new Game;
					 g->run(map, 7);
					 delete map;
					 delete g;
					 main_menue();
					 return;
		  } 

		  // Never here
		  assert(0);
		  return;
}
