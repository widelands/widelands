/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "economy/flag.h"
#include "economy/road.h"
#include "economy/ware_instance.h"
#include "editor_game_base.h"
#include "rendertarget.h"

namespace Widelands {

void Flag::draw
	(Editor_Game_Base const &       game,
	 RenderTarget           &       dst,
	 FCoords,
	 Point                    const pos)
{
	static struct {int32_t x, y;} ware_offsets[8] = {
		{-5,  1},
		{-1,  3},
		{3,  3},
		{7,  1},
		{-6, -3},
		{-1, -2},
		{3, -2},
		{8, -3}
	};

	dst.drawanim(pos, m_anim, game.get_gametime() - m_animstart, get_owner());

	const uint32_t item_filled = m_item_filled;
	for (uint32_t i = 0; i < item_filled; ++i) { //  draw wares
		Point warepos = pos;
		if (i < 8) {
			warepos.x += ware_offsets[i].x;
			warepos.y += ware_offsets[i].y;
		} else
			warepos.y -= 6 + (i - 8) * 3;
		dst.drawanim
			(warepos,
			 m_items[i].item->descr().get_animation("idle"),
			 0,
			 get_owner());
	}
}

/** The road is drawn by the terrain renderer via marked fields. */
void Road::draw(Editor_Game_Base const &, RenderTarget &, FCoords, Point) {}

};
