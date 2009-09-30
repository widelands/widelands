/*
 * Copyright (C) 2003, 2006-2009 by the Widelands Development Team
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

#ifndef UI_ICONGRID_H
#define UI_ICONGRID_H

#include "panel.h"
#include "m_signal.h"

#include "rgbcolor.h"

#include <vector>

namespace UI {

/**
 * Arranges clickable pictures of common size in a regular grid.
 *
 * Arrangement can be horizontal (pictures fill the grid from left to right, top
 * to bottom) or vertical (pictures fill the grid from top to bottom, then left
 * to right).
 *
 * An Icon_Grid can be persistant, which means that one of the pictures can be
 * the currently selected picture. This picture is highlighted all the time.
*/
struct Icon_Grid : public Panel {
	enum {
		Grid_Horizontal = 0,
		Grid_Vertical = 1,
		Grid_Orientation_Mask = 1,

		Grid_Persistant = 2,
	};

	Icon_Grid
		(Panel  * parent,
		 int32_t x, int32_t y, int32_t cellw, int32_t cellh,
		 uint32_t flags,
		 int32_t  cols);

	Signal1<int32_t> clicked;
	Signal1<int32_t> mouseout;
	Signal1<int32_t> mousein;

	bool is_persistant() const {return m_flags & Grid_Persistant;}
	uint32_t get_orientation() const {return m_flags & Grid_Orientation_Mask;}

	int32_t add
		(PictureID           picid,
		 void              * data,
		 std::string const & descr = std::string());
	void * get_data(int32_t idx);

	void set_selection(int32_t idx);
	int32_t get_selection() const {return m_selected;}

	void set_selectbox_color(RGBColor clr);

protected:
	void draw(RenderTarget &);

	int32_t index_for_point(int32_t x, int32_t y);
	void get_cell_position(int32_t idx, uint32_t & px, uint32_t & py);
	void update_for_index(int32_t idx);

	void handle_mousein(bool inside);
	bool handle_mousemove
		(Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	struct Item {
		PictureID   picid;
		void      * data;
		std::string descr;
	};

	uint32_t   m_flags;

	/// max # of columns (or rows, depending on orientation) in the grid
	int32_t m_columns;

	///currently highlight (mouseover) icon idx (-1 = no highlight)
	int32_t m_highlight;

	int32_t m_clicked; ///< icon that was clicked (only while LMB is down)

	///currently selected (persistant) icon idx (-1 = no selection)
	int32_t m_selected;

	int32_t               m_cell_width; ///< size of one cell
	int32_t               m_cell_height;
	int32_t               m_font_height;

	std::vector<Item> m_items;

	RGBColor          m_selectbox_color;
};

}

#endif
