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

#include "log.h"

#include "button.h"
#include "mouse_constants.h"

#include "font_handler.h"
#include "graphic/rendertarget.h"
#include "constants.h"

#include "icongrid.h"

namespace UI {

struct IconGridButton : public Callback_IDButton<Icon_Grid, uint32_t> {
	IconGridButton
		(Icon_Grid * const parent,
		 std::string const & name,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const PictureID background_pictute_id,
		 const PictureID foreground_picture_id,
		 void (Icon_Grid::*callback_function)(uint32_t),
	    Icon_Grid & callback_argument_this,
		 const uint32_t callback_argument_id,
		 Textarea * ta, std::string descr ) :
			Callback_IDButton<Icon_Grid, uint32_t>(
					parent, name, x, y, w, h, background_pictute_id,
					foreground_picture_id, callback_function,
					callback_argument_this, callback_argument_id, "", true, true),
			m_icongrid(parent), m_ta(ta), m_descr(descr) {}

private:
	Icon_Grid * m_icongrid;
	Textarea * m_ta;
	std::string m_descr;

	void handle_mousein(bool inside) {
		if (inside) {
			m_icongrid->mousein.call(_callback_argument_id);
			m_ta->set_text(m_descr);
		} else {
			m_icongrid->mouseout.call(_callback_argument_id);
			m_ta->set_text("");
		}
		Callback_IDButton<Icon_Grid, uint32_t>::handle_mousein(inside);
	}
};

/**
 * Initialize the grid
*/
Icon_Grid::Icon_Grid
	(Panel  * const parent,
	 int32_t const x, int32_t const y, int32_t const cellw, int32_t const cellh,
	 int32_t  const cols)
	:
	Panel            (parent, x, y, 0, 0),
	m_columns        (cols),
	m_cell_width     (cellw),
	m_cell_height    (cellh),
	m_ta             (new Textarea
	(this, 0, 0, 0, g_fh->get_fontheight(UI_FONT_SMALL) + 2))
{}


/**
 * Add a new icon to the list and resize appropriately.
 * Returns the index of the newly added icon.
*/
int32_t Icon_Grid::add
	(std::string const & name, PictureID const picid,
	 void * const data, std::string const & descr)
{
	Item it;

	it.data = data;

	m_items.push_back(it);

	// resize
	int32_t rows = (m_items.size() + m_columns - 1) / m_columns;

	if (rows <= 1) {
		set_desired_size(m_cell_width * m_columns, m_cell_height + m_ta->get_h());
		m_ta->set_size(get_inner_w(), m_ta->get_h());
		m_ta->set_pos(Point(0, m_cell_height));
	} else {
		set_desired_size(m_cell_width * m_columns, m_cell_height * rows + m_ta->get_h());
		m_ta->set_size(get_inner_w(), m_ta->get_h());
		m_ta->set_pos(Point(0, m_cell_height * rows));
	}

	uint32_t idx = m_items.size() - 1;
	uint32_t x = (idx % m_columns) * m_cell_width;
	uint32_t y = (idx / m_columns) * m_cell_height;

	new IconGridButton(this, name,
		x, y, m_cell_width, m_cell_height,
		g_gr->get_no_picture(), picid,
		&Icon_Grid::clicked_button, *this, idx, m_ta, descr);

	return idx;
}

void Icon_Grid::clicked_button(uint32_t idx) {
	clicked.call(idx);
	play_click();
}

/**
 * Returns the user-defined data of the icon with the given index.
*/
void * Icon_Grid::get_data(int32_t const idx)
{
	assert(static_cast<uint32_t>(idx) < m_items.size());

	return m_items[idx].data;
}

}
