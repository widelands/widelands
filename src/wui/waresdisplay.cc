/*
 * Copyright (C) 2003-2017 by the Widelands Development Team
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

#include "wui/waresdisplay.h"

#include <cstdio>
#include <utility>

#include <boost/lexical_cast.hpp>

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"

const int WARE_MENU_INFO_SIZE = 12;

AbstractWaresDisplay::AbstractWaresDisplay(
   UI::Panel* const parent,
   int32_t x,
   int32_t y,
   const Widelands::TribeDescr& tribe,
   Widelands::WareWorker type,
   bool selectable,
   boost::function<void(Widelands::DescriptionIndex, bool)> callback_function,
   bool horizontal)
   :  // Size is set when add_warelist is called, as it depends on the type_.
     UI::Panel(parent, x, y, 0, 0),
     tribe_(tribe),

     type_(type),
     indices_(type_ == Widelands::wwWORKER ? tribe_.workers() : tribe_.wares()),
     curware_(this, 0, get_inner_h() - 25, get_inner_w(), 20, "", UI::Align::kCenter),

     selectable_(selectable),
     horizontal_(horizontal),
     selection_anchor_(Widelands::INVALID_INDEX),
     callback_function_(callback_function) {
	for (const Widelands::DescriptionIndex& index : indices_) {
		selected_.insert(std::make_pair(index, false));
		hidden_.insert(std::make_pair(index, false));
		in_selection_.insert(std::make_pair(index, false));
	}

	curware_.set_text(_("Stock"));

	// Find out geometry from icons_order
	unsigned int columns = icons_order().size();
	unsigned int rows = 0;
	for (unsigned int i = 0; i < icons_order().size(); i++)
		if (icons_order()[i].size() > rows)
			rows = icons_order()[i].size();
	if (horizontal_) {
		unsigned int s = columns;
		columns = rows;
		rows = s;
	}

	// 25 is height of curware_ text
	set_desired_size(
	   columns * (WARE_MENU_PIC_WIDTH + WARE_MENU_PIC_PAD_X) + 1,
	   rows * (WARE_MENU_PIC_HEIGHT + WARE_MENU_INFO_SIZE + WARE_MENU_PIC_PAD_Y) + 1 + 25);
}

bool AbstractWaresDisplay::handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t, int32_t) {
	const Widelands::DescriptionIndex index = ware_at_point(x, y);

	curware_.set_fixed_width(get_inner_w());

	curware_.set_text(index != Widelands::INVALID_INDEX ?
	                     (type_ == Widelands::wwWORKER ? tribe_.get_worker_descr(index)->descname() :
	                                                     tribe_.get_ware_descr(index)->descname()) :
	                     "");
	if (selection_anchor_ != Widelands::INVALID_INDEX) {
		// Ensure mouse button is still pressed as some
		// mouse release events do not reach us
		if (state ^ SDL_BUTTON_LMASK) {
			// TODO(unknown): We should call another function that will not pass that events
			// to our Panel superclass
			handle_mouserelease(SDL_BUTTON_LEFT, x, y);
			return true;
		}
		update_anchor_selection(x, y);
	}
	return true;
}

bool AbstractWaresDisplay::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		Widelands::DescriptionIndex ware = ware_at_point(x, y);

		if (!tribe_.has_ware(ware) && !tribe_.has_worker(ware)) {
			return false;
		}
		if (!selectable_) {
			return true;
		}
		if (selection_anchor_ == Widelands::INVALID_INDEX) {
			// Create the selection anchor to be able to select
			// multiple ware by dragging.
			selection_anchor_ = ware;
			in_selection_[ware] = true;
		} else {
			// A mouse release has been missed
		}
		return true;
	}

	return UI::Panel::handle_mousepress(btn, x, y);
}

bool AbstractWaresDisplay::handle_mouserelease(uint8_t btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT || selection_anchor_ == Widelands::INVALID_INDEX) {
		return UI::Panel::handle_mouserelease(btn, x, y);
	}

	bool to_be_selected = !ware_selected(selection_anchor_);

	for (const Widelands::DescriptionIndex& index : indices_) {
		if (in_selection_[index]) {
			if (to_be_selected) {
				select_ware(index);
			} else {
				unselect_ware(index);
			}
		}
	}

	// Release anchor, empty selection
	selection_anchor_ = Widelands::INVALID_INDEX;
	for (auto& resetme : in_selection_) {
		in_selection_[resetme.first] = false;
	}
	return true;
}

/**
 * Returns the index of the ware under the given coordinates, or
 * DescriptionIndex::null() if the given point is outside the range.
 */
Widelands::DescriptionIndex AbstractWaresDisplay::ware_at_point(int32_t x, int32_t y) const {
	if (x < 0 || y < 0)
		return Widelands::INVALID_INDEX;

	unsigned int i = x / (WARE_MENU_PIC_WIDTH + WARE_MENU_PIC_PAD_X);
	unsigned int j = y / (WARE_MENU_PIC_HEIGHT + WARE_MENU_INFO_SIZE + WARE_MENU_PIC_PAD_Y);
	if (horizontal_) {
		unsigned int s = i;
		i = j;
		j = s;
	}
	if (i < icons_order().size() && j < icons_order()[i].size()) {
		const Widelands::DescriptionIndex& ware = icons_order()[i][j];
		assert(hidden_.count(ware) == 1);
		if (!(hidden_.find(ware)->second)) {
			return ware;
		}
	}

	return Widelands::INVALID_INDEX;
}

// Update the anchored selection. An anchor has been created by mouse
// press. Mouse move call this function with the current mouse position.
// This function will temporary store all wares in the rectangle between anchor
// and current pos to allow their selection on mouse release
void AbstractWaresDisplay::update_anchor_selection(int32_t x, int32_t y) {
	if (selection_anchor_ == Widelands::INVALID_INDEX || x < 0 || y < 0) {
		return;
	}

	for (auto& resetme : in_selection_) {
		in_selection_[resetme.first] = false;
	}

	Vector2i anchor_pos = ware_position(selection_anchor_);
	// Add an offset to make sure the anchor line and column will be
	// selected when selecting in topleft direction
	int32_t anchor_x = anchor_pos.x + WARE_MENU_PIC_WIDTH / 2;
	int32_t anchor_y = anchor_pos.y + WARE_MENU_PIC_HEIGHT / 2;

	unsigned int left_ware_idx = anchor_x / (WARE_MENU_PIC_WIDTH + WARE_MENU_PIC_PAD_X);
	unsigned int top_ware_idx =
	   anchor_y / (WARE_MENU_PIC_HEIGHT + WARE_MENU_INFO_SIZE + WARE_MENU_PIC_PAD_Y);
	unsigned int right_ware_idx = x / (WARE_MENU_PIC_WIDTH + WARE_MENU_PIC_PAD_X);
	unsigned int bottoware_idx_ =
	   y / (WARE_MENU_PIC_HEIGHT + WARE_MENU_INFO_SIZE + WARE_MENU_PIC_PAD_Y);
	unsigned int tmp;

	// Reverse col/row and anchor/endpoint if needed
	if (horizontal_) {
		tmp = left_ware_idx;
		left_ware_idx = top_ware_idx;
		top_ware_idx = tmp;
		tmp = right_ware_idx;
		right_ware_idx = bottoware_idx_;
		bottoware_idx_ = tmp;
	}
	if (left_ware_idx > right_ware_idx) {
		tmp = left_ware_idx;
		left_ware_idx = right_ware_idx;
		right_ware_idx = tmp;
	}
	if (top_ware_idx > bottoware_idx_) {
		tmp = top_ware_idx;
		top_ware_idx = bottoware_idx_;
		bottoware_idx_ = tmp;
	}

	for (unsigned int cur_ware_x = left_ware_idx; cur_ware_x <= right_ware_idx; cur_ware_x++) {
		if (cur_ware_x < icons_order().size()) {
			for (unsigned cur_ware_y = top_ware_idx; cur_ware_y <= bottoware_idx_; cur_ware_y++) {
				if (cur_ware_y < icons_order()[cur_ware_x].size()) {
					Widelands::DescriptionIndex ware = icons_order()[cur_ware_x][cur_ware_y];
					if (!hidden_[ware]) {
						in_selection_[ware] = true;
					}
				}
			}
		}
	}
}

void AbstractWaresDisplay::layout() {
	curware_.set_pos(Vector2i(0, get_inner_h() - 25));
	curware_.set_size(get_inner_w(), 20);
}

void WaresDisplay::remove_all_warelists() {
	warelists_.clear();
}

void AbstractWaresDisplay::draw(RenderTarget& dst) {
	for (const Widelands::DescriptionIndex& index : indices_) {
		if (!hidden_[index]) {
			draw_ware(dst, index);
		}
	}
}

const Widelands::TribeDescr::WaresOrder& AbstractWaresDisplay::icons_order() const {
	switch (type_) {
	case Widelands::wwWARE:
		return tribe_.wares_order();
	case Widelands::wwWORKER:
		return tribe_.workers_order();
	}
	NEVER_HERE();
}

const Widelands::TribeDescr::WaresOrderCoords& AbstractWaresDisplay::icons_order_coords() const {
	switch (type_) {
	case Widelands::wwWARE:
		return tribe_.wares_order_coords();
	case Widelands::wwWORKER:
		return tribe_.workers_order_coords();
	}
	NEVER_HERE();
}

Vector2i AbstractWaresDisplay::ware_position(Widelands::DescriptionIndex id) const {
	Vector2i p(2, 2);
	if (horizontal_) {
		p.x += icons_order_coords()[id].second * (WARE_MENU_PIC_WIDTH + WARE_MENU_PIC_PAD_X);
		p.y += icons_order_coords()[id].first *
		       (WARE_MENU_PIC_HEIGHT + WARE_MENU_PIC_PAD_Y + WARE_MENU_INFO_SIZE);
	} else {
		p.x += icons_order_coords()[id].first * (WARE_MENU_PIC_WIDTH + WARE_MENU_PIC_PAD_X);
		p.y += icons_order_coords()[id].second *
		       (WARE_MENU_PIC_HEIGHT + WARE_MENU_PIC_PAD_Y + WARE_MENU_INFO_SIZE);
	}
	return p;
}

/*
===============
WaresDisplay::draw_ware [virtual]

Draw one ware icon + additional information.
===============
*/
void AbstractWaresDisplay::draw_ware(RenderTarget& dst, Widelands::DescriptionIndex id) {
	bool draw_selected = selected_[id];
	if (selection_anchor_ != Widelands::INVALID_INDEX) {
		// Draw the temporary selected wares as if they were
		// selected.
		// TODO(unknown): Use another pic for the temporary selection
		if (!ware_selected(selection_anchor_)) {
			draw_selected |= in_selection_[id];
		} else {
			draw_selected &= !in_selection_[id];
		}
	}

	//  draw a background
	const Image* bgpic = g_gr->images().get(draw_selected ? "images/wui/ware_list_bg_selected.png" :
	                                                        "images/wui/ware_list_bg.png");
	uint16_t w = bgpic->width();

	const Vector2i p = ware_position(id);
	dst.blit(p, bgpic);

	const Image* icon = type_ == Widelands::wwWORKER ? tribe_.get_worker_descr(id)->icon() :
	                                                   tribe_.get_ware_descr(id)->icon();

	dst.blit(p + Vector2i((w - WARE_MENU_PIC_WIDTH) / 2, 1), icon);

	dst.fill_rect(Rectf(p + Vector2i(0.f, WARE_MENU_PIC_HEIGHT), w, WARE_MENU_INFO_SIZE),
	              info_color_for_ware(id));

	const Image* text = UI::g_fh1->render(as_waresinfo(info_for_ware(id)));
	if (text)  // might be zero when there is no info text.
		dst.blit(p + Vector2i(w - text->width() - 1,
		                      WARE_MENU_PIC_HEIGHT + WARE_MENU_INFO_SIZE + 1 - text->height()),
		         text);
}

// Wares highlighting/selecting
void AbstractWaresDisplay::select_ware(Widelands::DescriptionIndex ware) {
	if (selected_[ware])
		return;

	selected_[ware] = true;
	if (callback_function_)
		callback_function_(ware, true);
}

void AbstractWaresDisplay::unselect_ware(Widelands::DescriptionIndex ware) {
	if (!selected_[ware])
		return;

	selected_[ware] = false;
	if (callback_function_)
		callback_function_(ware, false);
}

bool AbstractWaresDisplay::ware_selected(Widelands::DescriptionIndex ware) {
	return selected_[ware];
}

// Wares hiding
void AbstractWaresDisplay::hide_ware(Widelands::DescriptionIndex ware) {
	if (hidden_[ware])
		return;
	hidden_[ware] = true;
}

void AbstractWaresDisplay::unhide_ware(Widelands::DescriptionIndex ware) {
	if (!hidden_[ware])
		return;
	hidden_[ware] = false;
}

bool AbstractWaresDisplay::ware_hidden(Widelands::DescriptionIndex ware) {
	return hidden_[ware];
}

WaresDisplay::WaresDisplay(UI::Panel* const parent,
                           int32_t x,
                           int32_t y,
                           const Widelands::TribeDescr& tribe,
                           Widelands::WareWorker type,
                           bool selectable)
   : AbstractWaresDisplay(parent, x, y, tribe, type, selectable) {
}

RGBColor AbstractWaresDisplay::info_color_for_ware(Widelands::DescriptionIndex /* ware */) {
	return RGBColor(0, 0, 0);
}

WaresDisplay::~WaresDisplay() {
	remove_all_warelists();
}

std::string WaresDisplay::info_for_ware(Widelands::DescriptionIndex ware) {
	int totalstock = 0;
	for (const Widelands::WareList* warelist : warelists_) {
		totalstock += warelist->stock(ware);
	}
	return boost::lexical_cast<std::string>(totalstock);
}

/*
===============
add a ware list to be displayed in this WaresDisplay
===============
*/
void WaresDisplay::add_warelist(const Widelands::WareList& wares) {
	//  If you register something twice, it is counted twice. Not my problem.
	warelists_.push_back(&wares);
}

std::string waremap_to_richtext(const Widelands::TribeDescr& tribe,
                                const std::map<Widelands::DescriptionIndex, uint8_t>& map) {
	std::string ret;

	std::map<Widelands::DescriptionIndex, uint8_t>::const_iterator c;

	Widelands::TribeDescr::WaresOrder::iterator i;
	std::vector<Widelands::DescriptionIndex>::iterator j;
	Widelands::TribeDescr::WaresOrder order = tribe.wares_order();

	for (i = order.begin(); i != order.end(); ++i)
		for (j = i->begin(); j != i->end(); ++j)
			if ((c = map.find(*j)) != map.end()) {
				ret += "<div width=30 padding=2><p align=center>"
				       "<div width=26 background=454545><p align=center><img src=\"" +
				       tribe.get_ware_descr(c->first)->icon_filename() +
				       "\"></p></div><div width=26 background=000000><p><font size=9>" +
				       boost::lexical_cast<std::string>(static_cast<int32_t>(c->second)) +
				       "</font></p></div></p></div>";
			}
	return ret;
}
