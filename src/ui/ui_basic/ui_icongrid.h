/*
 * Copyright (C) 2003 by the Widelands Development Team
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
// you MUST include ui.h before including this

#ifndef included_ui_icongrid_h
#define included_ui_icongrid_h


/**
Arranges clickable pictures of common size in a regular grid.

Arrangement can be horizontal (pictures fill the grid from left to right, top
to bottom) or vertical (pictures fill the grid from top to bottom, then left to
right).

An IconGrid can be persistant, which means that one of the pictures can be
the currently selected picture. This picture is highlighted all the time.
*/
class IconGrid : public Panel {
public:
	enum {
		Grid_Horizontal = 0,
		Grid_Vertical = 1,
		Grid_Orientation_Mask = 1,

		Grid_Persistant = 2,
	};

public:
	IconGrid(Panel* parent, int x, int y, int cellw, int cellh, uint flags, int cols);

	UISignal1<int> clicked;

	bool is_persistant() const { return m_flags & Grid_Persistant; }
	uint get_orientation() const { return m_flags & Grid_Orientation_Mask; }

	int add(uint picid, void* data);
	void* get_data(int idx);

	void set_selection(int idx);
	int get_selection() const { return m_selected; }

	void set_selectbox_color(RGBColor clr);

protected:
	void draw(RenderTarget* dst);

	int index_for_point(int x, int y);
	void get_cell_position(int idx, int* px, int* py);
	void update_for_index(int idx);

	void handle_mousein(bool inside);
	void handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns);
	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
	struct Item {
		uint		picid;
		void*		data;
	};

	uint	m_flags;
	int	m_columns;		///< max # of columns (or rows, depending on orientation) in the grid
	int	m_highlight;	///< currently highlight (mouseover) icon idx (-1 = no highlight)
	int	m_clicked;		///< icon that was clicked (only while LMB is down)
	int	m_selected;		///< currently selected (persistant) icon idx (-1 = no selection)

	int	m_cell_width;	///< size of one cell
	int	m_cell_height;

	std::vector<Item>	m_items;

	RGBColor		m_selectbox_color;
};

#endif // included_ui_icongrid_h
