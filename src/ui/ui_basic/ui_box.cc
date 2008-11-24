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

#include "ui_box.h"

#include "interactive_base.h"
#include "wexception.h"
#include <algorithm>

namespace UI {
/**
 * Initialize an empty box
*/
Box::Box
	(Panel * const parent,
	 int32_t const x, int32_t const y,
	 uint32_t const orientation,
	 int32_t const max_x, int32_t const max_y)
	:
	Panel        (parent, x, y, 0, 0),

	//  In case no boundries are given, never grow larger than the screen size.
	m_max_x      (max_x ? max_x : Interactive_Base::get_xres()),
	m_max_y      (max_y ? max_y : Interactive_Base::get_yres()),

	m_orientation(orientation)
{}


/**
 * Adjust all the children and the box's size.
*/
void Box::resize()
{
	uint32_t idx;
	int32_t totaldepth;
	int32_t maxbreadth;

	// Adjust out size
	totaldepth = 0;
	maxbreadth = 0;

	for (idx = 0; idx < m_items.size(); ++idx) {
		int32_t depth, breadth;

		get_item_size(idx, &depth, &breadth);

		totaldepth += depth;
		if (breadth > maxbreadth)
			maxbreadth = breadth;
	}

	if (m_orientation == Horizontal)
		set_size(std::min(totaldepth, m_max_x), std::min(maxbreadth, m_max_y));
	else
		set_size(std::min(maxbreadth, m_max_x), std::min(totaldepth, m_max_y));

	// Position the children
	totaldepth = 0;

	for (idx = 0; idx < m_items.size(); ++idx) {
		int32_t depth;

		get_item_size(idx, &depth, 0);
		set_item_pos(idx, totaldepth);

		totaldepth += depth;
	}
}


/**
 * Add a new panel to be controlled by this box
*/
void Box::add(Panel* panel, uint32_t align)
{
	Item it;

	it.type = Item::ItemPanel;
	it.u.panel.panel = panel;
	it.u.panel.align = align;

	m_items.push_back(it);

	resize();
}


/**
 * Add spacing of empty pixels.
*/
void Box::add_space(uint32_t space)
{
	Item it;

	it.type = Item::ItemSpace;
	it.u.space = space;

	m_items.push_back(it);

	resize();
}


/**
 * Retrieve the given item's size. depth is the size of the item along the
 * orientation axis, breadth is the size perpendicular to the orientation axis.
*/
void Box::get_item_size(uint32_t idx, int32_t* pdepth, int32_t* pbreadth)
{
	assert(idx < m_items.size());

	const Item& it = m_items[idx];
	int32_t depth, breadth;

	switch (it.type) {
	case Item::ItemPanel:
		if (m_orientation == Horizontal) {
			depth   = it.u.panel.panel->get_w();
			breadth = it.u.panel.panel->get_h();
		} else {
			depth   = it.u.panel.panel->get_h();
			breadth = it.u.panel.panel->get_w();
		}
		break;

	case Item::ItemSpace:
		depth   = it.u.space;
		breadth = 0;
		break;

	default:
		throw wexception("Box::get_item_size: bad type %u", it.type);
	}

	if (pdepth)
		*pdepth   = depth;
	if (pbreadth)
		*pbreadth = breadth;
}


/**
 * Position the given item according to its parameters.
 * pos is the position relative to the parent in the direction of the
 * orientation axis.
*/
void Box::set_item_pos(uint32_t idx, int32_t pos)
{
	assert(idx < m_items.size());

	const Item& it = m_items[idx];

	switch (it.type) {
	case Item::ItemPanel: {
		int32_t breadth, maxbreadth;

		if (m_orientation == Horizontal) {
			breadth = it.u.panel.panel->get_h();
			maxbreadth = get_h();
		} else {
			breadth = it.u.panel.panel->get_w();
			maxbreadth = get_w();
		}
		switch (it.u.panel.align) {
		case AlignLeft:
		default:
			breadth = 0;
			break;

		case AlignCenter:
			breadth = (maxbreadth - breadth) / 2;
			break;

		case AlignRight:
			breadth = maxbreadth - breadth;
			break;
		}

		if (m_orientation == Horizontal)
			it  .u.panel.panel->set_pos(Point(pos, breadth));
		else it.u.panel.panel->set_pos(Point(breadth, pos));
		break;
	}

	case Item::ItemSpace:; //  no need to do anything
	};
}
};
