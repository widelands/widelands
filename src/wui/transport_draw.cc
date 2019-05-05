/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "economy/flag.h"
#include "economy/road.h"
#include "economy/ware_instance.h"
#include "graphic/rendertarget.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"

namespace Widelands {

void Flag::draw(uint32_t gametime,
                const TextToDraw,
                const Vector2f& field_on_dst,
                const Coords& coords,
                float scale,
                RenderTarget* dst) {
	static struct {
		float x, y;
	} ware_offsets[8] = {{-5.f, 1.f},  {-1.f, 3.f},  {3.f, 3.f},  {7.f, 1.f},
	                     {-6.f, -3.f}, {-1.f, -2.f}, {3.f, -2.f}, {8.f, -3.f}};

	const RGBColor& player_color = owner().get_playercolor();
	dst->blit_animation(field_on_dst, coords, scale, owner().tribe().flag_animation(),
	                    gametime - animstart_, &player_color);

	for (int32_t i = 0; i < ware_filled_; ++i) {  //  draw wares
		Vector2f warepos = field_on_dst;
		if (i < 8) {
			warepos.x += ware_offsets[i].x * scale;
			warepos.y += ware_offsets[i].y * scale;
		} else {
			warepos.y -= (6.f + (i - 8.f) * 3.f) * scale;
		}
		dst->blit_animation(warepos, Widelands::Coords::null(), scale,
		                    wares_[i].ware->descr().get_animation("idle"), 0, &player_color);
	}
}

/** The road is drawn by the terrain renderer via marked fields. */
void Road::draw(uint32_t, const TextToDraw, const Vector2f&, const Coords&, float, RenderTarget*) {
}
}  // namespace Widelands
