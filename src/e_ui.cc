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
#include "e_ui.h"

/*
============================================

In here: all the classes needed only for the editor. 

============================================
*/

/*
===================

A ToolPanel lives at the bottom of the screen.
it can't be moved, nor clicked. it's a window, 
but with lesser functionality
===================
*/

ToolPanel::ToolPanel(Panel *parent, int x, int y, uint w, uint h)
	: Panel(parent, x, y-WINDOW_BORDER, w, h+WINDOW_BORDER)
{
	set_border(0, 0, WINDOW_BORDER, 0);
	set_cache(true);

	m_pic_top = g_gr->get_picture(PicMod_UI, "pics/toolpanel_top.bmp", WINDOW_CLRKEY);
	m_pic_background = g_gr->get_picture(PicMod_UI, "pics/toolpanel_bg.bmp");
}

/** ToolPanel::~ToolPanel()
 *
 * Resource cleanup
 */
ToolPanel::~ToolPanel()
{
}

/*
===============
ToolPanel::draw_border

Redraw the window frame and background
===============
*/
void ToolPanel::draw_border(RenderTarget* dst)
{
	int bgw, bgh;
	int px, py;

	g_gr->get_picture_size(m_pic_background, &bgw, &bgh);
	
	// fill background
   int blitw, blith;
   for(py = CORNER; py < get_h(); py += bgh) {
      for(px = 0; px < get_w(); px += bgw) {
         blitw=(get_w()-px) > bgw ? bgw : (get_w()-px);
         blith=(get_h()-py) > bgh ? bgh : (get_h()-py);
         dst->blitrect(px, py, m_pic_background, 0, 0, blitw, blith);
      }
   }

	// top left corner
	dst->blitrect(0, 0, m_pic_top, 0, 0, CORNER, CORNER);

	// top & bottom bar
	for(px = CORNER; px < get_w()-CORNER-MIDDLE; px += MIDDLE) {
		dst->blitrect(px, 0, m_pic_top, CORNER, 0, MIDDLE, CORNER);
	}
	// odd pixels of top & bottom bar
	dst->blitrect(px, 0, m_pic_top, CORNER, 0, get_w()-px-CORNER, CORNER);

	// top right corner
	dst->blitrect(get_w()-CORNER, 0, m_pic_top, MUST_HAVE_NPIX-CORNER, 0, CORNER, CORNER);
}
