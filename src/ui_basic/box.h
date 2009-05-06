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

#ifndef UI_BOX_H
#define UI_BOX_H

#include "panel.h"

#include <vector>

namespace UI {

struct Scrollbar;

/**
 * A layouting panel that holds a number of child panels.
 * The Panels you add to the Box must be children of the Box.
 * The Box automatically resizes itself and positions the added children.
*/
struct Box : public Panel {
	enum {
		Horizontal = 0,
		Vertical = 1,

		AlignLeft = 0,
		AlignTop = 0,
		AlignCenter = 1,
		AlignRight = 2,
		AlignBottom = 2,
	};
public:
	Box
		(Panel * parent,
		 int32_t x, int32_t y,
		 uint32_t orientation,
		 int32_t max_x = 0, int32_t max_y = 0);

	void set_scrolling(bool scroll);
	void resize();

	int32_t get_nritems() const {return m_items.size();}

	void add(Panel * panel, uint32_t align);
	void add_space(uint32_t space);
	bool is_snap_target() const {return true;}

private:
	void get_item_size(uint32_t idx, uint32_t & depth, uint32_t & breadth);
	void set_item_pos(uint32_t idx, int32_t pos);
	void update_positions();
	void scrollbar_moved(int32_t);

	//don't resize beyond this size
	uint32_t m_max_x, m_max_y;

private:
	struct Item {
		enum Type {
			ItemPanel,
			ItemSpace
		};

		Type type;

		union {
			struct {
				Panel * panel;
				uint32_t    align;
			} panel;
			uint32_t space;
		} u;
	};

	bool m_scrolling;
	Scrollbar * m_scrollbar;
	uint32_t m_orientation;

	std::vector<Item> m_items;
};
};

#endif
