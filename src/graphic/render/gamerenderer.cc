/*
 * Copyright (C) 2010-2013 by the Widelands Development Team
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

#include "graphic/render/gamerenderer.h"

#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"
#include "upcast.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/overlay_manager.h"

using namespace Widelands;

GameRenderer::GameRenderer()
{
}

GameRenderer::~GameRenderer()
{
}

void GameRenderer::rendermap
	(RenderTarget & dst,
	 const Widelands::Editor_Game_Base &       egbase,
	 const Widelands::Player           &       player,
	 const Point                       &       viewofs)
{
	m_dst = &dst;
	m_dst_offset = -viewofs;
	m_egbase = &egbase;
	m_player = &player;

	draw_wrapper();
}

void GameRenderer::rendermap
	(RenderTarget & dst,
	 const Widelands::Editor_Game_Base & egbase,
	 const Point                       & viewofs)
{
	m_dst = &dst;
	m_dst_offset = -viewofs;
	m_egbase = &egbase;
	m_player = nullptr;

	draw_wrapper();
}

void GameRenderer::draw_wrapper()
{
	Point tl_map = m_dst->get_offset() - m_dst_offset;

	assert(tl_map.x >= 0); // divisions involving negative numbers are bad
	assert(tl_map.y >= 0);

	m_minfx = tl_map.x / TRIANGLE_WIDTH - 1;
	m_minfy = tl_map.y / TRIANGLE_HEIGHT - 1;
	m_maxfx = (tl_map.x + m_dst->get_rect().w + (TRIANGLE_WIDTH / 2)) / TRIANGLE_WIDTH;
	m_maxfy = (tl_map.y + m_dst->get_rect().h) / TRIANGLE_HEIGHT;

	// fudge for triangle boundary effects and for height differences
	m_minfx -= 1;
	m_minfy -= 1;
	m_maxfx += 1;
	m_maxfy += 10;

	draw();
}

void GameRenderer::draw_objects()
{
	static const uint32_t F = 0;
	static const uint32_t R = 1;
	static const uint32_t BL = 2;
	static const uint32_t BR = 3;
	const Map & map = m_egbase->map();

	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords coords[4];
			coords[F] = map.get_fcoords(ncoords);
			coords[R] = map.r_n(coords[F]);
			coords[BL] = map.bl_n(coords[F]);
			coords[BR] = map.br_n(coords[F]);
			Point pos[4];
			MapviewPixelFunctions::get_basepix(Coords(fx, fy), pos[F].x, pos[F].y);
			MapviewPixelFunctions::get_basepix(Coords(fx + 1, fy), pos[R].x, pos[R].y);
			MapviewPixelFunctions::get_basepix(Coords(fx + (fy & 1) - 1, fy + 1), pos[BL].x, pos[BL].y);
			MapviewPixelFunctions::get_basepix(Coords(fx + (fy & 1), fy + 1), pos[BR].x, pos[BR].y);
			for (uint32_t d = 0; d < 4; ++d) {
				pos[d].y -= coords[d].field->get_height() * HEIGHT_FACTOR;
				pos[d] += m_dst_offset;
			}

			Player_Number owner_number[4];
			bool isborder[4];
			Vision vision[4] = {2, 2, 2, 2};
			for (uint32_t d = 0; d < 4; ++d)
				owner_number[d] = coords[d].field->get_owned_by();
			for (uint32_t d = 0; d < 4; ++d)
				isborder[d] = coords[d].field->is_border();

			if (m_player && !m_player->see_all()) {
				for (uint32_t d = 0; d < 4; ++d) {
					const Player::Field & pf = m_player->fields()[map.get_index(coords[d], map.get_width())];
					vision[d] = pf.vision;
					if (pf.vision == 1) {
						owner_number[d] = pf.owner;
						isborder[d] = pf.border;
					}
				}
			}

			if (isborder[F]) {
				const Player & owner = m_egbase->player(owner_number[F]);
				uint32_t const anim_idx = owner.frontier_anim();
				if (vision[F])
					m_dst->drawanim(pos[F], anim_idx, 0, &owner);
				for (uint32_t d = 1; d < 4; ++d) {
					if
						((vision[F] || vision[d]) &&
						 isborder[d] &&
						 (owner_number[d] == owner_number[F] || !owner_number[d]))
					{
						m_dst->drawanim(middle(pos[F], pos[d]), anim_idx, 0, &owner);
					}
				}
			}

			if (1 < vision[F]) { // Render stuff that belongs to the node.
				if (BaseImmovable * const imm = coords[F].field->get_immovable())
					imm->draw(*m_egbase, *m_dst, coords[F], pos[F]);
				for
					(Bob * bob = coords[F].field->get_first_bob();
					 bob;
					 bob = bob->get_next_bob())
					bob->draw(*m_egbase, *m_dst, pos[F]);
			} else if (vision[F] == 1) {
				const Player::Field & f_pl = m_player->fields()[map.get_index(coords[F], map.get_width())];
				const Player * owner = owner_number[F] ? m_egbase->get_player(owner_number[F]) : nullptr;
				if
					(const Map_Object_Descr * const map_object_descr =
						f_pl.map_object_descr[TCoords<>::None])
				{
					if
						(f_pl.constructionsite.becomes)
					{
						const Player::Constructionsite_Information & csinf = f_pl.constructionsite;
						// draw the partly finished constructionsite
						uint32_t anim_idx;
						try {
							anim_idx = csinf.becomes->get_animation("build");
						} catch (Map_Object_Descr::Animation_Nonexistent &) {
							try {
								anim_idx = csinf.becomes->get_animation("unoccupied");
							} catch (Map_Object_Descr::Animation_Nonexistent) {
								anim_idx = csinf.becomes->get_animation("idle");
							}
						}
						const Animation& anim = g_gr->animations().get_animation(anim_idx);
						const size_t nr_frames = anim.nr_frames();
						uint32_t cur_frame =
							csinf.totaltime ? csinf.completedtime * nr_frames / csinf.totaltime : 0;
						uint32_t tanim = cur_frame * FRAME_LENGTH;

						const uint16_t w = anim.width();
						const uint16_t h = anim.height();
						uint32_t lines = h * csinf.completedtime * nr_frames;
						if (csinf.totaltime)
							lines /= csinf.totaltime;
						assert(h * cur_frame <= lines);
						lines -= h * cur_frame;

						if (cur_frame) // not the first frame
							// draw the prev frame from top to where next image will be drawing
							m_dst->drawanimrect
								(pos[F], anim_idx, tanim - FRAME_LENGTH, owner, Rect(Point(0, 0), w, h - lines));
						else if (csinf.was) {
							// Is the first frame, but there was another building here before,
							// get its last build picture and draw it instead.
							uint32_t a;
							try {
								a = csinf.was->get_animation("unoccupied");
							} catch (Map_Object_Descr::Animation_Nonexistent &) {
								a = csinf.was->get_animation("idle");
							}
							m_dst->drawanimrect
								(pos[F], a, tanim - FRAME_LENGTH, owner, Rect(Point(0, 0), w, h - lines));
						}
						assert(lines <= h);
						m_dst->drawanimrect(pos[F], anim_idx, tanim, owner, Rect(Point(0, h - lines), w, lines));
					} else if (upcast(const Building_Descr, building, map_object_descr)) {
						// this is a building therefore we either draw unoccupied or idle animation
						uint32_t pic;
						try {
							pic = building->get_animation("unoccupied");
						} catch (Map_Object_Descr::Animation_Nonexistent &) {
							pic = building->get_animation("idle");
						}
						m_dst->drawanim(pos[F], pic, 0, owner);
					} else if (const uint32_t pic = map_object_descr->main_animation()) {
						m_dst->drawanim(pos[F], pic, 0, owner);
					} else if (map_object_descr == &Widelands::g_flag_descr) {
						m_dst->drawanim(pos[F], owner->flag_anim(), 0, owner);
					}
				}
			}

			{
				// Render overlays on the node
				Overlay_Manager::Overlay_Info overlay_info[MAX_OVERLAYS_PER_NODE];

				const Overlay_Manager::Overlay_Info * const end =
					overlay_info
					+
					map.overlay_manager().get_overlays(coords[F], overlay_info);

				for
					(const Overlay_Manager::Overlay_Info * it = overlay_info;
					 it < end;
					 ++it)
					m_dst->blit(pos[F] - it->hotspot, it->pic);
			}

			{
				// Render overlays on the R triangle
				Overlay_Manager::Overlay_Info overlay_info[MAX_OVERLAYS_PER_TRIANGLE];
				Overlay_Manager::Overlay_Info const * end =
					overlay_info
					+
					map.overlay_manager().get_overlays
							 	(TCoords<>(coords[F], TCoords<>::R), overlay_info);

				Point tripos
					((pos[F].x + pos[R].x + pos[BR].x) / 3,
					 (pos[F].y + pos[R].y + pos[BR].y) / 3);

				for
					(Overlay_Manager::Overlay_Info const * it = overlay_info;
					 it < end;
					 ++it)
					m_dst->blit(tripos - it->hotspot, it->pic);
			}

			{
				// Render overlays on the D triangle
				Overlay_Manager::Overlay_Info overlay_info[MAX_OVERLAYS_PER_TRIANGLE];
				Overlay_Manager::Overlay_Info const * end =
					overlay_info
					+
					map.overlay_manager().get_overlays
							 	(TCoords<>(coords[F], TCoords<>::D), overlay_info);

				Point tripos
					((pos[F].x + pos[BL].x + pos[BR].x) / 3,
					 (pos[F].y + pos[BL].y + pos[BR].y) / 3);

				for
					(Overlay_Manager::Overlay_Info const * it = overlay_info;
					 it < end;
					 ++it)
					m_dst->blit(tripos - it->hotspot, it->pic);
			}
		}
	}
}
