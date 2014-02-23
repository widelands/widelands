/*
 * Copyright (C) 2003, 2006-2011, 2013 by the Widelands Development Team
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

#include "ui_basic/box.h"

#include <algorithm>

#include <boost/bind.hpp>

#include "graphic/graphic.h"
#include "ui_basic/scrollbar.h"
#include "wexception.h"

namespace UI {
/**
 * Initialize an empty box
*/
Box::Box
	(Panel * const parent,
	 int32_t const x, int32_t const y,
	 uint32_t const orientation,
	 int32_t const max_x, int32_t const max_y, uint32_t const inner_spacing)
	:
	Panel        (parent, x, y, 0, 0),

	m_max_x      (max_x ? max_x : g_gr->get_xres()),
	m_max_y      (max_y ? max_y : g_gr->get_yres()),

	m_scrolling(false),
	m_scrollbar(nullptr),
	m_orientation(orientation),
	m_mindesiredbreadth(0),
	m_inner_spacing(inner_spacing)
{}

/**
 * Enable or disable the creation of a scrollbar if the maximum
 * depth is exceeded.
 *
 * Scrollbars are only created for the direction in which boxes
 * are added, e.g. if the box has a \ref Vertical orientation,
 * only a vertical scrollbar may be added.
 */
void Box::set_scrolling(bool scroll)
{
	if (scroll == m_scrolling)
		return;

	m_scrolling = scroll;
	update_desired_size();
}

/**
 * Set the minimum desired breadth.
 *
 * The breadth is the dimension of the box that is orthogonal to
 * its orientation.
 */
void Box::set_min_desired_breadth(uint32_t min)
{
	if (min == m_mindesiredbreadth)
		return;

	m_mindesiredbreadth = min;
	update_desired_size();
}

/**
 * Compute the desired size based on our children. This assumes that the
 * infinite space is zero, and is later on also re-used to calculate the
 * space assigned to an infinite space.
 */
void Box::update_desired_size()
{
	uint32_t totaldepth = 0;
	uint32_t maxbreadth = m_mindesiredbreadth;

	for (uint32_t idx = 0; idx < m_items.size(); ++idx) {
		uint32_t depth, breadth;
		get_item_desired_size(idx, depth, breadth);

		totaldepth += depth;
		if (breadth > maxbreadth)
			maxbreadth = breadth;
	}

	if (!m_items.empty())
		totaldepth += (m_items.size() - 1) * m_inner_spacing;

	if (m_orientation == Horizontal) {
		if (totaldepth > m_max_x && m_scrolling) {
			maxbreadth += Scrollbar::Size;
		}
		set_desired_size
			(std::min(totaldepth, m_max_x), // + get_lborder() + get_rborder(),
			 std::min(maxbreadth, m_max_y)); // + get_tborder() + get_bborder());
	} else {
		if (totaldepth > m_max_y && m_scrolling) {
			maxbreadth += Scrollbar::Size;
		}
		set_desired_size
			(std::min(maxbreadth, m_max_x) + get_lborder() + get_rborder(),
			 std::min(totaldepth, m_max_y) + get_tborder() + get_bborder());
	}

	//  This is not redundant, because even if all this does not change our
	//  desired size, we were typically called because of a child window that
	//  changed, and we need to relayout that.
	layout();
}

/**
 * Adjust all the children and the box's size.
 */
void Box::layout()
{
	// First pass: compute the depth and adjust whether we have a scrollbar
	uint32_t totaldepth = 0;

	for (uint32_t idx = 0; idx < m_items.size(); ++idx) {
		uint32_t depth, tmp;
		get_item_desired_size(idx, depth, tmp);

		totaldepth += depth;
	}

	if (!m_items.empty())
		totaldepth += (m_items.size() - 1) * m_inner_spacing;

	bool needscrollbar = false;
	if (m_orientation == Horizontal) {
		if (totaldepth > m_max_x && m_scrolling) {
			needscrollbar = true;
		}
	} else {
		if (totaldepth > m_max_y && m_scrolling) {
			needscrollbar = true;
		}
	}

	if (needscrollbar) {
		int32_t sb_x, sb_y, sb_w, sb_h;
		int32_t pagesize;
		if (m_orientation == Horizontal) {
			sb_x = 0;
			sb_y = get_inner_h() - Scrollbar::Size;
			sb_w = get_inner_w();
			sb_h = Scrollbar::Size;
			pagesize = get_inner_w() - Scrollbar::Size;
		} else {
			sb_x = get_inner_w() - Scrollbar::Size;
			sb_y = 0;
			sb_w = Scrollbar::Size;
			sb_h = get_inner_h();
			pagesize = get_inner_h() - Scrollbar::Size;
		}
		if (!m_scrollbar) {
			m_scrollbar = new Scrollbar
					(this, sb_x, sb_y, sb_w,
					 sb_h, m_orientation == Horizontal);
			m_scrollbar->moved.connect(boost::bind(&Box::scrollbar_moved, this, _1));
		} else {
			m_scrollbar->set_pos(Point(sb_x, sb_y));
			m_scrollbar->set_size(sb_w, sb_h);
		}

		m_scrollbar->set_steps(totaldepth - pagesize);
		m_scrollbar->set_singlestepsize(Scrollbar::Size);
		m_scrollbar->set_pagesize(pagesize);
	} else {
		delete m_scrollbar;
		m_scrollbar = nullptr;
	}

	// Second pass: Count number of infinite spaces
	uint32_t infspace_count = 0;
	for (uint32_t idx = 0; idx < m_items.size(); ++idx)
		if (m_items[idx].fillspace)
			infspace_count++;

	// Third pass: Distribute left over space to all infinite spaces. To
	// avoid having some pixels left at the end due to rounding errors, we
	// divide the remaining space by the number of remaining infinite
	// spaces every time, and not just one.
	uint32_t max_depths =
		m_orientation == Horizontal ? get_inner_w() : get_inner_h();
	for (uint32_t idx = 0; idx < m_items.size(); ++idx)
		if (m_items[idx].fillspace) {
			m_items[idx].assigned_var_depth =
				(max_depths - totaldepth) / infspace_count;
			totaldepth += m_items[idx].assigned_var_depth;
			infspace_count--;
	}

	// Forth pass: Update positions of all other items
	update_positions();
}

void Box::update_positions()
{
	int32_t scrollpos = m_scrollbar ? m_scrollbar->get_scrollpos() : 0;

	uint32_t totaldepth = 0;
	uint32_t totalbreadth = m_orientation == Horizontal ? get_inner_h() : get_inner_w();
	if (m_scrollbar)
		totalbreadth -= Scrollbar::Size;

	for (uint32_t idx = 0; idx < m_items.size(); ++idx) {
		uint32_t depth, breadth;
		get_item_size(idx, depth, breadth);

		if (m_items[idx].type == Item::ItemPanel) {
			set_item_size
				(idx, depth, m_items[idx].u.panel.fullsize ?
				 totalbreadth : breadth);
			set_item_pos(idx, totaldepth - scrollpos);
		}

		totaldepth += depth;
		totaldepth += m_inner_spacing;
	}
}

/**
 * Callback for scrollbar movement.
 */
void Box::scrollbar_moved(int32_t)
{
	update_positions();
}


/**
 * Add a new panel to be controlled by this box
 *
 * @param fullsize when true, @p panel will be extended to cover the entire width (or height)
 * of the box for horizontal (vertical) panels. If false, then @p panel may end up smaller;
 * in that case, it will be aligned according to @p align
 *
 * @param fillspace when true, @p panel will be expanded as an infinite space would be.
 * This can be used to make buttons fill a box completely.
 *
 */
void Box::add(Panel * const panel, uint32_t const align, bool fullsize, bool fillspace)
{
	Item it;

	it.type = Item::ItemPanel;
	it.u.panel.panel = panel;
	it.u.panel.align = align;
	it.u.panel.fullsize = fullsize;
	it.fillspace = fillspace;
	it.assigned_var_depth = 0;

	m_items.push_back(it);

	update_desired_size();
}


/**
 * Add spacing of empty pixels.
*/
void Box::add_space(uint32_t space)
{
	Item it;

	it.type = Item::ItemSpace;
	it.u.space = space;
	it.assigned_var_depth = 0;
	it.fillspace = false;

	m_items.push_back(it);

	update_desired_size();
}


/**
 * Add some infinite space (to align some buttons to the right)
*/
void Box::add_inf_space()
{
	Item it;

	it.type = Item::ItemSpace;
	it.u.space = 0;
	it.assigned_var_depth = 0;
	it.fillspace = true;

	m_items.push_back(it);

	update_desired_size();
}


/**
 * Retrieve the given item's desired size. depth is the size of the
 * item along the orientation axis, breadth is the size perpendicular
 * to the orientation axis.
*/
void Box::get_item_desired_size
	(uint32_t const idx, uint32_t & depth, uint32_t & breadth)
{
	assert(idx < m_items.size());

	const Item & it = m_items[idx];

	switch (it.type) {
	case Item::ItemPanel:
		if (m_orientation == Horizontal) {
			it.u.panel.panel->get_desired_size(depth, breadth);
		} else {
			it.u.panel.panel->get_desired_size(breadth, depth);
		}
		break;

	case Item::ItemSpace:
		depth   = it.u.space;
		breadth = 0;
		break;

	default:
		throw wexception("Box::get_item_size: bad type %u", it.type);
	}
}

/**
 * Retrieve the given item's size. This differs from get_item_desired_size only
 * for expanding items, at least for now.
 */
void Box::get_item_size
	(uint32_t const idx, uint32_t & depth, uint32_t & breadth)
{
	assert(idx < m_items.size());

	const Item & it = m_items[idx];

	get_item_desired_size(idx, depth, breadth);
	depth += it.assigned_var_depth;
}

/**
 * Set the given items actual size.
 */
void Box::set_item_size(uint32_t idx, uint32_t depth, uint32_t breadth)
{
	assert(idx < m_items.size());

	const Item & it = m_items[idx];

	if (it.type == Item::ItemPanel) {
		if (m_orientation == Horizontal)
			it.u.panel.panel->set_size(depth, breadth);
		else
			it.u.panel.panel->set_size(breadth, depth);
	}
}

/**
 * Position the given item according to its parameters.
 * pos is the position relative to the parent in the direction of the
 * orientation axis.
*/
void Box::set_item_pos(uint32_t idx, int32_t pos)
{
	assert(idx < m_items.size());

	const Item & it = m_items[idx];

	switch (it.type) {
	case Item::ItemPanel: {
		int32_t breadth, maxbreadth;

		if (m_orientation == Horizontal) {
			breadth = it.u.panel.panel->get_inner_h();
			maxbreadth = get_inner_h();
		} else {
			breadth = it.u.panel.panel->get_inner_w();
			maxbreadth = get_inner_w();
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

	case Item::ItemSpace:
		break; //  no need to do anything
	default:
		assert(false);
	};
}

}
