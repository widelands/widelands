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

#ifndef included_ui_box_h
#define included_ui_box_h

#include <vector>
#include "ui_panel.h"

/**
A layouting panel that holds a number of child panels.
The UIPanels you add to the UIBox must be children of the UIBox.
The UIBox automatically resizes itself and positions the added children.
*/
class UIBox : public UIPanel {
public:
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
	UIBox(UIPanel* parent, int x, int y, uint orientation);

	void resize();

	int get_nritems() const { return m_items.size(); }

	void add(UIPanel* panel, uint align);
	void add_space(uint space);

private:
	void get_item_size(uint idx, int* depth, int* breadth);
	void set_item_pos(uint idx, int pos);

private:
	struct Item {
		enum Type {
			ItemPanel,
			ItemSpace
		};

		Type		type;

		union {
			struct {
				UIPanel*	panel;
				uint		align;
			} panel;
			uint		space;
		} u;
	};

	uint	m_orientation;

	std::vector<Item>	m_items;
};


#endif // included_ui_box_h
