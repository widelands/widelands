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
#include "cursor.h"

/** void menue_butclick_func(Window*, void* b) 
 *
 * This function is used to register button clicks in a menue.
 * it expects a pt to bool in b and sets it true.
 *
 * Args:	b	pointer to bool
 * Returns: nothing
 */
void menue_butclick_func(Window*, void* b) { *((bool*) b)=true; }


/** int int menue_lclick(const bool b, const uint x, const uint y, void* )
 *
 * This function is the generic left click handler for a menue
 *
 * Args:	b	button pressed or not
 * 		x, y	position of click
 * Returns: INPUT_HANDLED
 */
int menue_lclick(const bool b, const uint x, const uint y, void* ) {
		  if(g_ui.handle_click(1, b, x, y, NULL) == INPUT_HANDLED) return INPUT_HANDLED; 
		  return INPUT_HANDLED;
}

/** int int menue_rclick(const bool b, const uint x, const uint y, void* )
 *
 * This function is the generic right click handler for a menue
 *
 * Args:	b	button pressed or not
 * 		x, y	position of click
 * Returns: INPUT_HANDLED
 */
int menue_rclick(const bool b, const uint x, const uint y, void*) {
		  if(g_ui.handle_click(2, b, x, y, NULL) == INPUT_HANDLED) return INPUT_HANDLED; 
		  return INPUT_HANDLED;
}

/** int menue_mmf(const uint x, const uint y, const int xdiff, const int ydiff, const bool b1, const bool b2, void*) 
 * 
 * This is the generic mouse movement function for a menue.
 *
 * Args:	x, y	position of the cursor on the screen
 * 		xdiff, ydiff	differences since last call
 * 		b1, b2	state of the two mouse buttons
 * Returns:	INPUT_HANDLED
 */
int menue_mmf(const uint x, const uint y, const int xdiff, const int ydiff, const bool b1, const bool b2, 
					 void* ) {
		  g_gr.register_update_rect(x, y, g_cur.get_w(), g_cur.get_h());
		  g_gr.register_update_rect(g_ip.get_mplx(), g_ip.get_mply(), g_cur.get_w(), g_cur.get_h());

		  g_ui.handle_mm(x, y, xdiff, ydiff, b1, b2, NULL); 

		  return INPUT_HANDLED;
}

/** void menue_loop(void) 
 *
 * This is the generic menue loop. It redraws and updates the screen and 
 * cares for pending input
 *
 * Args: None
 * Returns: Nothing
 */
void menue_loop(void) {
		  g_ip.handle_pending_input(); 
		  if(g_gr.does_need_update()) {
					 g_ui.draw();
					 g_cur.draw(g_ip.get_mpx(), g_ip.get_mpy());
					 g_gr.update();
					 //g_gr.update_quarter();
		  }

}

