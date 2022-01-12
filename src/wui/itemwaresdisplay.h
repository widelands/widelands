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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_WUI_ITEMWARESDISPLAY_H
#define WL_WUI_ITEMWARESDISPLAY_H

#include "logic/widelands.h"
#include "ui_basic/panel.h"

namespace Widelands {
class Player;
}

/**
 * Display a mixed list of wares and workers using their in-game graphics,
 * as seen in the @ref ShipWindow.
 */
struct ItemWaresDisplay : UI::Panel {
	ItemWaresDisplay(UI::Panel* parent, const Widelands::Player& player);

	const Widelands::Player& player() const {
		return player_;
	}

	uint32_t capacity() const {
		return capacity_;
	}
	void set_capacity(uint32_t cap);

	uint32_t items_per_row() const {
		return items_per_row_;
	}

	void clear();
	void add(bool worker, Widelands::DescriptionIndex index);

	void draw(RenderTarget&) override;

private:
	struct Item {
		bool worker;
		Widelands::DescriptionIndex index;
	};

	void recalc_desired_size();

	const Widelands::Player& player_;
	uint32_t capacity_;
	uint32_t items_per_row_;
	std::vector<Item> items_;
};

#endif  // end of include guard: WL_WUI_ITEMWARESDISPLAY_H
