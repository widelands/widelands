/*
 * Copyright (C) 2003, 2006 by the Widelands Development Team
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

#ifndef included_ui_box_h
#define included_ui_box_h

#include <vector>
#include <stdint.h>
#include "ui_panel.h"

namespace UI {
/**
A layouting panel that holds a number of child panels.
The Panels you add to the Box must be children of the Box.
The Box automatically resizes itself and positions the added children.
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
	Box(Panel* parent, int32_t x, int32_t y, uint32_t orientation);

	void resize();

	int32_t get_nritems() const {return m_items.size();}

	void add(Panel* panel, uint32_t align);
	void add_space(uint32_t space);

private:
	void get_item_size(uint32_t idx, int32_t* depth, int32_t* breadth);
	void set_item_pos(uint32_t idx, int32_t pos);

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

	uint32_t              m_orientation;

	std::vector<Item> m_items;
};
};

#endif // included_ui_box_h
