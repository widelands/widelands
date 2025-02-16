/*
 * Copyright (C) 2010-2025 by the Widelands Development Team
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

#ifndef WL_WUI_ACTIONCONFIRM_H
#define WL_WUI_ACTIONCONFIRM_H

#include "logic/widelands.h"

class InteractivePlayer;

namespace Widelands {
class Building;
struct PlayerImmovable;
struct Ship;
}  // namespace Widelands

// Building confirm windows
void show_bulldoze_confirm(InteractivePlayer& player,
                           Widelands::Building& building,
                           Widelands::PlayerImmovable* todestroy = nullptr);

void show_dismantle_confirm(InteractivePlayer& player, Widelands::Building& building);

void show_enhance_confirm(InteractivePlayer& player,
                          Widelands::Building& building,
                          const Widelands::DescriptionIndex& id,
                          bool constructionsite = false);

// Ship confirm windows
void show_ship_sink_confirm(InteractivePlayer& player, Widelands::Ship& ship);

void show_ship_cancel_expedition_confirm(InteractivePlayer& player, Widelands::Ship& ship);

// Diplomacy confirm windows
void show_resign_confirm(InteractivePlayer& player);

// Trade confirm windows
void show_cancel_trade_confirm(InteractivePlayer& player, Widelands::TradeID trade_id);

#endif  // end of include guard: WL_WUI_ACTIONCONFIRM_H
