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

#ifndef __S__CURSOR_H
#define __S__CURSOR_H

#include "singleton.h"
#include "graphic.h"

/** class Cursor
 *
 * This class represents the cursor AS AN IMAGE
 * so, this knows how to display a cursor on the graphics
 * device. But it can't get the current cordinates
 *
 * it's a singleton
 * 
 * DEPENDS: Graph::Pic class
 * DEPENDS: Graph::draw_bob function
 */
class Cursor : public Singleton<Cursor> {
		  Cursor(const Cursor&);
		  Cursor& operator=(const Cursor&);

		  public:
					 Cursor(void);
					 ~Cursor(void);

					 void show_cursor(const bool);
					 void set_pic(Pic* );
					 void draw(const unsigned int, const unsigned int);

		  private:
					 bool show_c;
					 Pic* pic;
};

#define g_cur	Cursor::get_singleton()

#endif /* __S__CURSOR_H */
