/*
 * Copyright (C) 2003, 2006-2008 by the Widelands Development Team
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

#include "ui_icongrid.h"

#include "graphic.h"
#include "rendertarget.h"
#include "font_handler.h"
#include "constants.h"

namespace UI {
/**
 * Initialize the grid
*/
Icon_Grid::Icon_Grid(Panel* parent, int32_t x, int32_t y, int32_t cellw, int32_t cellh, uint32_t flags, int32_t cols)
	: Panel(parent, x, y, 0, 0)
{
	m_flags = flags;
	m_columns = cols;

	m_cell_width = cellw;
	m_cell_height = cellh;

	m_highlight = -1;
	m_clicked = -1;
	m_selected = -1;
	m_font_height = 0;

	m_selectbox_color = RGBColor(255, 255, 0);
}


/**
 * Add a new icon to the list and resize appropriately.
 * Returns the index of the newly added icon.
*/
int32_t Icon_Grid::add(uint32_t picid, void* data, std::string descr)
{
	Item it;

	it.picid = picid;
	it.data = data;
	it.descr = descr;

	m_items.push_back(it);

	if (it.descr.size() && !m_font_height)
		m_font_height = g_fh->get_fontheight(UI_FONT_SMALL) + 2;


	// resize
	if (get_orientation() == Grid_Horizontal)
	{
		int32_t rows = (m_items.size() + m_columns - 1) / m_columns;

		if (rows <= 1)
			set_size(m_cell_width * m_items.size(), m_cell_height + m_font_height);
		else
			set_size(m_cell_width * m_columns, m_cell_height * rows + m_font_height);
	}
	else
	{
		int32_t cols = (m_items.size() + m_columns - 1) / m_columns;

		if (cols <= 1)
			set_size(m_cell_width, m_cell_height * m_items.size() + m_font_height);
		else
			set_size(m_cell_width * cols, m_cell_height * m_columns + m_font_height);
	}

	return m_items.size() - 1;
}


/**
 * Returns the user-defined data of the icon with the given index.
*/
void* Icon_Grid::get_data(int32_t idx)
{
	assert(static_cast<uint32_t>(idx) < m_items.size());

	return m_items[idx].data;
}


/**
 * Set the currently selected icon for persistant grids.
*/
void Icon_Grid::set_selection(int32_t idx)
{
	assert(is_persistant());
	assert(static_cast<uint32_t>(idx) < m_items.size());

	if (m_selected >= 0)
		update_for_index(m_selected);
	update_for_index(idx);

	m_selected = idx;
}


/**
 * Change the color of the selection box (default is yellow).
*/
void Icon_Grid::set_selectbox_color(RGBColor clr)
{
	m_selectbox_color = clr;
}


/**
 * Draw the building symbols
*/
void Icon_Grid::draw(RenderTarget* dst)
{
	int32_t x, y;
	bool highlight = false;

	// First of all, draw the highlight
	if (m_highlight >= 0 && (m_clicked < 0 || m_clicked == m_highlight)) {
		get_cell_position(m_highlight, &x, &y);
		dst->brighten_rect
			(Rect(Point(x, y), m_cell_width, m_cell_height),
			 MOUSE_OVER_BRIGHT_FACTOR);
		highlight = true;
	}

	// Draw the symbols
	x = 0;
	y = 0;

	for (uint32_t idx = 0; idx < m_items.size(); ++idx) {
		const uint32_t picid = m_items[idx].picid;
		uint32_t w, h;
		g_gr->get_picture_size(picid, w, h);

		dst->blit
			(Point(x + (m_cell_width - w) / 2, y + (m_cell_height - h) / 2),
			 picid);

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
			dst->draw_rect
				(Rect(Point(x, y), m_cell_width, m_cell_height), m_selectbox_color);
		}
	}

	if (highlight) {
		int32_t w, h;
		g_fh->get_size
			(UI_FONT_SMALL,  m_items[m_highlight].descr.c_str(), &w, &h);
		if (w > get_inner_w())
			set_inner_size(w, get_inner_h());
		g_fh->draw_string
			(*dst,
			 UI_FONT_SMALL, UI_FONT_SMALL_CLR,
			 Point(1, get_h() - m_font_height + 10),
			 m_items[m_highlight].descr.c_str());
	}
}

/**
 * Return the item index for a given point inside the Icon_Grid.
 * Returns -1 if no item is below the point.
*/
int32_t Icon_Grid::index_for_point(int32_t x, int32_t y)
{
	int32_t w = m_cell_width;
	int32_t h = m_cell_height;

	if (get_orientation() != Grid_Horizontal) {
		std::swap(x, y);
		std::swap(w, h);
	}

	if (x < 0 || x >= m_columns * w || y < 0)
		return -1;

	const int32_t index = m_columns * (y / h) + (x / w);

	if (static_cast<size_t>(index) >= m_items.size()) return -1;

	return index;
}


/**
 * Calculate the upper left corner of the cell with the given index.
*/
void Icon_Grid::get_cell_position(int32_t idx, int32_t* px, int32_t* py)
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
 * Issue an update() call for the cell with the given idx.
*/
void Icon_Grid::update_for_index(int32_t idx)
{
	if (static_cast<size_t>(idx) < m_items.size()) {
		int32_t x, y;

		get_cell_position(idx, &x, &y);
		update(x, y, m_cell_width, m_cell_height);
	}
}


/**
 * Turn highlight off when the mouse leaves the grid
*/
void Icon_Grid::handle_mousein(bool inside)
{
	if (!inside) {
		if (m_highlight != -1) {
			update_for_index(m_highlight);
			mouseout.call(m_highlight);
		}

		m_highlight = -1;
	}
}


/**
 * Update highlight under the mouse and send signals.
*/
bool Icon_Grid::handle_mousemove(const Uint8, int32_t x, int32_t y, int32_t, int32_t) {
	int32_t hl = index_for_point(x, y);

	if (hl != m_highlight) {
		update_for_index(m_highlight);
		if (m_highlight != -1) mouseout.call(m_highlight);
		update_for_index(hl);
		if (hl != -1) mousein.call(hl);

		m_highlight = hl;
	}
	return true;
}


/**
 * Left mouse down selects the building, left mouse up acknowledges and sends
 * the signal.
*/
bool Icon_Grid::handle_mousepress(const Uint8 btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT) return false;

	const int32_t hl = index_for_point(x, y);

	if (hl >= 0) {
		grab_mouse(true);
		m_clicked = hl;
	}

	return true;
}
bool Icon_Grid::handle_mouserelease(const Uint8 btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT) return false;

	const int32_t hl = index_for_point(x, y);

	if (m_clicked >= 0) {
		grab_mouse(false);
		if (hl == m_clicked) {
			if (is_persistant())
				set_selection(hl);
			clicked.call(hl);
			play_click();
		}
		m_clicked = -1;
	}

	return true;
}
};
