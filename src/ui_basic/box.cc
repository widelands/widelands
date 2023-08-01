/*
 * Copyright (C) 2003-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ui_basic/box.h"

#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "ui_basic/scrollbar.h"

namespace UI {
/**
 * Initialize an empty box
 */
Box::Box(Panel* const parent,
         PanelStyle s,
         const std::string& name,
         int32_t const x,
         int32_t const y,
         uint32_t const orientation,
         int32_t const max_x,
         int32_t const max_y,
         uint32_t const inner_spacing)
   : Panel(parent, s, name, x, y, 0, 0),

     max_x_(max_x != 0 ? max_x : g_gr->get_xres()),
     max_y_(max_y != 0 ? max_y : g_gr->get_yres()),

     scrollbar_(nullptr),
     orientation_(orientation),

     inner_spacing_(inner_spacing) {
	set_snap_target(true);
}

/**
 * Enable or disable the creation of a scrollbar if the maximum
 * depth is exceeded.
 *
 * Scrollbars are only created for the direction in which boxes
 * are added, e.g. if the box has a \ref Vertical orientation,
 * only a vertical scrollbar may be added.
 */
void Box::set_scrolling(bool scroll) {
	if (scroll == scrolling_) {
		return;
	}

	scrolling_ = scroll;
	update_desired_size();
}

/**
 * If set to `true`, a scrollbar will always be used regardless of
 * whether `scrolling_` is true and whether a scrollbar is needed.
 */
void Box::set_force_scrolling(bool f) {
	if (force_scrolling_ == f) {
		return;
	}
	force_scrolling_ = f;
	update_desired_size();
}

/**
 * Set the minimum desired breadth.
 *
 * The breadth is the dimension of the box that is orthogonal to
 * its orientation.
 */
void Box::set_min_desired_breadth(uint32_t min) {
	if (min == mindesiredbreadth_) {
		return;
	}

	mindesiredbreadth_ = min;
	update_desired_size();
}

/**
 * Sets the value for the inner spacing.
 * \note This does not relayout the box.
 */
void Box::set_inner_spacing(uint32_t size) {
	inner_spacing_ = size;
}

void Box::set_max_size(int w, int h) {
	max_x_ = w;
	max_y_ = h;
	set_desired_size(w, h);
}

/**
 * Compute the desired size based on our children. This assumes that the
 * infinite space is zero, and is later on also re-used to calculate the
 * space assigned to an infinite space.
 */
void Box::update_desired_size() {
	int totaldepth = 0;
	int maxbreadth = mindesiredbreadth_;
	int spacing = -1 * inner_spacing_;

	for (uint32_t idx = 0; idx < items_.size(); ++idx) {
		int depth = 0;
		int breadth = 0;
		get_item_desired_size(idx, &depth, &breadth);

		totaldepth += depth;
		if (is_item_visible(idx)) {
			spacing += inner_spacing_;
		}
		if (breadth > maxbreadth) {
			maxbreadth = breadth;
		}
	}

	if (spacing > 0) {
		totaldepth += spacing;
	}

	if (orientation_ == Horizontal) {
		if ((totaldepth > max_x_ && scrolling_) || force_scrolling_) {
			maxbreadth += Scrollbar::kSize;
		}
		set_desired_size(std::min(totaldepth, max_x_),   // + get_lborder() + get_rborder(),
		                 std::min(maxbreadth, max_y_));  // + get_tborder() + get_bborder());
	} else {
		if ((totaldepth > max_y_ && scrolling_) || force_scrolling_) {
			maxbreadth += Scrollbar::kSize;
		}
		set_desired_size(std::min(maxbreadth, max_x_) + get_lborder() + get_rborder(),
		                 std::min(totaldepth, max_y_) + get_tborder() + get_bborder());
	}

	//  This is not redundant, because even if all this does not change our
	//  desired size, we were typically called because of a child window that
	//  changed, and we need to relayout that.
	layout();
}

bool Box::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	if (scrollbar_) {
		assert(scrolling_ || force_scrolling_);
		return scrollbar_->handle_mousewheel(x, y, modstate);
	}
	return Panel::handle_mousewheel(x, y, modstate);
}

bool Box::handle_key(bool down, SDL_Keysym code) {
	if (scrollbar_) {
		assert(scrolling_ || force_scrolling_);
		return scrollbar_->handle_key(down, code);
	}
	return Panel::handle_key(down, code);
}

/**
 * Adjust all the children and the box's size.
 */
void Box::layout() {
	// First pass: compute the depth and count number of infinite spaces
	int totaldepth = 0;
	int spacing = -1 * inner_spacing_;
	int infspace_count = 0;

	for (size_t idx = 0; idx < items_.size(); ++idx) {
		int depth;
		int unused = 0;
		get_item_desired_size(idx, &depth, &unused);
		totaldepth += depth;
		if (is_item_visible(idx)) {
			spacing += inner_spacing_;
			if (items_[idx].fillspace) {
				infspace_count++;
			}
		}
	}

	if (spacing > 0) {
		totaldepth += spacing;
	}

	// Second pass: Adjust whether we have a scrollbar
	int max_depths = orientation_ == Horizontal ? get_inner_w() : get_inner_h();
	bool needscrollbar = force_scrolling_;
	if (!force_scrolling_ && scrolling_) {
		needscrollbar = totaldepth > max_depths;
	}

	if (needscrollbar) {
		int32_t sb_x;
		int32_t sb_y;
		int32_t sb_w;
		int32_t sb_h;
		int32_t pagesize;
		if (orientation_ == Horizontal) {
			sb_x = 0;
			sb_y = get_inner_h() - Scrollbar::kSize;
			sb_w = get_inner_w();
			sb_h = Scrollbar::kSize;
			pagesize = get_inner_w() - Scrollbar::kSize;
		} else {
			sb_x = get_inner_w() - Scrollbar::kSize;
			sb_y = 0;
			sb_w = Scrollbar::kSize;
			sb_h = get_inner_h();
			pagesize = get_inner_h() - Scrollbar::kSize;
		}
		if (scrollbar_ == nullptr) {
			scrollbar_.reset(new Scrollbar(
			   this, "scrollbar", sb_x, sb_y, sb_w, sb_h, panel_style_, orientation_ == Horizontal));
			scrollbar_->moved.connect([this](int32_t a) { scrollbar_moved(a); });
		} else {
			scrollbar_->set_pos(Vector2i(sb_x, sb_y));
			scrollbar_->set_size(sb_w, sb_h);
		}
		scrollbar_->set_steps(totaldepth - pagesize);
		scrollbar_->set_singlestepsize(Scrollbar::kSize);
		scrollbar_->set_pagesize(pagesize);
		scrollbar_->set_force_draw(force_scrolling_);
	} else {
		scrollbar_.reset();
	}

	// Third pass: Distribute left over space to all infinite spaces. To
	// avoid having some pixels left at the end due to rounding errors, we
	// divide the remaining space by the number of remaining infinite
	// spaces every time, and not just one.
	for (size_t idx = 0; idx < items_.size(); ++idx) {
		Item& item = items_[idx];
		if (item.fillspace && is_item_visible(idx)) {
			assert(infspace_count > 0);
			// Avoid division by 0
			item.assigned_var_depth =
			   std::max(0, (max_depths - totaldepth) / std::max(1, infspace_count));
			totaldepth += item.assigned_var_depth;
			infspace_count--;
		} else {
			item.assigned_var_depth = 0;
		}
	}

	// Fourth pass: Update positions of all other items
	update_positions();
}

void Box::update_positions() {
	int32_t scrollpos = scrollbar_ ? scrollbar_->get_scrollpos() : 0;

	int32_t totaldepth = 0;
	uint32_t totalbreadth = orientation_ == Horizontal ? get_inner_h() : get_inner_w();
	if (scrollbar_ && scrollbar_->is_enabled()) {
		totalbreadth -= Scrollbar::kSize;
	}

	for (uint32_t idx = 0; idx < items_.size(); ++idx) {
		if (!is_item_visible(idx)) {
			// Do not modify hidden items
			continue;
		}
		int depth;
		int breadth = 0;
		get_item_size(idx, &depth, &breadth);

		if (items_[idx].type == Item::ItemPanel) {
			set_item_size(idx, depth, items_[idx].u.panel.fullsize ? totalbreadth : breadth);
			// Update depth, in case item did self-layouting
			get_item_size(idx, &depth, &breadth);
			set_item_pos(idx, totaldepth - scrollpos);
		}

		totaldepth += depth + inner_spacing_;
	}

#ifndef NDEBUG
	// Subtract last spacing + some leeway from infspace calculation
	totaldepth = totaldepth - items_.size() - inner_spacing_;
	int max_depths = orientation_ == Horizontal ? get_w() : get_h();
	if (!force_scrolling_ && !scrolling_ && max_depths > 0 && totaldepth > max_depths) {
		// TODO(matthiakl): one day this should be an error
		verb_log_dbg("Overflowing %s box %s: %d > %d ",
		             orientation_ == Horizontal ? "horizontal" : "vertical  ", get_name().c_str(),
		             totaldepth, max_depths);
	}
#endif
}

/**
 * Callback for scrollbar movement.
 */
void Box::scrollbar_moved(int32_t /* movement */) {
	update_positions();
}

/**
 * Add a new panel to be controlled by this box
 *
 * @param resizing:
 *
 * When Resizing::kAlign, then @p panel will be aligned according to @p align
 *
 * When Resizing::kFullSize, @p panel will be extended to cover the entire width (or height)
 * of the box for horizontal (vertical) panels.
 *
 * When Resizing::kFillSpace, @p panel will be expanded as an infinite space would be.
 * This can be used to make buttons fill a box completely.
 *
 * When Resizing::kExpandBoth, both width and height of @p panel will be expanded.
 *
 */
void Box::add(Panel* const panel, Resizing resizing, UI::Align const align) {
	assert(panel->get_parent() == this);

	Item it;

	it.type = Item::ItemPanel;
	it.u.panel.panel = panel;
	it.u.panel.align = align;
	it.u.panel.fullsize = resizing == Resizing::kFullSize || resizing == Resizing::kExpandBoth;
	it.fillspace = resizing == Resizing::kFillSpace || resizing == Resizing::kExpandBoth;
	it.assigned_var_depth = 0;

	// Ensure that tab focus order follows layout
	panel->move_to_top();
	items_.push_back(it);

	update_desired_size();
}

/**
 * Add spacing of empty pixels.
 */
void Box::add_space(uint32_t space) {
	Item it;

	it.type = Item::ItemSpace;
	it.u.space = space;
	it.assigned_var_depth = 0;
	it.fillspace = false;

	items_.push_back(it);

	update_desired_size();
}

/**
 * Add some infinite space (to align some buttons to the right)
 */
void Box::add_inf_space() {
	Item it;

	it.type = Item::ItemSpace;
	it.u.space = 0;
	it.assigned_var_depth = 0;
	it.fillspace = true;

	items_.push_back(it);

	update_desired_size();
}

/**
 * Checks whether the given item is currently visible.
 * Spaces are always visible.
 */
bool Box::is_item_visible(uint32_t idx) {
	return items_[idx].type == Item::ItemSpace || items_[idx].u.panel.panel->is_visible();
}

/**
 * Retrieve the given item's desired size. depth is the size of the
 * item along the orientation axis, breadth is the size perpendicular
 * to the orientation axis.
 */
void Box::get_item_desired_size(uint32_t const idx, int* depth, int* breadth) {
	assert(idx < items_.size());

	const Item& it = items_[idx];

	switch (it.type) {
	case Item::ItemPanel:
		if (!it.u.panel.panel->is_visible()) {
			*depth = 0;
			*breadth = 0;
			return;
		}
		if (orientation_ == Horizontal) {
			it.u.panel.panel->get_desired_size(depth, breadth);
		} else {
			it.u.panel.panel->get_desired_size(breadth, depth);
		}
		break;

	case Item::ItemSpace:
		*depth = it.u.space;
		*breadth = 0;
		break;
	}
}

/**
 * Retrieve the given item's size. This differs from get_item_desired_size only
 * for expanding items, at least for now.
 */
void Box::get_item_size(uint32_t const idx, int* depth, int* breadth) {
	assert(idx < items_.size());

	const Item& it = items_[idx];

	get_item_desired_size(idx, depth, breadth);
	*depth += it.assigned_var_depth;
}

/**
 * Set the given items actual size.
 */
void Box::set_item_size(uint32_t idx, int depth, int breadth) {
	assert(idx < items_.size());

	const Item& it = items_[idx];

	if (it.type == Item::ItemPanel) {
		if (orientation_ == Horizontal) {
			it.u.panel.panel->set_size(depth, breadth);
		} else {
			it.u.panel.panel->set_size(breadth, depth);
		}
	}
}

/**
 * Position the given item according to its parameters.
 * pos is the position relative to the parent in the direction of the
 * orientation axis.
 */
void Box::set_item_pos(uint32_t idx, int32_t pos) {
	assert(idx < items_.size());

	const Item& it = items_[idx];

	switch (it.type) {
	case Item::ItemPanel: {
		int32_t breadth;
		int32_t maxbreadth = 0;

		if (orientation_ == Horizontal) {
			breadth = it.u.panel.panel->get_inner_h();
			maxbreadth = get_inner_h();
		} else {
			breadth = it.u.panel.panel->get_inner_w();
			maxbreadth = get_inner_w();
		}
		if (scrollbar_ && scrollbar_->is_enabled()) {
			maxbreadth -= Scrollbar::kSize;
		}
		switch (it.u.panel.align) {
		case UI::Align::kCenter:
			breadth = (maxbreadth - breadth) / 2;
			break;

		case UI::Align::kRight:
			breadth = maxbreadth - breadth;
			break;
		case UI::Align::kLeft:
			breadth = 0;
		}

		if (orientation_ == Horizontal) {
			it.u.panel.panel->set_pos(Vector2i(pos, breadth));
		} else {
			it.u.panel.panel->set_pos(Vector2i(breadth, pos));
		}
		break;
	}

	case Item::ItemSpace:
		break;  //  no need to do anything
	}
}
void Box::on_death(Panel* p) {
	auto is_deleted_panel = [p](Box::Item i) { return p == i.u.panel.panel; };
	items_.erase(std::remove_if(items_.begin(), items_.end(), is_deleted_panel), items_.end());

	update_desired_size();
}
void Box::on_visibility_changed() {
	update_desired_size();
}
}  // namespace UI
