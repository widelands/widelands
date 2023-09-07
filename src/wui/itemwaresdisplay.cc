/*
 * Copyright (C) 2011-2023 by the Widelands Development Team
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

#include "graphic/animation/animation_manager.h"
#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "ui_basic/mouse_constants.h"

constexpr int kMargin = 4;
constexpr int kHBorder = 8;
constexpr int kVBorder = 8;
constexpr int kItemWidth = 22;
constexpr int kItemHeight = 28;

/**
 * Create an ItemWaresDisplay with no items and zero capacity.
 */
ItemWaresDisplay::ItemWaresDisplay(Panel* parent, const Widelands::Player& gplayer)
   : Panel(parent, UI::PanelStyle::kWui, "item_wares_display", 0, 0, 0, 0), player_(gplayer) {
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
void ItemWaresDisplay::add(Widelands::WareWorker type, Widelands::DescriptionIndex index) {
	items_.emplace_back(type, index);
}

bool ItemWaresDisplay::handle_mousemove(
   uint8_t /* state */, int32_t x, int32_t y, int32_t /* xdiff */, int32_t /* ydiff */) {
	const Item* i = at(x, y);
	set_tooltip(i == nullptr ?
                  std::string() :
	            i->type == Widelands::wwWARE ?
                  player_.egbase().descriptions().get_ware_descr(i->index)->descname() :
                  player_.egbase().descriptions().get_worker_descr(i->index)->descname());
	return true;
}

const ItemWaresDisplay::Item* ItemWaresDisplay::at(int32_t x, int32_t y) const {
	x -= (kHBorder + kMargin);
	y -= (kVBorder + kMargin);
	x /= kItemWidth;
	y /= kItemHeight;
	int32_t index = y * items_per_row_ + x;
	return (index >= 0 && index < static_cast<int32_t>(items_.size())) ? &items_.at(index) : nullptr;
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

		uint32_t x = kHBorder + col * kItemWidth + kMargin;
		uint32_t y = kVBorder + row * kItemHeight + kMargin;

		if (it.type == Widelands::wwWORKER) {
			constexpr float kZoom = 1.f;
			const uint32_t anim_id = tribe.get_worker_descr(it.index)->main_animation();
			const Animation& anim = g_animation_manager->get_animation(anim_id);
			dst.blit_animation(Vector2f(x + anim.hotspot().x + (kItemWidth - anim.width()) / 2.f,
			                            y + anim.hotspot().y + (kItemHeight - anim.height()) / 2.f),
			                   Widelands::Coords::null(), kZoom, anim_id, Time(0),
			                   &player().get_playercolor());
		} else {
			if (const Image* icon = tribe.get_ware_descr(it.index)->icon(); icon != nullptr) {
				dst.blit(Vector2i(x + (kItemWidth - icon->width()) / 2.f,
				                  y + (kItemHeight - icon->height()) / 2.f),
				         icon);
			}
		}
	}
}
