/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "rendertarget.h"

#include "graphic.h"
#include "wui/mapviewpixelconstants.h"
#include "overlay_manager.h"
#include "surface.h"
#include "tribe.h"
#include "vertex.h"

#include "log.h"

using Widelands::BaseImmovable;
using Widelands::Coords;
using Widelands::FCoords;
using Widelands::Map;
using Widelands::Map_Object_Descr;
using Widelands::Player;
using Widelands::TCoords;

/**
 * Build a render target for the given bitmap.
 * \note The bitmap will not be owned by the renderer, i.e. it won't be
 * deleted by the destructor.
 */
RenderTarget::RenderTarget(Surface * const bmp)
{
	m_surface = bmp;

	reset();
}


/**
 * Sets an arbitrary drawing window.
 */
void RenderTarget::set_window(Rect const & rc, Point const & ofs)
{
	m_rect = rc;
	m_offset = ofs;

	// safeguards clipping against the bitmap itself

	if (m_rect.x < 0) {
		m_offset.x += m_rect.x;
		m_rect.w = std::max(static_cast<int32_t>(m_rect.w) + m_rect.x, 0);
		m_rect.x = 0;
	}

	if (m_rect.x + m_rect.w > m_surface->get_w())
		m_rect.w =
			std::max(static_cast<int32_t>(m_surface->get_w()) - m_rect.x, 0);

	if (m_rect.y < 0) {
		m_offset.y += m_rect.y;
		m_rect.h = std::max(static_cast<int32_t>(m_rect.h) + m_rect.y, 0);
		m_rect.y = 0;
	}

	if (m_rect.y + m_rect.h > m_surface->get_h())
		m_rect.h =
			std::max(static_cast<int32_t>(m_surface->get_h()) - m_rect.y, 0);
}

/**
 * Builds a subwindow. rc is relative to the current drawing window. The
 * subwindow will be clipped appropriately.
 *
 * The previous window state is returned in previous and prevofs.
 *
 * Returns false if the subwindow is invisible. In that case, the window state
 * is not changed at all. Otherwise, the function returns true.
 */
bool RenderTarget::enter_window
	(Rect const & rc, Rect * const previous, Point * const prevofs)
{
	Rect newrect = rc;

	if (clip(newrect)) {
		if (previous)
			*previous = m_rect;
		if (prevofs)
			*prevofs = m_offset;

		// Apply the changes
		m_offset = rc - (newrect - m_rect - m_offset);
		m_rect = newrect;

		return true;
	} else return false;
}

/**
 * Returns the true size of the render target (ignoring the window settings).
 */
int32_t RenderTarget::get_w() const
{
	return m_surface->get_w();
}

/**
 * Returns the true size of the render target (ignoring the window settings).
 */
int32_t RenderTarget::get_h() const
{
	return m_surface->get_h();
}

/**
 * This functions draws a (not horizontal or vertical)
 * line in the target, using Bresenham's algorithm
 *
 * This function is still quite slow, since it draws
 * every pixel as a rectangle. So use it with care
 */
void RenderTarget::draw_line
	(int32_t const x1, int32_t const y1, int32_t const x2, int32_t const y2,
	 RGBColor const color)
{
#if HAS_OPENGL
	//use opengl drawing if available
	if (g_opengl) {
		glBegin(GL_LINES);
		glColor3f
			((color.r() / 256.0f),
			 (color.g() / 256.0f),
			 (color.b() / 256.0f));
		glVertex2f(x1, y1);
		glVertex2f(x2, y2);
		glEnd();
		return;
	}
#endif

	int32_t dx = x2 - x1;      /* the horizontal distance of the line */
	int32_t dy = y2 - y1;      /* the vertical distance of the line */
	const uint32_t dxabs = abs(dx);
	const uint32_t dyabs = abs(dy);
	int32_t sdx = dx < 0 ? -1 : 1;
	int32_t sdy = dy < 0 ? -1 : 1;
	uint32_t x = dyabs / 2;
	uint32_t y = dxabs / 2;
	Point p(x1, y1);

	draw_rect(Rect(p, 1, 1), color);

	if (dxabs >= dyabs)
		for (uint32_t i = 0; i < dxabs; ++i) {
			//  the line is more horizontal than vertical
			y += dyabs;

			if (y >= dxabs) {
				y   -= dxabs;
				p.y += sdy;
			}

			p.x += sdx;
			draw_rect(Rect(p, 1, 1), color);
		}
	else
		for (uint32_t i = 0; i < dyabs; ++i) {
			//  the line is more vertical than horizontal
			x += dxabs;

			if (x >= dyabs) {
				x   -= dyabs;
				p.x += sdx;
			}

			p.y += sdy;
			draw_rect(Rect(p, 1, 1), color);
		}
}

/**
 * Clip against window and pass those primitives along to the bitmap.
 */
void RenderTarget::draw_rect(Rect r, const RGBColor clr)
{
	if (clip(r))
		m_surface->draw_rect(r, clr);
}

void RenderTarget::fill_rect(Rect r, const RGBColor clr)
{
	if (clip(r))
		m_surface->fill_rect(r, clr);
}

void RenderTarget::brighten_rect(Rect r, const int32_t factor)
{
	if (clip(r))
		m_surface->brighten_rect(r, factor);
}

void RenderTarget::clear()
{
	if
		(not m_rect.x and not m_rect.y
		 and
		 m_rect.w == m_surface->get_w() and m_rect.h == m_surface->get_h())
		m_surface->clear();
	else m_surface->fill_rect(m_rect, RGBColor(0, 0, 0));
}

/**
 * Blits a blitsource into this bitmap
 */
void RenderTarget::blit(const Point dst, const PictureID picture)
{
	if (Surface * const src = g_gr->get_picture_surface(picture))
		doblit(dst, src, Rect(Point(0, 0), src->get_w(), src->get_h()));
}

void RenderTarget::blitrect
	(Point const dst, PictureID const picture, Rect const srcrc)
{
	assert(0 <= srcrc.x);
	assert(0 <= srcrc.y);

	if (Surface * const src = g_gr->get_picture_surface(picture))
		doblit(dst, src, srcrc);
}

/**
 * Fill the given rectangle with the given picture.
 *
 * The pixel from ofs inside picture is placed at the top-left corner of
 * the filled rectangle.
 */
void RenderTarget::tile(Rect r, PictureID const picture, Point ofs)
{
	Surface * const src = g_gr->get_picture_surface(picture);

	if (!src)
		return;

	if (clip(r)) {

		// Make sure the offset is within bounds
		ofs.x = ofs.x % src->get_w();

		if (ofs.x < 0)
			ofs.x += src->get_w();

		ofs.y = ofs.y % src->get_h();

		if (ofs.y < 0)
			ofs.y += src->get_h();

		// Blit the picture into the rectangle
		uint32_t ty = 0;

		while (ty < r.h) {
			uint32_t tx = 0;
			int32_t tofsx = ofs.x;
			Rect srcrc;

			srcrc.y = ofs.y;
			srcrc.h = src->get_h() - ofs.y;

			if (ty + srcrc.h > r.h)
				srcrc.h = r.h - ty;

			while (tx < r.w) {
				srcrc.x = tofsx;
				srcrc.w = src->get_w() - tofsx;

				if (tx + srcrc.w > r.w)
					srcrc.w = r.w - tx;

				m_surface->blit(r + Point(tx, ty), src, srcrc);

				tx += srcrc.w;

				tofsx = 0;
			}

			ty += srcrc.h;
			ofs.y = 0;
		}
	}
}


inline static Sint8 node_brightness
	(Widelands::Time   gametime,
	 Widelands::Time   last_seen,
	 Widelands::Vision vision,
	 Sint8             result)
__attribute__((const));
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


#define RENDERMAP_INITIALIZANTONS                                             \
   viewofs -= m_offset;                                                       \
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
   maxfx = (viewofs.x + (TRIANGLE_WIDTH >> 1) + m_rect.w) / TRIANGLE_WIDTH;   \
   maxfy = (viewofs.y + m_rect.h) / TRIANGLE_HEIGHT;                          \
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
void RenderTarget::rendermap
	(Widelands::Editor_Game_Base const &       egbase,
	 Player                      const &       player,
	 Point                                     viewofs)
{
	if (player.see_all())
		return rendermap(egbase, viewofs);

	RENDERMAP_INITIALIZANTONS;

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
			const FCoords l = f, bl = br;
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
					(world.terrain_descr(first_player_field[tr_index].terrains.d)
					 .get_texture());
			const Texture & f_d_texture =
				*g_gr->get_maptexture_data
					(world.terrain_descr(f_player_field.terrains.d).get_texture());
			f_r_texture =
				g_gr->get_maptexture_data
					(world.terrain_descr(f_player_field.terrains.r).get_texture());

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
				bl_vert.tx -= TRIANGLE_WIDTH / 2;
				br_vert.tx -= TRIANGLE_WIDTH / 2;
			}

			m_surface->draw_field //  Render ground
				(m_rect,
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
		bool row_is_forward2 = linear_fy2 & 1;
		int32_t b_posy2          = linear_fy2 * TRIANGLE_HEIGHT - viewofs.y;

		while (dy2--) {
			const int32_t posy = b_posy2;
			b_posy2 += TRIANGLE_HEIGHT;

			{ //  Draw things on the node.
				const int32_t linear_fx = minfx;
				FCoords r(Coords(linear_fx, linear_fy2));
				FCoords br
					(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));

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
				bool r_is_border;
				uint8_t f_owner_number = f.field->get_owned_by(); //  FIXME PPoV
				uint8_t r_owner_number;
				r_is_border = r.field->is_border(); //  FIXME PPoV
				r_owner_number = r.field->get_owned_by(); //  FIXME PPoV
				uint8_t br_owner_number = br.field->get_owned_by(); //  FIXME PPoV
				const Player::Field * r_player_field = first_player_field + r_index;
				const Player::Field * br_player_field =
					first_player_field + br_index;
				Widelands::Vision  r_vision =  r_player_field->vision;
				Widelands::Vision br_vision = br_player_field->vision;
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
					const FCoords l = f, bl = br;
					f = r;
					const Player::Field & f_player_field = *r_player_field;
					move_r(mapwidth, tr);
					move_r(mapwidth,  r,  r_index);
					move_r(mapwidth, br, br_index);
					r_player_field  = first_player_field +  r_index;
					br_player_field = first_player_field + br_index;

					//  FIXME PPoV
					const uint8_t tr_owner_number = tr.field->get_owned_by();

					const bool f_is_border = r_is_border;
					const uint8_t l_owner_number = f_owner_number;
					const uint8_t bl_owner_number = br_owner_number;
					f_owner_number = r_owner_number;
					r_is_border = r.field->is_border();         //  FIXME PPoV
					r_owner_number = r.field->get_owned_by();   //  FIXME PPoV
					br_owner_number = br.field->get_owned_by(); //  FIXME PPoV
					Widelands::Vision const  f_vision =  r_vision;
					Widelands::Vision const bl_vision = br_vision;
					r_vision  = player.vision (r_index);
					br_vision = player.vision(br_index);
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
						const uint32_t anim = owner.tribe().get_frontier_anim();
						if (1 < f_vision)
							drawanim(f_pos, anim, 0, &owner);
						if
							((f_vision | r_vision)
							 and
							 r_owner_number == f_owner_number
							 and
							 ((tr_owner_number == f_owner_number)
							  xor
							  (br_owner_number == f_owner_number)))
							drawanim(middle(f_pos, r_pos), anim, 0, &owner);
						if
							((f_vision | bl_vision)
							 and
							 bl_owner_number == f_owner_number
							 and
							 ((l_owner_number == f_owner_number)
							  xor
							  (br_owner_number == f_owner_number)))
							drawanim(middle(f_pos, bl_pos), anim, 0, &owner);
						if
							((f_vision | br_vision)
							 and
							 br_owner_number == f_owner_number
							 and
							 ((r_owner_number == f_owner_number)
							  xor
							  (bl_owner_number == f_owner_number)))
							drawanim(middle(f_pos, br_pos), anim, 0, &owner);
					}

					if (1 < f_vision) { // Render stuff that belongs to the node.

						// Render bobs
						// TODO - rendering order?
						//  This must be defined somehow. Some bobs have a higher
						//  priority than others. Maybe this priority is a moving
						//  versus non-moving bobs thing? draw_ground implies that
						//  this doesn't render map objects. Are there any overdraw
						//  issues with the current rendering order?

						// Draw Map_Objects hooked to this field
						if (BaseImmovable * const imm = f.field->get_immovable())
							imm->draw(egbase, *this, f, f_pos);
						for
							(Widelands::Bob * bob = f.field->get_first_bob();
							 bob;
							 bob = bob->get_next_bob())
							bob->draw(egbase, *this, f_pos);

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
							blit(f_pos - it->hotspot, it->picid);
					} else if (f_vision == 1)
						if
							(const Map_Object_Descr * const map_object_descr =
							 f_player_field.map_object_descr[TCoords<>::None])
						{
							Player const * const owner =
								f_owner_number ? egbase.get_player(f_owner_number) : 0;
							if
								(const uint32_t picid =
								 	map_object_descr->main_animation())
									drawanim(f_pos, picid, 0, owner);
							else if (map_object_descr == &Widelands::g_flag_descr) {
								drawanim
									(f_pos, owner->tribe().get_flag_anim(), 0, owner);
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
							blit
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
								 it->picid);
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
						const Overlay_Manager::Overlay_Info * const overlay_info_end =
							overlay_info
							+
							overlay_manager.get_overlays
								(TCoords<>(f, TCoords<>::D), overlay_info);

						for
							(const Overlay_Manager::Overlay_Info * it = overlay_info;
							 it < overlay_info_end;
							 ++it)
							blit
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
								 it->picid);
					}
				}
			}

			++linear_fy2;
			row_is_forward2 = not row_is_forward2;
		}
	}

	g_gr->reset_texture_animation_reminder();
}


void RenderTarget::rendermap
	(Widelands::Editor_Game_Base const &       egbase,
	 Point                                     viewofs)
{
	RENDERMAP_INITIALIZANTONS;

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
			const FCoords l = f, bl = br;
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

			m_surface->draw_field //  Render ground
				(m_rect,
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
				FCoords br(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));

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
					const FCoords l = f, bl = br;
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
						const uint32_t anim = owner.tribe().get_frontier_anim();
						drawanim(f_pos, anim, 0, &owner);
						if
							(r_owner_number == f_owner_number
							 and
							 ((tr_owner_number == f_owner_number)
							  xor
							  (br_owner_number == f_owner_number)))
							drawanim(middle(f_pos, r_pos), anim, 0, &owner);
						if
							(bl_owner_number == f_owner_number
							 and
							 ((l_owner_number == f_owner_number)
							  xor
							  (br_owner_number == f_owner_number)))
							drawanim(middle(f_pos, bl_pos), anim, 0, &owner);
						if
							(br_owner_number == f_owner_number
							 and
							 ((r_owner_number == f_owner_number)
							  xor
							  (bl_owner_number == f_owner_number)))
							drawanim(middle(f_pos, br_pos), anim, 0, &owner);
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
							imm->draw(egbase, *this, f, f_pos);
						for
							(Widelands::Bob * bob = f.field->get_first_bob();
							 bob;
							 bob = bob->get_next_bob())
							bob->draw(egbase, *this, f_pos);

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
							blit(f_pos - it->hotspot, it->picid);
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
						const Overlay_Manager::Overlay_Info & overlay_info_end = *
							(overlay_info
							 +
							 overlay_manager.get_overlays
							 	(TCoords<>(f, TCoords<>::R), overlay_info));

						for
							(const Overlay_Manager::Overlay_Info * it =
							 overlay_info;
							 it < &overlay_info_end;
							 ++it)
							blit
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
								 it->picid);
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
							blit
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
								 it->picid);
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
 * Renders a minimap into the current window. The field at viewpoint will be
 * in the top-left corner of the window. Flags specifies what information to
 * display (see Minimap_XXX enums).
 *
 * Calculate the field at the top-left corner of the clipping rect
 * The entire clipping rect will be used for drawing.
 */
void RenderTarget::renderminimap
	(Widelands::Editor_Game_Base const &       egbase,
	 Player                      const * const player,
	 Point                               const viewpoint,
	 uint32_t                            const flags)
{
	m_surface->draw_minimap
			(egbase, player, m_rect, viewpoint - m_offset, flags);
}

/**
 * Draws a frame of an animation at the given location
 * Plays sound effect that is registered with this frame (the Sound_Handler
 * decides if the fx really does get played)
 *
 * \param dstx, dsty the on-screen location of the animation hot spot
 * \param animation the animation ID
 * \param time the time, in milliseconds, in the animation
 * \param player the player this object belongs to, for player colour
 * purposes. May be 0 (for example, for world objects).
 *
 * \todo Correctly calculate the stereo position for sound effects
 * \todo The chosen semantics of animation sound effects is problematic:
 * What if the game runs very slowly or very quickly?
 */
void RenderTarget::drawanim
	(Point                dst,
	 uint32_t       const animation,
	 uint32_t       const time,
	 Player const * const player)
{
	AnimationData const & data = *g_anim.get_animation(animation);
	AnimationGfx        & gfx  = *g_gr-> get_animation(animation);

	assert(&data);
	assert(&gfx);

	// Get the frame and its data

	uint32_t const framenumber = (time / data.frametime) % gfx.nr_frames();

	Surface * const frame =
		gfx.get_frame(framenumber, player ? player->player_number() : 0, player);

	dst -= gfx.get_hotspot();

	Rect srcrc(Point(0, 0), frame->get_w(), frame->get_h());

	doblit(dst, frame, srcrc);

	// Look if there's a sound effect registered for this frame and trigger
	// the effect
	uint32_t stereo_position = 128; //  see Sound_Handler::stereo_position()

	g_anim.trigger_soundfx(animation, framenumber, stereo_position);
}

/**
 * Draws a part of a frame of an animation at the given location
 */
void RenderTarget::drawanimrect
	(Point                dst,
	 uint32_t       const animation,
	 uint32_t       const time,
	 Player const * const player,
	 Rect                 srcrc)
{
	AnimationData const * data = g_anim.get_animation(animation);
	if (!data || !g_gr) {
		log("WARNING: Animation %u does not exist\n", animation);
		return;
	}

	// Get the frame and its data
	Surface * const frame = g_gr->get_animation(animation)->get_frame
			((time / data->frametime) % g_gr->nr_frames(animation),
			 player ? player->player_number() : 0,
			 player);

	dst -= g_gr->get_animation(animation)->get_hotspot();

	dst += srcrc;

	doblit(dst, frame, srcrc);
}

/**
 * Called every time before the render target is handed out by the Graphic
 * implementation to start in a neutral state.
 */
void RenderTarget::reset()
{
	m_rect.x = m_rect.y = 0;
	m_rect.w = m_surface->get_w();
	m_rect.h = m_surface->get_h();

	m_offset.x = m_offset.y = 0;
}

/**
 * Offsets r by m_offset and clips r against m_rect.
 *
 * If true is returned, r a valid rectangle that can be used.
 * If false is returned, r may not be used and may be partially modified.
 */
bool RenderTarget::clip(Rect & r) const throw ()
{
	r += m_offset;

	if (r.x < 0) {
		if (r.w <= static_cast<uint32_t>(-r.x))
			return false;

		r.w += r.x;

		r.x = 0;
	}

	if (r.x + r.w > m_rect.w) {
		if (static_cast<int32_t>(m_rect.w) <= r.x)
			return false;
		r.w = m_rect.w - r.x;
	}

	if (r.y < 0) {
		if (r.h <= static_cast<uint32_t>(-r.y))
			return false;
		r.h += r.y;
		r.y = 0;
	}

	if (r.y + r.h > m_rect.h) {
		if (static_cast<int32_t>(m_rect.h) <= r.y)
			return false;
		r.h = m_rect.h - r.y;
	}

	r += m_rect;

	return r.w and r.h;
}

/**
 * Clip against window and source bitmap, then call the Bitmap blit routine.
 */
void RenderTarget::doblit(Point dst, Surface * const src, Rect srcrc)
{
	assert(0 <= srcrc.x);
	assert(0 <= srcrc.y);
	dst += m_offset;

	// Clipping

	if (dst.x < 0) {
		if (srcrc.w <= static_cast<uint32_t>(-dst.x))
			return;

		srcrc.x -= dst.x;

		srcrc.w += dst.x;

		dst.x = 0;
	}

	if (dst.x + srcrc.w > m_rect.w) {
		if (static_cast<int32_t>(m_rect.w) <= dst.x)
			return;
		srcrc.w = m_rect.w - dst.x;
	}

	if (dst.y < 0) {
		if (srcrc.h <= static_cast<uint32_t>(-dst.y))
			return;
		srcrc.y -= dst.y;
		srcrc.h += dst.y;
		dst.y = 0;
	}

	if (dst.y + srcrc.h > m_rect.h) {
		if (static_cast<int32_t>(m_rect.h) <= dst.y)
			return;
		srcrc.h = m_rect.h - dst.y;
	}

	dst += m_rect;

	// Draw it
	m_surface->blit(dst, src, srcrc);
}
