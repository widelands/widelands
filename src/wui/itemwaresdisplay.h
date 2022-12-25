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

#ifndef WL_WUI_ITEMWARESDISPLAY_H
#define WL_WUI_ITEMWARESDISPLAY_H

#include "logic/map_objects/tribes/wareworker.h"
#include "logic/widelands.h"
#include "ui_basic/panel.h"

namespace Widelands {
class Player;
}  // namespace Widelands

/**
 * Display a mixed list of wares and workers using their in-game graphics,
 * as seen in the @ref ShipWindow.
 */
struct ItemWaresDisplay : UI::Panel {
	struct Item {
		Item(Widelands::WareWorker ww, Widelands::DescriptionIndex di) : type(ww), index(di) {
		}
		Widelands::WareWorker type;
		Widelands::DescriptionIndex index;
	};

	ItemWaresDisplay(UI::Panel* parent, const Widelands::Player& player);

	[[nodiscard]] const Widelands::Player& player() const {
		return player_;
	}

	[[nodiscard]] uint32_t capacity() const {
		return capacity_;
	}
	void set_capacity(uint32_t cap);

	[[nodiscard]] uint32_t items_per_row() const {
		return items_per_row_;
	}

	[[nodiscard]] const Item* at(int32_t x, int32_t y) const;

	void clear();
	void add(Widelands::WareWorker type, Widelands::DescriptionIndex index);

	void draw(RenderTarget&) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;

private:
	static constexpr unsigned kDefaultItemsPerRow = 7;

	void recalc_desired_size();

	const Widelands::Player& player_;
	uint32_t capacity_{0U};
	uint32_t items_per_row_{kDefaultItemsPerRow};
	std::vector<Item> items_;
};

#endif  // end of include guard: WL_WUI_ITEMWARESDISPLAY_H
