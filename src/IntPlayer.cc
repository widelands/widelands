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

#include "ui.h"
#include "input.h"
#include "graphic.h"
#include "IntPlayer.h"
#include "cursor.h"


/** class Interactive_Player
 *
 * This is the interactive player. this one is 
 * responsible to show the correct map
 * to the player and draws the user interface, 
 * cares for input and so on.
 *
 * Depends: g_ip, g_ui, g_gr, g_cur
 */

/** Interactive_Player::Interactive_Player(Map* m)
 *
 * Init
 *
 * Args: m	map to use for the mapview
 */
Interactive_Player::Interactive_Player(Map* m) {
		  grab_input();
		  
		  bshould_exit=false;
		  bshould_endgame=false;

		  mv= new Map_View(m);

		  // Creating the user interface (lots, and lots to do)
		  mwin=g_ui.create_window((g_gr.get_xres()>>1)-(MAIN_WINDOW_WIDTH>>1), g_gr.get_yres()-MAIN_WINDOW_HEIGHT, MAIN_WINDOW_WIDTH,
								MAIN_WINDOW_HEIGHT, Window::FLAT);
		  Button* b=mwin->create_button(0,0,30,30,2);
		  b->set_pic(g_fh.get_string("EXIT", 0));
		  b->register_func(exit_click, this);
		  
		  b=mwin->create_button(34,0,30,30,2);
		  b->set_pic(g_fh.get_string("WIN", 0));
		  b->register_func(window_click, this);

		  mwin->create_button(68,0,30,30,2);
		  mwin->create_button(102,0,30,30,2);
}

/** Interactive_Player::~Interactive_Player(void) 
 *
 * cleanups
 */
Interactive_Player::~Interactive_Player(void)  {
		  g_ui.delete_all_windows();
		  delete mv;
}

/** void Interactive_Player::draw_interact(void) 
 *
 * Draws the screen for the interactive user
 * and cares for events
 *
 * Args: none
 * Returns: Nothing
 */
void Interactive_Player::interact(void) {

		  g_ip.handle_pending_input(); 
		  if(g_gr.does_need_update()) {
					 mv->draw();
					 g_ui.draw();
					 g_cur.draw(g_ip.get_mpx(), g_ip.get_mpy());
					 g_gr.update();
					 //g_gr.update_quarter();
		  }

}
		  
/** void Interactive_Player::grab_input(void) 
 *
 * Grabs the current input handler for the Interactive user
 *
 * Args: none
 * Returns: Nothing
 */
void Interactive_Player::grab_input(void) {
		  g_ip.register_mmf(IP_mmf, this);
		  g_ip.register_mcf(IP_lclick, Input::BUT1, this);
		  g_ip.register_mcf(IP_rclick, Input::BUT2, this);
}

/** int IP_lclick(const bool b, const uint x, const uint y, void* )
 *
 *  This functions is responsible for the user input in a running game
 *
 * Args: b  button pressed or not
 *       x, y  position of click
 *       a 	pointer to interactive player class
 * Returns: INPUT_HANDLED
 */
int IP_lclick(const bool b, const uint x, const uint y, void* a) {
		  if(g_ui.handle_click(1, b, x, y, NULL) == INPUT_HANDLED) return INPUT_HANDLED;
		  
		  // Interactive_Player* pl= (Interactive_Player*) a;
		  
		  return INPUT_HANDLED;
}

/** int int IP_rclick(const bool b, const uint x, const uint y, void* )
 *  
 *  This functions is responsible for the user input in a running game
 *  
 * Args: b  button pressed or not
 *       x, y  position of click
 *       a 	pointer to interactive player class
 * Returns: INPUT_HANDLED
 */
int IP_rclick(const bool b, const uint x, const uint y, void*) {
		  if(g_ui.handle_click(2, b, x, y, NULL) == INPUT_HANDLED) return INPUT_HANDLED;
		  return INPUT_HANDLED;
}

/** int IP_mmf(const uint x, const uint y, const int xdiff, const int ydiff, const bool b1, const bool b2, void*) 
 *  
 *  This functions is responsible for the user input in a running game
 *
 * Args: x, y  position of the cursor on the screen
 *       xdiff, ydiff   differences since last call
 *       b1, b2   state of the two mouse buttons
 *       a 	pointer to interactive player class
 * Returns: INPUT_HANDLED
 */
int IP_mmf(const uint x, const uint y, const int xdiff, const int ydiff, const bool b1, const bool b2, void* a ) {
		  static bool drag=false;
		  int ret=INPUT_UNHANDLED;

		  if(!b2) drag=false;


		  if(drag==false) ret=g_ui.handle_mm(x, y, xdiff, ydiff, b1, b2, NULL) ;

		  if(ret == INPUT_UNHANDLED && b2) {
					 Interactive_Player* pl= (Interactive_Player*) a;
					 
					 pl->mv->set_rel_viewpoint(xdiff, ydiff);
					 g_gr.needs_fs_update(); // map scroll, invalidates all
					 g_ip.set_mouse_pos(g_ip.get_mplx(), g_ip.get_mply());
					 drag=1;
		  } else {
					 g_gr.register_update_rect(x, y, g_cur.get_w(), g_cur.get_h());
					 g_gr.register_update_rect(g_ip.get_mplx(), g_ip.get_mply(), g_cur.get_w(), g_cur.get_h());
		  }

		  return INPUT_HANDLED;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BUTTON FUNCS down here. Will be a hole bunch of them, i guess
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Main Window: Second leftest button is clicked.
void window_click(Window* par, void* a) {
	//	  Interactive_Player* pl= (Interactive_Player*) a;
		  
		  g_ui.create_window(40, 40, 100, 100);
};
// Main Window: Leftest button is clicked.
void exit_click(Window* par, void* a) {
		  Interactive_Player* pl= (Interactive_Player*) a;
		  
		  pl->bshould_endgame=true;
};
