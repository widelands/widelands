/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#include "areawatcher.h"

#include "game.h"
#include "player.h"

namespace Widelands {

void AreaWatcher::act(Game * game, uint32_t)
{game->player(player_number).remove_areawatcher(*this); remove(game);}


Map_Object_Descr g_areawatcher_descr("areawatcher", "Areawatcher");


inline AreaWatcher::AreaWatcher(const Player_Area<> player_area) :
Map_Object(&g_areawatcher_descr), Player_Area<>(player_area)
{}


AreaWatcher & AreaWatcher::create
	(Editor_Game_Base & egbase, Player_Area<> const player_area)
{
	AreaWatcher & result = *new AreaWatcher(player_area);
	result.init(&egbase);
	return result;
}

};
