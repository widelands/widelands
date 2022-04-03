/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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

#include "wui/itemwaresdisplay.h"

#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "ui_basic/mouse_constants.h"

namespace {

constexpr int kMargin = 4;
constexpr unsigned kHBorder = 10;
constexpr unsigned kVBorder = 10;
constexpr unsigned kDefaultItemsPerRow = 9;
constexpr unsigned kItemWidth = 14;
constexpr unsigned kItemHeight = 26;
constexpr unsigned kWorkerBaseline = 2;  ///< Offset of anim center from bottom border of item rect
constexpr unsigned kWareBaseLine = -6;

}  // anonymous namespace

/**
 * Create an ItemWaresDisplay with no items and zero capacity.
 */
ItemWaresDisplay::ItemWaresDisplay(Panel* parent, const Widelands::Player& gplayer)
   : Panel(parent, UI::PanelStyle::kWui, 0, 0, 0, 0),
     player_(gplayer),
     capacity_(0),
     items_per_row_(kDefaultItemsPerRow) {
	recalc_desired_size();
}

/**
 * Remove all items from the internal list of items.
 */
void ItemWaresDisplay::clear() {
	items_.clear();
}

/**
 * Set the nominal capacity of the display.
 *
 * This does not actually affect the number of items that can be added to the internal list,
 * but it does affect the desired size for this panel.
 */
void ItemWaresDisplay::set_capacity(uint32_t cap) {
	if (cap != capacity_) {
		capacity_ = cap;
		recalc_desired_size();
	}
}

void ItemWaresDisplay::recalc_desired_size() {
	uint32_t nrrows = (capacity_ + items_per_row_ - 1) / items_per_row_;
	uint32_t rowitems = capacity_ >= items_per_row_ ? items_per_row_ : capacity_;

	set_desired_size(2 * (kHBorder + kMargin) + rowitems * kItemWidth,
	                 2 * (kVBorder + kMargin) + nrrows * kItemHeight);
}

/**
 * Add an item to the end of the internal list.
 */
void ItemWaresDisplay::add(bool worker, Widelands::DescriptionIndex index) {
	Item it;
	it.worker = worker;
	it.index = index;
	items_.push_back(it);
}

void ItemWaresDisplay::draw(RenderTarget& dst) {
	const Widelands::TribeDescr& tribe(player().tribe());

	// Snazzy background
	const int width = get_w() - 2 * kMargin;
	const int height = get_h() - 2 * kMargin;
	RGBAColor black(0, 0, 0, 255);
	dst.brighten_rect(
	   Recti(kMargin, kMargin, width - 1, height - 1), -BUTTON_EDGE_BRIGHT_FACTOR / 2);
	//  bottom edge
	dst.brighten_rect(Recti(kMargin, height + 2, width, 2), 1.5 * BUTTON_EDGE_BRIGHT_FACTOR);
	//  right edge
	dst.brighten_rect(
	   Recti(kMargin + width - 2, kMargin, 2, height - 2), 1.5 * BUTTON_EDGE_BRIGHT_FACTOR);
	//  top edge
	dst.fill_rect(Recti(kMargin, kMargin, width - 1, 1), black);
	dst.fill_rect(Recti(kMargin, kMargin + 1, width - 2, 1), black);
	//  left edge
	dst.fill_rect(Recti(kMargin, kMargin, 1, height - 1), black);
	dst.fill_rect(Recti(kMargin + 1, kMargin, 1, height - 2), black);

	for (uint32_t idx = 0; idx < items_.size(); ++idx) {
		const Item& it = items_[idx];
		uint32_t row = idx / items_per_row_;
		uint32_t col = idx % items_per_row_;

		uint32_t x = kHBorder / 2 + col * kItemWidth + kMargin;
		uint32_t y = kVBorder + row * kItemHeight + kMargin;

		if (it.worker) {
			y += kWorkerBaseline;
			constexpr float kZoom = 1.f;
			dst.blit_animation(Vector2f(x + (kItemWidth / 2.f), y + (kItemHeight / 2.f)),
			                   Widelands::Coords::null(), kZoom,
			                   tribe.get_worker_descr(it.index)->main_animation(), Time(0),
			                   &player().get_playercolor());
		} else {
			y += kWareBaseLine;
			if (tribe.get_ware_descr(it.index)->icon() != nullptr) {
				dst.blit(Vector2i(x, y), tribe.get_ware_descr(it.index)->icon());
			}
		}
	}
}
