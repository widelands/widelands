/*
 * Copyright (C) 2003, 2006-2007 by the Widelands Development Team
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

#ifndef included_ui_icongrid_h
#define included_ui_icongrid_h

#include <stdint.h>
#include <string>
#include <vector>

#include "rgbcolor.h"
#include "ui_panel.h"
#include "ui_signal.h"

namespace UI {
/**
Arranges clickable pictures of common size in a regular grid.

Arrangement can be horizontal (pictures fill the grid from left to right, top
to bottom) or vertical (pictures fill the grid from top to bottom, then left to
right).

An Icon_Grid can be persistant, which means that one of the pictures can be
the currently selected picture. This picture is highlighted all the time.
*/
struct Icon_Grid : public Panel {
	enum {
		Grid_Horizontal = 0,
		Grid_Vertical = 1,
		Grid_Orientation_Mask = 1,

		Grid_Persistant = 2,
	};

	Icon_Grid(Panel* parent, int x, int y, int cellw, int cellh, uint32_t flags, int cols);

	Signal1<int> clicked;
	Signal1<int> mouseout;
	Signal1<int> mousein;

	bool is_persistant() const {return m_flags & Grid_Persistant;}
	uint32_t get_orientation() const {return m_flags & Grid_Orientation_Mask;}

	int add(uint32_t picid, void* data, std::string descr = std::string());
	void* get_data(int idx);

	void set_selection(int idx);
	int get_selection() const {return m_selected;}

	void set_selectbox_color(RGBColor clr);

protected:
	void draw(RenderTarget* dst);

	int index_for_point(int x, int y);
	void get_cell_position(int idx, int* px, int* py);
	void update_for_index(int idx);

	void handle_mousein(bool inside);
	bool handle_mousemove(const Uint8 state, int x, int y, int xdiff, int ydiff);
	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);

private:
	struct Item {
		uint32_t        picid;
		void      * data;
		std::string descr;
	};

	uint32_t   m_flags;

	 ///< max # of columns (or rows, depending on orientation) in the grid
	int m_columns;

	///< currently highlight (mouseover) icon idx (-1 = no highlight)
	int m_highlight;

	int m_clicked; ///< icon that was clicked (only while LMB is down)

	///< currently selected (persistant) icon idx (-1 = no selection)
	int m_selected;

	int               m_cell_width; ///< size of one cell
	int               m_cell_height;
	int               m_font_height;

	std::vector<Item> m_items;

	RGBColor          m_selectbox_color;
};
};

#endif // included_ui_icongrid_h
