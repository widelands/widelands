/*
 * Copyright (C) 2010, 2013 by the Widelands Development Team
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

#ifndef _ACTIONCONFIRM_H_
#define _ACTIONCONFIRM_H_

struct Interactive_Player;

namespace Widelands {
class Building;
struct Building_Index;
struct PlayerImmovable;
struct Ship;
}

// Building confirm windows
void show_bulldoze_confirm
	(Interactive_Player & player,
	 Widelands::Building & building,
	 Widelands::PlayerImmovable * const todestroy = nullptr);

void show_dismantle_confirm
	(Interactive_Player & player,
	 Widelands::Building & building);

void show_enhance_confirm
	(Interactive_Player & player,
	 Widelands::Building & building,
	 const Widelands::Building_Index & id);

// Ship confirm windows
void show_ship_sink_confirm
	(Interactive_Player & player,
	 Widelands::Ship & ship);

void show_ship_cancel_expedition_confirm
	(Interactive_Player & player,
	 Widelands::Ship & ship);

#endif // _ACTIONCONFIRM_H_
