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

#include "gamerenderer.h"

#include "graphic/rendertarget.h"

#include "logic/editor_game_base.h"
#include "logic/player.h"

#include "wui/mapviewpixelfunctions.h"
#include "wui/overlay_manager.h"

#include "upcast.h"

using namespace Widelands;

GameRenderer::GameRenderer()
{
}

GameRenderer::~GameRenderer()
{
}

void GameRenderer::rendermap
	(RenderTarget & dst,
	 Widelands::Editor_Game_Base const &       egbase,
	 Widelands::Player           const &       player,
	 Point                       const &       viewofs)
{
	m_dst = &dst;
	m_dst_offset = -viewofs;
	m_egbase = &egbase;
	m_player = &player;

	draw_wrapper();
}

void GameRenderer::rendermap
	(RenderTarget & dst,
	 Widelands::Editor_Game_Base const & egbase,
	 Point                       const & viewofs)
{
	m_dst = &dst;
	m_dst_offset = -viewofs;
	m_egbase = &egbase;
	m_player = 0;

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
	Map const & map = m_egbase->map();

	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords f = map.get_fcoords(ncoords);
			FCoords r = map.r_n(f);
			FCoords bl = map.bl_n(f);
			FCoords br = map.br_n(f);
			Point f_pos, r_pos, bl_pos, br_pos;
			MapviewPixelFunctions::get_basepix(Coords(fx, fy), f_pos.x, f_pos.y);
			MapviewPixelFunctions::get_basepix(Coords(fx + 1, fy), r_pos.x, r_pos.y);
			MapviewPixelFunctions::get_basepix(Coords(fx + (fy & 1) - 1, fy + 1), bl_pos.x, bl_pos.y);
			MapviewPixelFunctions::get_basepix(Coords(fx + (fy & 1), fy + 1), br_pos.x, br_pos.y);
			f_pos.y -= f.field->get_height() * HEIGHT_FACTOR;
			r_pos.y -= r.field->get_height() * HEIGHT_FACTOR;
			bl_pos.y -= bl.field->get_height() * HEIGHT_FACTOR;
			br_pos.y -= br.field->get_height() * HEIGHT_FACTOR;
			f_pos += m_dst_offset;
			r_pos += m_dst_offset;
			bl_pos += m_dst_offset;
			br_pos += m_dst_offset;

			uint8_t f_owner_number = f.field->get_owned_by();
			uint8_t r_owner_number = r.field->get_owned_by();
			uint8_t bl_owner_number = bl.field->get_owned_by();
			uint8_t br_owner_number = br.field->get_owned_by();
			bool f_isborder = f.field->is_border();
			bool r_isborder = r.field->is_border();
			bool bl_isborder = bl.field->is_border();
			bool br_isborder = br.field->is_border();
			Vision f_vision = 2;
			Vision r_vision = 2;
			Vision bl_vision = 2;
			Vision br_vision = 2;

			if (m_player && !m_player->see_all()) {
				Player::Field const & f_pl = m_player->fields()[map.get_index(f, map.get_width())];
				Player::Field const & r_pl = m_player->fields()[map.get_index(r, map.get_width())];
				Player::Field const & bl_pl = m_player->fields()[map.get_index(bl, map.get_width())];
				Player::Field const & br_pl = m_player->fields()[map.get_index(br, map.get_width())];

				f_vision = f_pl.vision;
				r_vision = r_pl.vision;
				bl_vision = bl_pl.vision;
				br_vision = br_pl.vision;
				if (f_vision == 1) {
					f_owner_number = f_pl.owner;
					f_isborder = f_pl.border;
				}
				if (r_vision == 1) {
					r_owner_number = r_pl.owner;
					r_isborder = r_pl.border;
				}
				if (bl_vision == 1) {
					bl_owner_number = bl_pl.owner;
					bl_isborder = bl_pl.border;
				}
				if (br_vision == 1) {
					br_owner_number = br_pl.owner;
					br_isborder = br_pl.border;
				}
			}

			if (f_isborder) {
				Player const & owner = m_egbase->player(f_owner_number);
				uint32_t const anim = owner.frontier_anim();
				if (f_vision)
					m_dst->drawanim(f_pos, anim, 0, &owner);
				if
					((f_vision || r_vision) &&
					 r_isborder &&
					 (r_owner_number == f_owner_number || !r_owner_number))
					m_dst->drawanim(middle(f_pos, r_pos), anim, 0, &owner);
				if
					((f_vision || bl_vision) &&
					 bl_isborder &&
					 (bl_owner_number == f_owner_number || !bl_owner_number))
					m_dst->drawanim(middle(f_pos, bl_pos), anim, 0, &owner);
				if
					((f_vision || br_vision) &&
					 br_isborder &&
					 (br_owner_number == f_owner_number || !br_owner_number))
					m_dst->drawanim(middle(f_pos, br_pos), anim, 0, &owner);
			}

			if (1 < f_vision) { // Render stuff that belongs to the node.
				if (BaseImmovable * const imm = f.field->get_immovable())
					imm->draw(*m_egbase, *m_dst, f, f_pos);
				for
					(Bob * bob = f.field->get_first_bob();
					 bob;
					 bob = bob->get_next_bob())
					bob->draw(*m_egbase, *m_dst, f_pos);
			} else if (f_vision == 1) {
				Player::Field const & f_pl = m_player->fields()[map.get_index(f, map.get_width())];
				const Player * owner = f_owner_number ? m_egbase->get_player(f_owner_number) : 0;
				if
					(const Map_Object_Descr * const map_object_descr =
						f_pl.map_object_descr[TCoords<>::None])
				{
					if
						(const Player::Constructionsite_Information * const csinf =
						 f_pl.constructionsite[TCoords<>::None])
					{
						// draw the partly finished constructionsite
						uint32_t anim;
						try {
							anim = csinf->becomes->get_animation("build");
						} catch (Map_Object_Descr::Animation_Nonexistent & e) {
							try {
								anim = csinf->becomes->get_animation("unoccupied");
							} catch (Map_Object_Descr::Animation_Nonexistent) {
								anim = csinf->becomes->get_animation("idle");
							}
						}
						const AnimationGfx::Index nr_frames = g_gr->nr_frames(anim);
						uint32_t cur_frame =
							csinf->totaltime ? csinf->completedtime * nr_frames / csinf->totaltime : 0;
						uint32_t tanim = cur_frame * FRAME_LENGTH;
						uint32_t w, h;
						g_gr->get_animation_size(anim, tanim, w, h);
						uint32_t lines = h * csinf->completedtime * nr_frames;
						if (csinf->totaltime)
							lines /= csinf->totaltime;
						assert(h * cur_frame <= lines);
						lines -= h * cur_frame; //  This won't work if pictures have various sizes.

						if (cur_frame) // not the first frame
							// draw the prev frame from top to where next image will be drawing
							m_dst->drawanimrect
								(f_pos, anim, tanim - FRAME_LENGTH, owner, Rect(Point(0, 0), w, h - lines));
						else if (csinf->was) {
							// Is the first frame, but there was another building here before,
							// get its last build picture and draw it instead.
							uint32_t a;
							try {
								a = csinf->was->get_animation("unoccupied");
							} catch (Map_Object_Descr::Animation_Nonexistent & e) {
								a = csinf->was->get_animation("idle");
							}
							m_dst->drawanimrect
								(f_pos, a, tanim - FRAME_LENGTH, owner, Rect(Point(0, 0), w, h - lines));
						}
						assert(lines <= h);
						m_dst->drawanimrect(f_pos, anim, tanim, owner, Rect(Point(0, h - lines), w, lines));
					} else if (upcast(const Building_Descr, building, map_object_descr)) {
						// this is a building therefore we either draw unoccupied or idle animation
						uint32_t pic;
						try {
							pic = building->get_animation("unoccupied");
						} catch (Map_Object_Descr::Animation_Nonexistent & e) {
							pic = building->get_animation("idle");
						}
						m_dst->drawanim(f_pos, pic, 0, owner);
					} else if (const uint32_t pic = map_object_descr->main_animation()) {
						m_dst->drawanim(f_pos, pic, 0, owner);
					} else if (map_object_descr == &Widelands::g_flag_descr) {
						m_dst->drawanim(f_pos, owner->flag_anim(), 0, owner);
					}
				}
			}

			{
				// Render overlays on the node
				Overlay_Manager::Overlay_Info overlay_info[MAX_OVERLAYS_PER_NODE];

				const Overlay_Manager::Overlay_Info * const end =
					overlay_info
					+
					map.overlay_manager().get_overlays(f, overlay_info);

				for
					(const Overlay_Manager::Overlay_Info * it = overlay_info;
					 it < end;
					 ++it)
					m_dst->blit(f_pos - it->hotspot, it->pic);
			}

			{
				// Render overlays on the R triangle
				Overlay_Manager::Overlay_Info overlay_info[MAX_OVERLAYS_PER_TRIANGLE];
				Overlay_Manager::Overlay_Info const * end =
					overlay_info
					+
					map.overlay_manager().get_overlays
							 	(TCoords<>(f, TCoords<>::R), overlay_info);

				Point pos
					((f_pos.x + r_pos.x + br_pos.x) / 3,
					 (f_pos.y + r_pos.y + br_pos.y) / 3);

				for
					(Overlay_Manager::Overlay_Info const * it = overlay_info;
					 it < end;
					 ++it)
					m_dst->blit(pos - it->hotspot, it->pic);
			}

			{
				// Render overlays on the D triangle
				Overlay_Manager::Overlay_Info overlay_info[MAX_OVERLAYS_PER_TRIANGLE];
				Overlay_Manager::Overlay_Info const * end =
					overlay_info
					+
					map.overlay_manager().get_overlays
							 	(TCoords<>(f, TCoords<>::D), overlay_info);

				Point pos
					((f_pos.x + bl_pos.x + br_pos.x) / 3,
					 (f_pos.y + bl_pos.y + br_pos.y) / 3);

				for
					(Overlay_Manager::Overlay_Info const * it = overlay_info;
					 it < end;
					 ++it)
					m_dst->blit(pos - it->hotspot, it->pic);
			}
		}
	}
}
