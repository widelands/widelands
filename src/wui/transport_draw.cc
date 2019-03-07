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
#include "economy/roadbase.h"
#include "economy/ware_instance.h"
#include "graphic/rendertarget.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"

namespace Widelands {

void Flag::draw(uint32_t gametime, const Vector2f& point_on_dst, float scale, RenderTarget* dst) {
	static struct {
		float x, y;
	} ware_offsets[8] = {{-5.f, 1.f},  {-1.f, 3.f},  {3.f, 3.f},  {7.f, 1.f},
	                     {-6.f, -3.f}, {-1.f, -2.f}, {3.f, -2.f}, {8.f, -3.f}};

	const RGBColor& player_color = owner().get_playercolor();
	dst->blit_animation(
	   point_on_dst, scale, owner().tribe().flag_animation(), gametime - animstart_, player_color);

	for (int32_t i = 0; i < ware_filled_; ++i) {  //  draw wares
		Vector2f warepos = point_on_dst;
		if (i < 8) {
			warepos.x += ware_offsets[i].x * scale;
			warepos.y += ware_offsets[i].y * scale;
		} else {
			warepos.y -= (6.f + (i - 8.f) * 3.f) * scale;
		}
		dst->blit_animation(
		   warepos, scale, wares_[i].ware->descr().get_animation("idle"), 0, player_color);
	}
}

/** The road is drawn by the terrain renderer via marked fields.
  * This function must only be called if the road segment to draw is a bridge!
  * The direction of the bridge is cached in cache_bridge_dir_to_draw_
  */
void RoadBase::draw(uint32_t gametime, const Vector2f& point_on_dst, float scale, RenderTarget* dst) {
	if (cache_bridge_dir_to_draw_ != WALK_E && cache_bridge_dir_to_draw_ != WALK_SE &&
			cache_bridge_dir_to_draw_ != WALK_SW) {
		return;
	}
	uint32_t const anim_idx = cache_bridge_dir_to_draw_ == WALK_E ? owner().tribe().bridge_e_animation() :
			cache_bridge_dir_to_draw_ == WALK_SE ? owner().tribe().bridge_se_animation() :
			owner().tribe().bridge_sw_animation();
	dst->blit_animation(point_on_dst, scale, anim_idx, gametime, owner().get_playercolor());
	cache_bridge_dir_to_draw_ = 0;
}
}  // namespace Widelands
