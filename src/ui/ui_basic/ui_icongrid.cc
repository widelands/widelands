/*
 * Copyright (C) 2003 by Widelands Development Team
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

#include <cassert>
#include "graphic.h"
#include "rendertarget.h"
#include "types.h"
#include "ui_icongrid.h"
#include "font.h"

/**
Initialize the grid
*/
UIIcon_Grid::UIIcon_Grid(UIPanel* parent, int x, int y, int cellw, int cellh, uint flags, int cols)
	: UIPanel(parent, x, y, 0, 0)
{
	m_flags = flags;
	m_columns = cols;

	m_cell_width = cellw;
	m_cell_height = cellh;

	m_highlight = -1;
	m_clicked = -1;
	m_selected = -1;
	m_font_height = 0;

	m_selectbox_color.set(255, 255, 0);
}


/**
Add a new icon to the list and resize appropriately.
Returns the index of the newly added icon.
*/
int UIIcon_Grid::add(uint picid, void* data, std::string descr)
{
	Item it;

	it.picid = picid;
	it.data = data;
	it.descr = descr;

	m_items.push_back(it);

	if (it.descr.size() && !m_font_height)
		m_font_height = g_font->get_fontheight() + 2;


	// resize
	if (get_orientation() == Grid_Horizontal)
	{
		int rows = (m_items.size() + m_columns - 1) / m_columns;

		if (rows <= 1)
			set_size(m_cell_width * m_items.size(), m_cell_height + m_font_height);
		else
			set_size(m_cell_width * m_columns, m_cell_height * rows + m_font_height);
	}
	else
	{
		int cols = (m_items.size() + m_columns - 1) / m_columns;

		if (cols <= 1)
			set_size(m_cell_width, m_cell_height * m_items.size() + m_font_height);
		else
			set_size(m_cell_width * cols, m_cell_height * m_columns + m_font_height);
	}

	return m_items.size() - 1;
}


/**
Returns the user-defined data of the icon with the given index.
*/
void* UIIcon_Grid::get_data(int idx)
{
	assert((uint)idx < m_items.size());

	return m_items[idx].data;
}


/**
Set the currently selected icon for persistant grids.
*/
void UIIcon_Grid::set_selection(int idx)
{
	assert(is_persistant());
	assert((uint)idx < m_items.size());

	if (m_selected >= 0)
		update_for_index(m_selected);
	update_for_index(idx);

	m_selected = idx;
}


/**
Change the color of the selection box (default is yellow).
*/
void UIIcon_Grid::set_selectbox_color(RGBColor clr)
{
	m_selectbox_color = clr;
}


/**
Draw the building symbols
*/
void UIIcon_Grid::draw(RenderTarget* dst)
{
	int x, y;
	bool highlight = false;

	// First of all, draw the highlight
	if (m_highlight >= 0 && (m_clicked < 0 || m_clicked == m_highlight)) {
		get_cell_position(m_highlight, &x, &y);
		dst->brighten_rect(x, y, m_cell_width, m_cell_height, MOUSE_OVER_BRIGHT_FACTOR);
		highlight = true;
	}

	// Draw the symbols
	x = 0;
	y = 0;

	for(uint idx = 0; idx < m_items.size(); idx++)
	{
		uint picid = m_items[idx].picid;
		int w, h;

		g_gr->get_picture_size(picid, &w, &h);

		dst->blit(x + (m_cell_width-w)/2, y + (m_cell_height-h)/2, picid);

		if (get_orientation() == Grid_Horizontal)
		{
			x += m_cell_width;
			if (!((idx+1) % m_columns)) {
				x = 0;
				y += m_cell_height;
			}
		}
		else
		{
			y += m_cell_height;
			if (!((idx+1) % m_columns)) {
				y = 0;
				x += m_cell_width;
			}
		}
	}

	// Draw selection mark
	if (is_persistant())
	{
		if (m_selected >= 0) {
			get_cell_position(m_selected, &x, &y);
			dst->draw_rect(x, y, m_cell_width, m_cell_height, m_selectbox_color);
		}
	}

	if (highlight)
		g_font->draw_string(dst, 1, get_h() - m_font_height, m_items[m_highlight].descr.c_str());
}

/**
Return the item index for a given point inside the UIIcon_Grid.
Returns -1 if no item is below the point.
*/
int UIIcon_Grid::index_for_point(int x, int y)
{
	int w = m_cell_width;
	int h = m_cell_height;

	if (get_orientation() != Grid_Horizontal) {
		std::swap(x, y);
		std::swap(w, h);
	}

	if (x < 0 || x >= m_columns * w || y < 0)
		return -1;

	int index = m_columns * (y / h) + (x / w);

	if (index < 0 || index >= (int)m_items.size())
		return -1;

	return index;
}


/**
Calculate the upper left corner of the cell with the given index.
*/
void UIIcon_Grid::get_cell_position(int idx, int* px, int* py)
{
	if (get_orientation() == Grid_Horizontal)
	{
		*px = (idx % m_columns) * m_cell_width;
		*py = (idx / m_columns) * m_cell_height;
	}
	else
	{
		*px = (idx / m_columns) * m_cell_width;
		*py = (idx % m_columns) * m_cell_height;
	}
}


/**
Issue an update() call for the cell with the given idx.
*/
void UIIcon_Grid::update_for_index(int idx)
{
	if (idx >= 0 && idx < (int)m_items.size()) {
		int x, y;

		get_cell_position(idx, &x, &y);
		update(x, y, m_cell_width, m_cell_height);
	}
}


/**
Turn highlight off when the mouse leaves the grid
*/
void UIIcon_Grid::handle_mousein(bool inside)
{
	if (!inside) {
		if (m_highlight != -1)
			update_for_index(m_highlight);

		m_highlight = -1;
	}
}


/**
Update highlight under the mouse
*/
void UIIcon_Grid::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
{
	int hl = index_for_point(x, y);

	if (hl != m_highlight) {
		update_for_index(m_highlight);
		update_for_index(hl);

		m_highlight = hl;
	}
}


/**
Left mouse down selects the building, left mouse up acknowledges and sends the
signal.
*/
bool UIIcon_Grid::handle_mouseclick(uint btn, bool down, int x, int y)
{
	int hl;

	if (btn != 0)
		return false;

	hl = index_for_point(x, y);

	if (down)
	{
		if (hl >= 0) {
			grab_mouse(true);
			m_clicked = hl;
		}
	}
	else
	{
		if (m_clicked >= 0) {
			grab_mouse(false);

			if (hl == m_clicked) {
				if (is_persistant())
					set_selection(hl);
				clicked.call(hl);
			}

			m_clicked = -1;
		}
	}

	return true;
}

