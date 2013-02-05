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

#include "gamerenderer_sdl.h"

#include "logic/field.h"
#include "logic/map.h"
#include "logic/player.h"

#include "graphic/rendertarget.h"

#include "wui/overlay_manager.h"

#include "terrain_sdl.h"


using namespace Widelands;

///This is used by rendermap to calculate the brightness of the terrain.
inline static Sint8 node_brightness
	(Widelands::Time   const gametime,
	 Widelands::Time   const last_seen,
	 Widelands::Vision const vision,
	 int8_t                  result)
{
	if      (vision == 0)
		result = -128;
	else if (vision == 1) {
		assert(last_seen <= gametime);
		Widelands::Duration const time_ago = gametime - last_seen;
		result =
			static_cast<Sint16>
			(((static_cast<Sint16>(result) + 128) >> 1)
			 *
			 (1.0 + (time_ago < 45000 ? expf(-8.46126929e-5 * time_ago) : 0)))
			-
			128;
	}

	return result;
}


#define RENDERMAP_INITIALIZATIONS                                             \
   viewofs -= dst.get_offset();                                                       \
                                                                              \
   Map                   const & map             = egbase.map();              \
   Widelands::World      const & world           = map.world();               \
   Overlay_Manager       const & overlay_manager = map.get_overlay_manager(); \
   uint32_t const                mapwidth        = map.get_width();           \
   int32_t minfx, minfy;                                                      \
   int32_t maxfx, maxfy;                                                      \
                                                                              \
   /* hack to prevent negative numbers */                                     \
   minfx = (viewofs.x + (TRIANGLE_WIDTH >> 1)) / TRIANGLE_WIDTH - 1;          \
                                                                              \
   minfy = viewofs.y / TRIANGLE_HEIGHT;                                       \
   maxfx = (viewofs.x + (TRIANGLE_WIDTH >> 1) + dst.get_rect().w) / TRIANGLE_WIDTH;   \
   maxfy = (viewofs.y + dst.get_rect().h) / TRIANGLE_HEIGHT;                          \
   maxfx +=  1; /* because of big buildings */                                \
   maxfy += 10; /* because of heights */                                      \
                                                                              \
   int32_t dx              = maxfx - minfx + 1;                               \
   int32_t dy              = maxfy - minfy + 1;                               \
   int32_t linear_fy       = minfy;                                           \
   bool row_is_forward     = linear_fy & 1;                                   \
   int32_t b_posy          = linear_fy * TRIANGLE_HEIGHT - viewofs.y;         \


/**
 * Loop through fields row by row. For each field, draw ground textures, then
 * roads, then immovables, then bobs, then overlay stuff (build icons etc...)
 */
void GameRendererSDL::rendermap
	(RenderTarget & dst,
	 Widelands::Editor_Game_Base const &       egbase,
	 Widelands::Player           const &       player,
	 Point                                     viewofs)
{
	if (player.see_all()) {
		rendermap(dst, egbase, viewofs);
		return;
	}

	dst.get_surface()->fill_rect(dst.get_rect(), RGBAColor(0, 0, 0, 255));

	RENDERMAP_INITIALIZATIONS;

	const Player::Field * const first_player_field = player.fields();
	Widelands::Time const gametime = egbase.get_gametime();

	while (dy--) {
		const int32_t posy = b_posy;
		b_posy += TRIANGLE_HEIGHT;
		const int32_t linear_fx = minfx;
		FCoords r(Coords(linear_fx, linear_fy));
		FCoords br(Coords(linear_fx - not row_is_forward, linear_fy + 1));
		int32_t r_posx =
			r.x * TRIANGLE_WIDTH
			+
			row_is_forward * (TRIANGLE_WIDTH / 2)
			-
			viewofs.x;
		int32_t br_posx = r_posx - TRIANGLE_WIDTH / 2;

		// Calculate safe (bounded) field coordinates and get field pointers
		map.normalize_coords(r);
		map.normalize_coords(br);
		Widelands::Map_Index  r_index = Map::get_index (r, mapwidth);
		r.field = &map[r_index];
		Widelands::Map_Index br_index = Map::get_index(br, mapwidth);
		br.field = &map[br_index];
		const Player::Field *  r_player_field = first_player_field +  r_index;
		const Player::Field * br_player_field = first_player_field + br_index;
		FCoords tr, f;
		map.get_tln(r, &tr);
		map.get_ln(r, &f);
		Widelands::Map_Index tr_index = tr.field - &map[0];
		const Texture * f_r_texture =
			g_gr->get_maptexture_data
				(world
				 .terrain_descr(first_player_field[f.field - &map[0]].terrains.r)
				 .get_texture());

		uint32_t count = dx;

		while (count--) {
			const FCoords bl = br;
			const Player::Field &  f_player_field =  *r_player_field;
			const Player::Field & bl_player_field = *br_player_field;
			f = r;
			const int32_t f_posx = r_posx, bl_posx = br_posx;
			const Texture & l_r_texture = *f_r_texture;
			move_r(mapwidth, tr, tr_index);
			move_r(mapwidth,  r,  r_index);
			move_r(mapwidth, br, br_index);
			r_player_field  = first_player_field +  r_index;
			br_player_field = first_player_field + br_index;
			r_posx  += TRIANGLE_WIDTH;
			br_posx += TRIANGLE_WIDTH;
			const Texture & tr_d_texture =
				*g_gr->get_maptexture_data
					(world.terrain_descr(first_player_field[tr_index].terrains.d).get_texture());
			const Texture & f_d_texture =
				*g_gr->get_maptexture_data(world.terrain_descr(f_player_field.terrains.d).get_texture());
			f_r_texture =
				g_gr->get_maptexture_data(world.terrain_descr(f_player_field.terrains.r).get_texture());

			uint8_t const roads =
				f_player_field.roads | overlay_manager.get_road_overlay(f);

			Vertex f_vert
				(f_posx, posy - f.field->get_height() * HEIGHT_FACTOR,
				 node_brightness
				 	(gametime, f_player_field.time_node_last_unseen,
				 	 f_player_field.vision, f.field->get_brightness()),
				 0, 0);
			Vertex r_vert
				(r_posx, posy - r.field->get_height() * HEIGHT_FACTOR,
				 node_brightness
				 	(gametime, r_player_field->time_node_last_unseen,
				 	 r_player_field->vision, r.field->get_brightness()),
				 TRIANGLE_WIDTH, 0);
			Vertex bl_vert
				(bl_posx, b_posy - bl.field->get_height() * HEIGHT_FACTOR,
				 node_brightness
				 	(gametime, bl_player_field.time_node_last_unseen,
				 	 bl_player_field.vision, bl.field->get_brightness()),
				 0, 64);
			Vertex br_vert
				(br_posx, b_posy - br.field->get_height() * HEIGHT_FACTOR,
				 node_brightness
				 	(gametime, br_player_field->time_node_last_unseen,
				 	 br_player_field->vision, br.field->get_brightness()),
				 TRIANGLE_WIDTH, 64);

			if (row_is_forward) {
				f_vert.tx += TRIANGLE_WIDTH / 2;
				r_vert.tx += TRIANGLE_WIDTH / 2;
			} else {
				f_vert.tx += TRIANGLE_WIDTH;
				r_vert.tx += TRIANGLE_WIDTH;
				bl_vert.tx += TRIANGLE_WIDTH / 2;
				br_vert.tx += TRIANGLE_WIDTH / 2;
			}

			draw_field //  Render ground
				(dst,
				 f_vert, r_vert, bl_vert, br_vert,
				 roads,
				 tr_d_texture, l_r_texture, f_d_texture, *f_r_texture);
		}

		++linear_fy;
		row_is_forward = not row_is_forward;
	}

	{
		const int32_t dx2        = maxfx - minfx + 1;
		int32_t dy2              = maxfy - minfy + 1;
		int32_t linear_fy2       = minfy;
		bool row_is_forward2     = linear_fy2 & 1;
		int32_t b_posy2          = linear_fy2 * TRIANGLE_HEIGHT - viewofs.y - dst.get_offset().y;

		while (dy2--) {
			const int32_t posy = b_posy2;
			b_posy2 += TRIANGLE_HEIGHT;

			{ //  Draw things on the node.
				const int32_t linear_fx = minfx;
				FCoords r (Coords(linear_fx, linear_fy2));
				FCoords br(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));

				//  Calculate safe (bounded) field coordinates and get field pointers.
				map.normalize_coords(r);
				map.normalize_coords(br);
				Widelands::Map_Index  r_index = Map::get_index (r, mapwidth);
				r.field = &map[r_index];
				Widelands::Map_Index br_index = Map::get_index(br, mapwidth);
				br.field = &map[br_index];
				FCoords tr, f;
				map.get_tln(r, &tr);
				map.get_ln(r, &f);
				bool r_is_border;
				uint8_t f_owner_number = f.field->get_owned_by();   //  do not use if f_vision < 1 -> PPOV
				uint8_t r_owner_number;
				r_is_border = r.field->is_border();                 //  do not use if f_vision < 1 -> PPOV
				r_owner_number = r.field->get_owned_by();           //  do not use if f_vision < 1 -> PPOV
				uint8_t br_owner_number = br.field->get_owned_by(); //  do not use if f_vision < 1 -> PPOV
				Player::Field const * r_player_field = first_player_field + r_index;
				const Player::Field * br_player_field = first_player_field + br_index;
				Widelands::Vision  r_vision =  r_player_field->vision;
				Widelands::Vision br_vision = br_player_field->vision;
				Point r_pos
					(linear_fx * TRIANGLE_WIDTH
					 +
					 row_is_forward2 * (TRIANGLE_WIDTH / 2)
					 -
					 viewofs.x - dst.get_offset().x,
					 posy - r.field->get_height() * HEIGHT_FACTOR);
				Point br_pos
					(r_pos.x - TRIANGLE_WIDTH / 2,
					 b_posy2 - br.field->get_height() * HEIGHT_FACTOR);

				int32_t count = dx2;

				while (count--) {
					f = r;
					const Player::Field & f_player_field = *r_player_field;
					move_r(mapwidth, tr);
					move_r(mapwidth,  r,  r_index);
					move_r(mapwidth, br, br_index);
					r_player_field  = first_player_field +  r_index;
					br_player_field = first_player_field + br_index;

					//  do not use if f_vision < 1 -> PPOV
					const uint8_t tr_owner_number = tr.field->get_owned_by();

					const bool f_is_border = r_is_border;
					const uint8_t l_owner_number = f_owner_number;
					const uint8_t bl_owner_number = br_owner_number;
					f_owner_number = r_owner_number;
					r_is_border = r.field->is_border();         //  do not use if f_vision < 1 -> PPOV
					r_owner_number = r.field->get_owned_by();   //  do not use if f_vision < 1 -> PPOV
					br_owner_number = br.field->get_owned_by(); //  do not use if f_vision < 1 -> PPOV
					Widelands::Vision const  f_vision =  r_vision;
					Widelands::Vision const bl_vision = br_vision;
					r_vision  = player.vision (r_index);
					br_vision = player.vision(br_index);
					const Point f_pos = r_pos, bl_pos = br_pos;
					r_pos = Point(r_pos.x + TRIANGLE_WIDTH, posy - r.field->get_height() * HEIGHT_FACTOR);
					br_pos = Point(br_pos.x + TRIANGLE_WIDTH, b_posy2 - br.field->get_height() * HEIGHT_FACTOR);

					if (1 < f_vision) { // Render stuff that belongs to the node.
						//  Render border markes on and halfway between border nodes.
						if (f_is_border) {
							const Player & owner = egbase.player(f_owner_number);
							uint32_t const anim = owner.frontier_anim();
							dst.drawanim(f_pos, anim, 0, &owner);
							if
								((f_vision | r_vision)
								 and
								 r_owner_number == f_owner_number
								 and
								 ((tr_owner_number == f_owner_number)
								 xor
								 (br_owner_number == f_owner_number)))
								dst.drawanim(middle(f_pos, r_pos), anim, 0, &owner);
							if
								((f_vision | bl_vision)
								 and
								 bl_owner_number == f_owner_number
								 and
								 ((l_owner_number == f_owner_number)
								 xor
								 (br_owner_number == f_owner_number)))
								dst.drawanim(middle(f_pos, bl_pos), anim, 0, &owner);
							if
								((f_vision | br_vision)
								 and
								 br_owner_number == f_owner_number
								 and
								 ((r_owner_number == f_owner_number)
								 xor
								 (bl_owner_number == f_owner_number)))
								dst.drawanim(middle(f_pos, br_pos), anim, 0, &owner);
						}


						// Render bobs
						// TODO - rendering order?
						//  This must be defined somehow. Some bobs have a higher
						//  priority than others. Maybe this priority is a moving
						//  versus non-moving bobs thing? draw_ground implies that
						//  this doesn't render map objects. Are there any overdraw
						//  issues with the current rendering order?

						// Draw Map_Objects hooked to this field
						if (BaseImmovable * const imm = f.field->get_immovable())
							imm->draw(egbase, dst, f, f_pos);
						for
							(Widelands::Bob * bob = f.field->get_first_bob();
							 bob;
							 bob = bob->get_next_bob())
							bob->draw(egbase, dst, f_pos);

						//  Render overlays on nodes.
						Overlay_Manager::Overlay_Info
							overlay_info[MAX_OVERLAYS_PER_NODE];

						const Overlay_Manager::Overlay_Info * const end =
							overlay_info
							+
							overlay_manager.get_overlays(f, overlay_info);

						for
							(const Overlay_Manager::Overlay_Info * it = overlay_info;
							 it < end;
							 ++it)
							dst.blit(f_pos - it->hotspot, it->pic);
					} else if (f_vision == 1) {
						const Player * owner = f_player_field.owner ? egbase.get_player(f_player_field.owner) : 0;
						if (owner) {
							// Draw borders as they stood the last time we saw them
							uint32_t const anim = owner->frontier_anim();
							if (f_player_field.border)
								dst.drawanim(f_pos, anim, 0, owner);
							if (f_player_field.border_r)
								dst.drawanim(middle(f_pos,  r_pos), anim, 0, owner);
							if (f_player_field.border_br)
								dst.drawanim(middle(f_pos, bl_pos), anim, 0, owner);
							if (f_player_field.border_bl)
								dst.drawanim(middle(f_pos, br_pos), anim, 0, owner);
						}
						if
							(const Map_Object_Descr * const map_object_descr =
							 f_player_field.map_object_descr[TCoords<>::None])
						{
							if
								(const Player::Constructionsite_Information * const csinf =
								 f_player_field.constructionsite[TCoords<>::None])
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
									dst.drawanimrect
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
									dst.drawanimrect
										(f_pos, a, tanim - FRAME_LENGTH, owner, Rect(Point(0, 0), w, h - lines));
								}
								assert(lines <= h);
								dst.drawanimrect(f_pos, anim, tanim, owner, Rect(Point(0, h - lines), w, lines));
							} else if (upcast(const Building_Descr, building, map_object_descr)) {
								// this is a building therefore we either draw unoccupied or idle animation
								uint32_t pic;
								try {
									pic = building->get_animation("unoccupied");
								} catch (Map_Object_Descr::Animation_Nonexistent & e) {
									pic = building->get_animation("idle");
								}
								dst.drawanim(f_pos, pic, 0, owner);
							} else if (const uint32_t pic = map_object_descr->main_animation()) {
								dst.drawanim(f_pos, pic, 0, owner);
							} else if (map_object_descr == &Widelands::g_flag_descr) {
								dst.drawanim(f_pos, owner->flag_anim(), 0, owner);
							}
						}
					}
				}
			}

			if (false) { //  Draw things on the R-triangle (nothing to draw yet).
				const int32_t linear_fx = minfx;
				FCoords r(Coords(linear_fx, linear_fy2));
				FCoords b(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));
				int32_t posx =
					(linear_fx - 1) * TRIANGLE_WIDTH
					+
					(row_is_forward2 + 1) * (TRIANGLE_WIDTH / 2)
					-
					viewofs.x;

				//  Calculate safe (bounded) field coordinates.
				map.normalize_coords(r);
				map.normalize_coords(b);

				//  Get field pointers.
				r.field = &map[Map::get_index(r, mapwidth)];
				b.field = &map[Map::get_index(b, mapwidth)];

				int32_t count = dx2;

				//  One less iteration than for nodes and D-triangles.
				while (--count) {
					const FCoords f = r;
					map.get_rn(r, &r);
					map.get_rn(b, &b);
					posx += TRIANGLE_WIDTH;

					//  FIXME Implement visibility rules for objects on triangles
					//  FIXME when they are used in the game. The only things that
					//  FIXME are drawn on triangles now (except the ground) are
					//  FIXME overlays for the editor terrain tool, and the editor
					//  FIXME does not need visibility rules.
					{ //  FIXME Visibility check here.
						Overlay_Manager::Overlay_Info overlay_info
							[MAX_OVERLAYS_PER_TRIANGLE];
						const Overlay_Manager::Overlay_Info & overlay_info_end = *
							(overlay_info
							 +
							 overlay_manager.get_overlays
							 	(TCoords<>(f, TCoords<>::R), overlay_info));

						for
							(const Overlay_Manager::Overlay_Info * it = overlay_info;
							 it < &overlay_info_end;
							 ++it)
							dst.blit
								(Point
								 	(posx,
								 	 posy
								 	 +
								 	 (TRIANGLE_HEIGHT
								 	  -
								 	  (f.field->get_height()
								 	   +
								 	   r.field->get_height()
								 	   +
								 	   b.field->get_height())
								 	  *
								 	  HEIGHT_FACTOR)
								 	 /
								 	 3)
								 -
								 it->hotspot,
								 it->pic);
					}
				}
			}

			if (false) { //  Draw things on the D-triangle (nothing to draw yet).
				const int32_t linear_fx = minfx;
				FCoords f(Coords(linear_fx - 1, linear_fy2));
				FCoords br
					(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));
				int32_t posx =
					(linear_fx - 1) * TRIANGLE_WIDTH
					+
					row_is_forward2 * (TRIANGLE_WIDTH / 2)
					-
					viewofs.x;

				//  Calculate safe (bounded) field coordinates.
				map.normalize_coords(f);
				map.normalize_coords(br);

				//  Get field pointers.
				f.field  = &map[Map::get_index(f,  mapwidth)];
				br.field = &map[Map::get_index(br, mapwidth)];

				int32_t count = dx2;

				while (count--) {
					const FCoords bl = br;
					map.get_rn(f, &f);
					map.get_rn(br, &br);
					posx += TRIANGLE_WIDTH;

					{ //  FIXME Visibility check here.
						Overlay_Manager::Overlay_Info overlay_info
							[MAX_OVERLAYS_PER_TRIANGLE];
						Overlay_Manager::Overlay_Info const * const overlay_info_end
							=
							overlay_info
							+
							overlay_manager.get_overlays
								(TCoords<>(f, TCoords<>::D), overlay_info);

						for
							(const Overlay_Manager::Overlay_Info * it = overlay_info;
							 it < overlay_info_end;
							 ++it)
							dst.blit
								(Point
								 	(posx,
								 	 posy
								 	 +
								 	 ((TRIANGLE_HEIGHT * 2)
								 	  -
								 	  (f.field->get_height()
								 	   +
								 	   bl.field->get_height()
								 	   +
								 	   br.field->get_height())
								 	  *
								 	  HEIGHT_FACTOR)
								 	 /
								 	 3)
								 -
								 it->hotspot,
								 it->pic);
					}
				}
			}

			++linear_fy2;
			row_is_forward2 = not row_is_forward2;
		}
	}

	g_gr->reset_texture_animation_reminder();
}

void GameRendererSDL::rendermap
	(RenderTarget & dst,
	 Widelands::Editor_Game_Base const &       egbase,
	 Point                                     viewofs)
{
	RENDERMAP_INITIALIZATIONS;

	while (dy--) {
		const int32_t posy = b_posy;
		b_posy += TRIANGLE_HEIGHT;
		const int32_t linear_fx = minfx;
		FCoords r(Coords(linear_fx, linear_fy));
		FCoords br(Coords(linear_fx - not row_is_forward, linear_fy + 1));
		int32_t r_posx =
			r.x * TRIANGLE_WIDTH
			+
			row_is_forward * (TRIANGLE_WIDTH / 2)
			-
			viewofs.x;
		int32_t br_posx = r_posx - TRIANGLE_WIDTH / 2;

		// Calculate safe (bounded) field coordinates and get field pointers
		map.normalize_coords(r);
		map.normalize_coords(br);
		Widelands::Map_Index  r_index = Map::get_index (r, mapwidth);
		r.field = &map[r_index];
		Widelands::Map_Index br_index = Map::get_index(br, mapwidth);
		br.field = &map[br_index];
		FCoords tr, f;
		map.get_tln(r, &tr);
		map.get_ln(r, &f);
		const Texture * f_r_texture =
			g_gr->get_maptexture_data
				(world.terrain_descr(f.field->terrain_r()).get_texture());

		uint32_t count = dx;

		while (count--) {
			const FCoords bl = br;
			f = r;
			const int32_t f_posx = r_posx, bl_posx = br_posx;
			const Texture & l_r_texture = *f_r_texture;
			move_r(mapwidth, tr);
			move_r(mapwidth,  r,  r_index);
			move_r(mapwidth, br, br_index);
			r_posx  += TRIANGLE_WIDTH;
			br_posx += TRIANGLE_WIDTH;
			const Texture & tr_d_texture =
				*g_gr->get_maptexture_data
					(world.terrain_descr(tr.field->terrain_d()).get_texture());
			const Texture & f_d_texture =
				*g_gr->get_maptexture_data
					(world.terrain_descr(f.field->terrain_d()).get_texture());
			f_r_texture =
				g_gr->get_maptexture_data
					(world.terrain_descr(f.field->terrain_r()).get_texture());

			const uint8_t roads =
				f.field->get_roads() | overlay_manager.get_road_overlay(f);

			Vertex f_vert
				(f_posx, posy - f.field->get_height() * HEIGHT_FACTOR,
				 f.field->get_brightness(),
				 0, 0);
			Vertex r_vert
				(r_posx, posy - r.field->get_height() * HEIGHT_FACTOR,
				 r.field->get_brightness(),
				 TRIANGLE_WIDTH, 0);
			Vertex bl_vert
				(bl_posx, b_posy - bl.field->get_height() * HEIGHT_FACTOR,
				 bl.field->get_brightness(),
				 0, 64);
			Vertex br_vert
				(br_posx, b_posy - br.field->get_height() * HEIGHT_FACTOR,
				 br.field->get_brightness(),
				 TRIANGLE_WIDTH, 64);

			if (row_is_forward) {
				f_vert.tx += TRIANGLE_WIDTH / 2;
				r_vert.tx += TRIANGLE_WIDTH / 2;
			} else {
				bl_vert.tx -= TRIANGLE_WIDTH / 2;
				br_vert.tx -= TRIANGLE_WIDTH / 2;
			}

			draw_field //  Render ground
				(dst,
				 f_vert, r_vert, bl_vert, br_vert,
				 roads,
				 tr_d_texture, l_r_texture, f_d_texture, *f_r_texture);
		}

		++linear_fy;
		row_is_forward = not row_is_forward;
	}

	{
		const int32_t dx2 = maxfx - minfx + 1;
		int32_t dy2 = maxfy - minfy + 1;
		int32_t linear_fy2 = minfy;
		bool row_is_forward2 = linear_fy2 & 1;
		int32_t b_posy2 = linear_fy2 * TRIANGLE_HEIGHT - viewofs.y;

		while (dy2--) {
			const int32_t posy = b_posy2;
			b_posy2 += TRIANGLE_HEIGHT;

			{ //  Draw things on the node.
				const int32_t linear_fx = minfx;
				FCoords r(Coords(linear_fx, linear_fy2));
				FCoords br
					(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));

				//  Calculate safe (bounded) field coordinates and get field
				//  pointers.
				map.normalize_coords(r);
				map.normalize_coords(br);
				Widelands::Map_Index  r_index = Map::get_index (r, mapwidth);
				r.field = &map[r_index];
				Widelands::Map_Index br_index = Map::get_index(br, mapwidth);
				br.field = &map[br_index];
				FCoords tr, f;
				map.get_tln(r, &tr);
				map.get_ln(r, &f);
				bool r_is_border;
				uint8_t f_owner_number = f.field->get_owned_by();
				uint8_t r_owner_number;
				r_is_border = r.field->is_border();
				r_owner_number = r.field->get_owned_by();
				uint8_t br_owner_number = br.field->get_owned_by();
				Point r_pos
					(linear_fx * TRIANGLE_WIDTH
					 +
					 row_is_forward2 * (TRIANGLE_WIDTH / 2)
					 -
					 viewofs.x,
					 posy - r.field->get_height() * HEIGHT_FACTOR);
				Point br_pos
					(r_pos.x - TRIANGLE_WIDTH / 2,
					 b_posy2 - br.field->get_height() * HEIGHT_FACTOR);

				int32_t count = dx2;

				while (count--) {
					f = r;
					move_r(mapwidth, tr);
					move_r(mapwidth,  r,  r_index);
					move_r(mapwidth, br, br_index);
					const uint8_t tr_owner_number = tr.field->get_owned_by();
					const bool f_is_border = r_is_border;
					const uint8_t l_owner_number = f_owner_number;
					const uint8_t bl_owner_number = br_owner_number;
					f_owner_number = r_owner_number;
					r_is_border = r.field->is_border();
					r_owner_number = r.field->get_owned_by();
					br_owner_number = br.field->get_owned_by();
					const Point f_pos = r_pos, bl_pos = br_pos;
					r_pos = Point
						(r_pos.x + TRIANGLE_WIDTH,
						 posy - r.field->get_height() * HEIGHT_FACTOR);
					br_pos = Point
						(br_pos.x + TRIANGLE_WIDTH,
						 b_posy2 - br.field->get_height() * HEIGHT_FACTOR);

					//  Render border markes on and halfway between border nodes.
					if (f_is_border) {
						const Player & owner = egbase.player(f_owner_number);
						uint32_t const anim = owner.frontier_anim();
						dst.drawanim(f_pos, anim, 0, &owner);
						if
							(r_owner_number == f_owner_number
							 and
							 ((tr_owner_number == f_owner_number)
							  xor
							  (br_owner_number == f_owner_number)))
							dst.drawanim(middle(f_pos, r_pos), anim, 0, &owner);
						if
							(bl_owner_number == f_owner_number
							 and
							 ((l_owner_number == f_owner_number)
							  xor
							  (br_owner_number == f_owner_number)))
							dst.drawanim(middle(f_pos, bl_pos), anim, 0, &owner);
						if
							(br_owner_number == f_owner_number
							 and
							 ((r_owner_number == f_owner_number)
							  xor
							  (bl_owner_number == f_owner_number)))
							dst.drawanim(middle(f_pos, br_pos), anim, 0, &owner);
					}

					{ // Render stuff that belongs to the node.

						// Render bobs
						// TODO - rendering order?
						//  This must be defined somehow. Some bobs have a higher
						//  priority than others. Maybe this priority is a moving
						//  versus non-moving bobs thing? draw_ground implies that
						//  this doesn't render map objects. Are there any overdraw
						//  issues with the current rendering order?

						// Draw Map_Objects hooked to this field
						if (BaseImmovable * const imm = f.field->get_immovable())
							imm->draw(egbase, dst, f, f_pos);
						for
							(Widelands::Bob * bob = f.field->get_first_bob();
							 bob;
							 bob = bob->get_next_bob())
							bob->draw(egbase, dst, f_pos);

						//  Render overlays on nodes.
						Overlay_Manager::Overlay_Info
							overlay_info[MAX_OVERLAYS_PER_NODE];

						const Overlay_Manager::Overlay_Info * const end =
							overlay_info
							+
							overlay_manager.get_overlays(f, overlay_info);

						for
							(const Overlay_Manager::Overlay_Info * it = overlay_info;
							 it < end;
							 ++it)
							dst.blit(f_pos - it->hotspot, it->pic);
					}
				}
			}

			{ //  Draw things on the R-triangle.
				const int32_t linear_fx = minfx;
				FCoords r(Coords(linear_fx, linear_fy2));
				FCoords b
					(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));
				int32_t posx =
					(linear_fx - 1) * TRIANGLE_WIDTH
					+
					(row_is_forward2 + 1) * (TRIANGLE_WIDTH / 2)
					-
					viewofs.x;

				//  Calculate safe (bounded) field coordinates.
				map.normalize_coords(r);
				map.normalize_coords(b);

				//  Get field pointers.
				r.field = &map[Map::get_index(r, mapwidth)];
				b.field = &map[Map::get_index(b, mapwidth)];

				int32_t count = dx2;

				//  One less iteration than for nodes and D-triangles.
				while (--count) {
					const FCoords f = r;
					map.get_rn(r, &r);
					map.get_rn(b, &b);
					posx += TRIANGLE_WIDTH;

					{
						Overlay_Manager::Overlay_Info overlay_info
							[MAX_OVERLAYS_PER_TRIANGLE];
						Overlay_Manager::Overlay_Info const & overlay_info_end =
							*
							(overlay_info
							 +
							 overlay_manager.get_overlays
							 	(TCoords<>(f, TCoords<>::R), overlay_info));

						for
							(Overlay_Manager::Overlay_Info const * it = overlay_info;
							 it < &overlay_info_end;
							 ++it)
							dst.blit
								(Point
								 	(posx,
								 	 posy
								 	 +
								 	 (TRIANGLE_HEIGHT
								 	  -
								 	  (f.field->get_height()
								 	   +
								 	   r.field->get_height()
								 	   +
								 	   b.field->get_height())
								 	  *
								 	  HEIGHT_FACTOR)
								 	 /
								 	 3)
								 -
								 it->hotspot,
								 it->pic);
					}
				}
			}

			{ //  Draw things on the D-triangle.
				const int32_t linear_fx = minfx;
				FCoords f(Coords(linear_fx - 1, linear_fy2));
				FCoords br(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));
				int32_t posx =
					(linear_fx - 1) * TRIANGLE_WIDTH
					+
					row_is_forward2 * (TRIANGLE_WIDTH / 2)
					-
					viewofs.x;

				//  Calculate safe (bounded) field coordinates.
				map.normalize_coords(f);
				map.normalize_coords(br);

				//  Get field pointers.
				f.field  = &map[Map::get_index(f,  mapwidth)];
				br.field = &map[Map::get_index(br, mapwidth)];

				int32_t count = dx2;

				while (count--) {
					const FCoords bl = br;
					map.get_rn(f, &f);
					map.get_rn(br, &br);
					posx += TRIANGLE_WIDTH;

					{
						Overlay_Manager::Overlay_Info overlay_info
							[MAX_OVERLAYS_PER_TRIANGLE];
						const Overlay_Manager::Overlay_Info & overlay_info_end = *
							(overlay_info
							 +
							 overlay_manager.get_overlays
							 	(TCoords<>(f, TCoords<>::D), overlay_info));

						for
							(const Overlay_Manager::Overlay_Info * it = overlay_info;
							 it < &overlay_info_end;
							 ++it)
							dst.blit
								(Point
								 	(posx,
								 	 posy
								 	 +
								 	 ((TRIANGLE_HEIGHT * 2)
								 	  -
								 	  (f.field->get_height()
								 	   +
								 	   bl.field->get_height()
								 	   +
								 	   br.field->get_height())
								 	  *
								 	  HEIGHT_FACTOR)
								 	 /
								 	 3)
								 -
								 it->hotspot,
								 it->pic);
					}
				}
			}

			++linear_fy2;
			row_is_forward2 = not row_is_forward2;
		}
	}

	g_gr->reset_texture_animation_reminder();
}


/**
 * Draw ground textures and roads for the given parallelogram (two triangles)
 * into the bitmap.
 *
 * Vertices:
 *   - f_vert vertex of the field
 *   - r_vert vertex right of the field
 *   - bl_vert vertex bottom left of the field
 *   - br_vert vertex bottom right of the field
 *
 * Textures:
 *   - f_r_texture Terrain of the triangle right of the field
 *   - f_d_texture Terrain of the triangle under of the field
 *   - tr_d_texture Terrain of the triangle top of the right triangle ??
 *   - l_r_texture Terrain of the triangle left of the down triangle ??
 *
 *             (tr_d)
 *
 *       (f) *------* (r)
 *          / \  r /
 *  (l_r)  /   \  /
 *        /  d  \/
 *  (bl) *------* (br)
 */
void GameRendererSDL::draw_field
	(RenderTarget & dst,
	 Vertex  const &  f_vert,
	 Vertex  const &  r_vert,
	 Vertex  const & bl_vert,
	 Vertex  const & br_vert,
	 uint8_t         roads,
	 Texture const & tr_d_texture,
	 Texture const &  l_r_texture,
	 Texture const &  f_d_texture,
	 Texture const &  f_r_texture)
{
	upcast(SDLSurface, sdlsurf, dst.get_surface());
	if (sdlsurf)
	{
		sdlsurf->set_subwin(dst.get_rect());
		switch (sdlsurf->format().BytesPerPixel) {
		case 2:
			draw_field_int<Uint16>
				(*sdlsurf,
				 f_vert, r_vert, bl_vert, br_vert,
				 roads,
				 tr_d_texture, l_r_texture, f_d_texture, f_r_texture);
			break;
		case 4:
			draw_field_int<Uint32>
				(*sdlsurf,
				 f_vert, r_vert, bl_vert, br_vert,
				 roads,
				 tr_d_texture, l_r_texture, f_d_texture, f_r_texture);
			break;
		default:
			assert(false);
			break;
		}
		sdlsurf->unset_subwin();
	}
}
