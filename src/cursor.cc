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

#include "cursor.h"


/** class Cursor
 *
 * This class represents the cursor AS AN IMAGE
 * so, this knows how to display a cursor on the graphics
 * device. But it can't get the current cordinates
 *
 * it's a singleton
 *
 * DEPENDS: Graph::draw_bob function
 * DEPENDS: Graph::Pic class
 */


/** Cursor::Cursor(void)
 *
 * default constructor. Simple inits
 *
 * Args: none
 * Returns: nothing
 */
Cursor::Cursor(void) {
		  show_c=1;
		  pic=NULL;
};

/** Cursor::~Cursor(void)
 *
 * Cleanup
 *
 * Args: None
 * Returns: Nothing
 */
Cursor::~Cursor(void) {
		  if(pic) {
					 delete pic;
		  }
}

/** void Cursor::show_cursor(const bool b)
 *
 * This toggles, if the cursor should be visible or not
 *
 * Args;	b 	true if it should be visible, false otherwise
 * Returns: Nothing
 */
void Cursor::show_cursor(const bool b) {
		  show_c=b;
}

/** void Cursor::set_pic(Pic* p)
 *
 * set a new cursor picture.
 *
 * Args; p 	new picture
 * Returns: Nothing
 */
void Cursor::set_pic(Pic* p) {
		  assert(p);

		  if(pic==p) return;


		  if(pic) delete pic;
		  pic=p;
}

/** void Cursor::draw(const uint x, const uint y)
 *
 * Draws the current picture at the given location on the screen
 *
 * Args:	x	x pos of cursor
 * 		y	y pos of cursor
 * Returns: Nothing
 */
void Cursor::draw(const uint x, const uint y) {
		  if(!show_c) return;
		  Graph::copy_pic(g_gr.get_screenbmp(), pic, x, y, 0,0, pic->get_w(), pic->get_h());
}

