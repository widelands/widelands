/*
 * Copyright (C) 2003, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef UI_ICONGRID_H
#define UI_ICONGRID_H

#include "panel.h"
#include "textarea.h"

#include <vector>

#include <boost/signal.hpp>

namespace UI {

/**
 * Arranges clickable pictures of common size in a regular grid.
 *
 * Arrangement is horizontal (pictures fill the grid from left to right, top to
 * bottom).
*/
struct Icon_Grid : public Panel {
	Icon_Grid
		(Panel  * parent,
		 int32_t x, int32_t y, int32_t cellw, int32_t cellh,
		 int32_t  cols);

	boost::signal<void (int32_t)> clicked;
	boost::signal<void (int32_t)> mouseout;
	boost::signal<void (int32_t)> mousein;

	int32_t add
		(std::string const & name,
		 PictureID           picid,
		 void              * data,
		 std::string const & descr = std::string());
	void * get_data(int32_t idx);

private:
	void clicked_button(uint32_t);

	struct Item {
		void      * data;
	};

	/// max # of columns (or rows, depending on orientation) in the grid
	int32_t m_columns;
	int32_t m_cell_width; ///< size of one cell
	int32_t m_cell_height;

	Textarea          m_ta;
	std::vector<Item> m_items;
};

}

#endif
