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

#include "rendertarget.h"

#include "graphic.h"
#include "mapviewpixelconstants.h"
#include "overlay_manager.h"
#include "surface.h"
#include "tribe.h"


/**
 * Build a render target for the given bitmap.
 * \note The bitmap will not be owned by the renderer, i.e. it won't be
 * deleted by the destructor.
 */
RenderTarget::RenderTarget(Surface* bmp)
{
	m_surface = bmp;
	m_ground_surface = 0;

	reset();
}

RenderTarget::~RenderTarget()
{
	delete m_ground_surface;
}

/**
 * Retrieve the current window setting.
 */
void RenderTarget::get_window(Rect* rc, Point* ofs) const
{
	*rc = m_rect;
	*ofs = m_offset;
}

/**
 * Sets an arbitrary drawing window.
 */
void RenderTarget::set_window(const Rect& rc, const Point& ofs)
{
	m_rect = rc;
	m_offset = ofs;

	// safeguards clipping against the bitmap itself

	if (m_rect.x < 0) {
		m_offset.x += m_rect.x;
		m_rect.w = std::max(static_cast<int>(m_rect.w) + m_rect.x, 0);
		m_rect.x = 0;
	}

	if (m_rect.x + m_rect.w > m_surface->get_w())
		m_rect.w =
			std::max(static_cast<int>(m_surface->get_w()) - m_rect.x, 0);

	if (m_rect.y < 0) {
		m_offset.y += m_rect.y;
		m_rect.h = std::max(static_cast<int>(m_rect.h) + m_rect.y, 0);
		m_rect.y = 0;
	}

	if (m_rect.y + m_rect.h > m_surface->get_h())
		m_rect.h =
			std::max(static_cast<int>(m_surface->get_h()) - m_rect.y, 0);
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
bool RenderTarget::enter_window(const Rect& rc, Rect* previous, Point* prevofs)
{
	Point newofs(0, 0);
	Rect newrect = rc;

	if (clip(newrect)) {

		// Apply the changes

		if (previous)
			*previous = m_rect;

		if (prevofs)
			*prevofs = m_offset;

		m_rect = newrect;

		m_offset = newofs;

		return true;
	} else return false;
}

/**
 * Returns the true size of the render target (ignoring the window settings).
 */
int RenderTarget::get_w() const
{
	return m_surface->get_w();
}

/**
 * Returns the true size of the render target (ignoring the window settings).
 */
int RenderTarget::get_h() const
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
void RenderTarget::draw_line(int x1, int y1, int x2, int y2, RGBColor color)
{
	int dx=x2-x1;      /* the horizontal distance of the line */
	int dy=y2-y1;      /* the vertical distance of the line */
	const uint dxabs = abs(dx);
	const uint dyabs = abs(dy);
	int sdx= dx < 0 ? -1 : 1;
	int sdy= dy < 0 ? -1 : 1;
	uint x = dyabs / 2;
	uint y = dxabs / 2;
	Point p(x1, y1);

	draw_rect(Rect(p, 1, 1), color);

	if (dxabs >= dyabs) for (uint i = 0;i < dxabs; ++i) {
			//  the line is more horizontal than vertical
		y+=dyabs;

		if (y >= dxabs) {y -= dxabs; p.y += sdy;}

		p.x += sdx;
		draw_rect(Rect(p, 1, 1), color);
	}
	else for (uint i = 0; i < dyabs; ++i) {
			// the line is more vertical than horizontal
		x+=dxabs;

		if (x >= dyabs) {x -= dyabs; p.x += sdx;}

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

void RenderTarget::brighten_rect(Rect r, const int factor)
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
void RenderTarget::blit(const Point dst, const uint picture)
{
	if (Surface * const src = g_gr->get_picture_surface(picture))
		doblit(dst, src, Rect(Point(0, 0), src->get_w(), src->get_h()));
}

void RenderTarget::blitrect(const Point dst, const uint picture,
			    const Rect srcrc)
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
void RenderTarget::tile(Rect r, uint picture, Point ofs)
{
	Surface* src = g_gr->get_picture_surface(picture);

	if (!src) {
		log("RenderTarget::tile: bad picture %u\n", picture);
		return;
	}

	if (clip(r)) {

		// Make sure the offset is within bounds
		ofs.x = ofs.x % src->get_w();

		if (ofs.x < 0) ofs.x += src->get_w();

		ofs.y = ofs.y % src->get_h();

		if (ofs.y < 0) ofs.y += src->get_h();

		// Blit the picture into the rectangle
		uint ty = 0;

		while (ty < r.h) {
			uint tx = 0;
			int tofsx = ofs.x;
			Rect srcrc;

			srcrc.y = ofs.y;
			srcrc.h = src->get_h() - ofs.y;

			if (ty + srcrc.h > r.h) srcrc.h = r.h - ty;

			while (tx < r.w) {
				srcrc.x = tofsx;
				srcrc.w = src->get_w() - tofsx;

				if (tx + srcrc.w > r.w) srcrc.w = r.w - tx;

				m_surface->blit(r + Point(tx, ty), src, srcrc);

				tx += srcrc.w;

				tofsx = 0;
			}

			ty += srcrc.h;
			ofs.y = 0;
		}
	}
}


#define RENDERMAP_INITIALIZANTONS                                              \
	/* Check if we have the ground surface set up. */                           \
	if (not m_ground_surface) m_ground_surface = new Surface(*m_surface);       \
                                                                               \
	viewofs -= m_offset;                                                        \
                                                                               \
	const Map             & map             = egbase.map();                     \
	const World           & world           = map.world();                      \
	const Overlay_Manager & overlay_manager = map.get_overlay_manager();        \
	const uint              mapwidth        = map.get_width();                  \
	int minfx, minfy;                                                           \
	int maxfx, maxfy;                                                           \
                                                                               \
	/* hack to prevent negative numbers */                                      \
	minfx = (viewofs.x + (TRIANGLE_WIDTH>>1)) / TRIANGLE_WIDTH - 1;             \
                                                                               \
	minfy = viewofs.y / TRIANGLE_HEIGHT;                                        \
	maxfx = (viewofs.x + (TRIANGLE_WIDTH>>1) + m_rect.w) / TRIANGLE_WIDTH;      \
	maxfy = (viewofs.y + m_rect.h) / TRIANGLE_HEIGHT;                           \
	maxfx +=  1; /* because of big buildings */                                 \
	maxfy += 10; /* because of heights */                                       \
                                                                               \
	int dx              = maxfx - minfx + 1;                                    \
	int dy              = maxfy - minfy + 1;                                    \
	int linear_fy       = minfy;                                                \
	bool row_is_forward = linear_fy & 1;                                        \
	int b_posy          = linear_fy * TRIANGLE_HEIGHT - viewofs.y;


/**
 * Loop through fields row by row. For each field, draw ground textures, then
 * roads, then immovables, then bobs, then overlay stuff (build icons etc...)
 */
void RenderTarget::rendermap
(const Editor_Game_Base & egbase,
 const Player           & player,
 Point                    viewofs,
 const bool               draw_all)
{
	if (player.see_all()) return rendermap(egbase, viewofs, draw_all);

	RENDERMAP_INITIALIZANTONS;

	const Player::Field * const first_player_field = player.fields();
	const Editor_Game_Base::Time gametime = egbase.get_gametime();

	while (dy--) {
		const int posy = b_posy;
		b_posy += TRIANGLE_HEIGHT;
		const int linear_fx = minfx;
		FCoords r(Coords(linear_fx, linear_fy));
		FCoords br(Coords(linear_fx - not row_is_forward, linear_fy + 1));
		int r_posx =
			r.x * TRIANGLE_WIDTH
			+
			row_is_forward * (TRIANGLE_WIDTH / 2)
			-
			viewofs.x;
		int br_posx = r_posx - TRIANGLE_WIDTH / 2;

		// Calculate safe (bounded) field coordinates and get field pointers
		map.normalize_coords(&r);
		map.normalize_coords(&br);
		Map::Index r_index = Map::get_index(r, mapwidth);
		r.field = &map[r_index];
		Map::Index br_index = Map::get_index(br, mapwidth);
		br.field = &map[br_index];
		const Player::Field *  r_player_field = first_player_field +  r_index;
		const Player::Field * br_player_field = first_player_field + br_index;
		FCoords tr, f;
		map.get_tln(r, &tr);
		map.get_ln(r, &f);
		Map::Index tr_index = tr.field - &map[0];
		const Texture * f_r_texture =
			g_gr->get_maptexture_data
			(world
			 .terrain_descr(first_player_field[f.field - &map[0]].terrains.r)
			 .get_texture());

		uint count = dx;

		while (count--) {
			const FCoords l = f, bl = br;
			const Player::Field &  f_player_field =  *r_player_field;
			const Player::Field & bl_player_field = *br_player_field;
			f = r;
			const int f_posx = r_posx, bl_posx = br_posx;
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

			const Uint8 roads =
				f_player_field.roads | overlay_manager.get_road_overlay(f);

			m_ground_surface->draw_field //  Render ground
				(m_rect,
				 f.field, r.field, bl.field, br.field,
				 f_posx, r_posx, posy, bl_posx, br_posx, b_posy,
				 roads,
				 node_brightness
				 (gametime, f_player_field.time_node_last_unseen,
				  f_player_field.vision, f.field->get_brightness()),
				 node_brightness
				 (gametime, r_player_field->time_node_last_unseen,
				  r_player_field->vision, r.field->get_brightness()),
				 node_brightness
				 (gametime, bl_player_field.time_node_last_unseen,
				  bl_player_field.vision, bl.field->get_brightness()),
				 node_brightness
				 (gametime, br_player_field->time_node_last_unseen,
				  br_player_field->vision, br.field->get_brightness()),
				 tr_d_texture, l_r_texture, f_d_texture, *f_r_texture,
				 draw_all);
		}

		++linear_fy;
		row_is_forward = not row_is_forward;
	}

	// Copy ground where it belongs: on the screen
	m_surface->blit(Point(m_rect.x, m_rect.y), m_ground_surface, m_rect);

	{
		const int dx2        = maxfx - minfx + 1;
		int dy2              = maxfy - minfy + 1;
		int linear_fy2       = minfy;
		bool row_is_forward2 = linear_fy2 & 1;
		int b_posy2          = linear_fy2 * TRIANGLE_HEIGHT - viewofs.y;

		while (dy2--) {
			const int posy = b_posy2;
			b_posy2 += TRIANGLE_HEIGHT;

			{//  Draw things on the node.
				const int linear_fx = minfx;
				FCoords r(Coords(linear_fx, linear_fy2));
				FCoords br
					(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));

				// Calculate safe (bounded) field coordinates and get field pointers
				map.normalize_coords(&r);
				map.normalize_coords(&br);
				Map::Index r_index = Map::get_index(r, mapwidth);
				r.field = &map[r_index];
				Map::Index br_index = Map::get_index(br, mapwidth);
				br.field = &map[br_index];
				FCoords tr, f;
				map.get_tln(r, &tr);
				map.get_ln(r, &f);
				bool r_is_border;
				uchar f_owner_number = f.field->get_owned_by();//  FIXME PPoV
				uchar r_owner_number;
				r_is_border = r.field->is_border();//  FIXME PPoV
				r_owner_number = r.field->get_owned_by();//  FIXME PPoV
				uchar br_owner_number = br.field->get_owned_by();//  FIXME PPoV
				const Player::Field * r_player_field = first_player_field + r_index;
				const Player::Field * br_player_field =
					first_player_field + br_index;
				Vision  r_vision =  r_player_field->vision;
				Vision br_vision = br_player_field->vision;
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

				int count = dx2;

				while (count--) {
					const FCoords l = f, bl = br;
					f = r;
					const Player::Field & f_player_field = *r_player_field;
					move_r(mapwidth, tr);
					move_r(mapwidth,  r,  r_index);
					move_r(mapwidth, br, br_index);
					r_player_field  = first_player_field +  r_index;
					br_player_field = first_player_field + br_index;
					const uchar tr_owner_number = tr.field->get_owned_by(); //  FIXME PPoV
					const bool f_is_border = r_is_border;
					const uchar l_owner_number = f_owner_number;
					const uchar bl_owner_number = br_owner_number;
					f_owner_number = r_owner_number;
					r_is_border = r.field->is_border();         //  FIXME PPoV
					r_owner_number = r.field->get_owned_by();   //  FIXME PPoV
					br_owner_number = br.field->get_owned_by(); //  FIXME PPoV
					const Vision  f_vision =  r_vision;
					const Vision bl_vision = br_vision;
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
						const uint anim = owner.tribe().get_frontier_anim();
						if (1 < f_vision) drawanim(f_pos, anim, 0, &owner);
						if
							((f_vision | r_vision)
							 and
							 r_owner_number == f_owner_number
							 and
							 (tr_owner_number == f_owner_number
							  xor
							  br_owner_number == f_owner_number))
							drawanim(middle(f_pos, r_pos), anim, 0, &owner);
						if
							((f_vision | bl_vision)
							 and
							 bl_owner_number == f_owner_number
							 and
							 (l_owner_number == f_owner_number
							  xor
							  br_owner_number == f_owner_number))
							drawanim(middle(f_pos, bl_pos), anim, 0, &owner);
						if
							((f_vision | br_vision)
							 and
							 br_owner_number == f_owner_number
							 and
							 (r_owner_number == f_owner_number
							  xor
							  bl_owner_number == f_owner_number))
							drawanim(middle(f_pos, br_pos), anim, 0, &owner);
					}

					if (1 < f_vision) {// Render stuff that belongs to the node

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
							(Bob * bob = f.field->get_first_bob();
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
							if (const uint picid = map_object_descr->main_animation())
								drawanim(f_pos, picid, 0);
							else if (map_object_descr == &g_flag_descr) {
								const Player & owner = egbase.player(f_owner_number);
								drawanim
									(f_pos, owner.tribe().get_flag_anim(), 0, &owner);
							}
						}
				}
			}

			if (false) {//  Draw things on the R-triangle (nothing to draw yet).
				const int linear_fx = minfx;
				FCoords r(Coords(linear_fx, linear_fy2));
				FCoords b(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));
				int posx =
					(linear_fx - 1) * TRIANGLE_WIDTH
					+
					(row_is_forward2 + 1) * (TRIANGLE_WIDTH / 2)
					-
					viewofs.x;

				//  Calculate safe (bounded) field coordinates.
				map.normalize_coords(&r);
				map.normalize_coords(&b);

				//  Get field pointers.
				r.field = &map[Map::get_index(r, mapwidth)];
				b.field = &map[Map::get_index(b, mapwidth)];

				int count = dx2;

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
					{//  FIXME Visibility check here.
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

			if (false) {//  Draw things on the D-triangle (nothing to draw yet).
				const int linear_fx = minfx;
				FCoords f(Coords(linear_fx - 1, linear_fy2));
				FCoords br
					(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));
				int posx =
					(linear_fx - 1) * TRIANGLE_WIDTH
					+
					row_is_forward2 * (TRIANGLE_WIDTH / 2)
					-
					viewofs.x;

				//  Calculate safe (bounded) field coordinates.
				map.normalize_coords(&f);
				map.normalize_coords(&br);

				//  Get field pointers.
				f.field  = &map[Map::get_index(f,  mapwidth)];
				br.field = &map[Map::get_index(br, mapwidth)];

				int count = dx2;

				while (count--) {
					const FCoords bl = br;
					map.get_rn(f, &f);
					map.get_rn(br, &br);
					posx += TRIANGLE_WIDTH;

					{//  FIXME Visibility check here.
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
(const Editor_Game_Base & egbase, Point viewofs, const bool draw_all)
{
	RENDERMAP_INITIALIZANTONS;

	while (dy--) {
		const int posy = b_posy;
		b_posy += TRIANGLE_HEIGHT;
		const int linear_fx = minfx;
		FCoords r(Coords(linear_fx, linear_fy));
		FCoords br(Coords(linear_fx - not row_is_forward, linear_fy + 1));
		int r_posx =
			r.x * TRIANGLE_WIDTH
			+
			row_is_forward * (TRIANGLE_WIDTH / 2)
			-
			viewofs.x;
		int br_posx = r_posx - TRIANGLE_WIDTH / 2;

		// Calculate safe (bounded) field coordinates and get field pointers
		map.normalize_coords(&r);
		map.normalize_coords(&br);
		Map::Index r_index = Map::get_index(r, mapwidth);
		r.field = &map[r_index];
		Map::Index br_index = Map::get_index(br, mapwidth);
		br.field = &map[br_index];
		FCoords tr, f;
		map.get_tln(r, &tr);
		map.get_ln(r, &f);
		const Texture * f_r_texture =
			g_gr->get_maptexture_data
			(world.terrain_descr(f.field->terrain_r()).get_texture());

		uint count = dx;

		while (count--) {
			const FCoords l = f, bl = br;
			f = r;
			const int f_posx = r_posx, bl_posx = br_posx;
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

			const uchar roads =
				f.field->get_roads() | overlay_manager.get_road_overlay(f);

			m_ground_surface->draw_field //  Render ground
				(m_rect,
				 f.field, r.field, bl.field, br.field,
				 f_posx, r_posx, posy, bl_posx, br_posx, b_posy,
				 roads,
				 f .field->get_brightness(), r .field->get_brightness(),
				 bl.field->get_brightness(), br.field->get_brightness(),
				 tr_d_texture, l_r_texture, f_d_texture, *f_r_texture,
				 draw_all);
		}

		++linear_fy;
		row_is_forward = not row_is_forward;
	}

	// Copy ground where it belongs: on the screen
	m_surface->blit(Point(m_rect.x, m_rect.y), m_ground_surface, m_rect);

	{
		const int dx2 = maxfx - minfx + 1;
		int dy2 = maxfy - minfy + 1;
		int linear_fy2 = minfy;
		bool row_is_forward2 = linear_fy2 & 1;
		int b_posy2 = linear_fy2 * TRIANGLE_HEIGHT - viewofs.y;

		while (dy2--) {
			const int posy = b_posy2;
			b_posy2 += TRIANGLE_HEIGHT;

			{//  Draw things on the node.
				const int linear_fx = minfx;
				FCoords r(Coords(linear_fx, linear_fy2));
				FCoords br(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));

				// Calculate safe (bounded) field coordinates and get field pointers
				map.normalize_coords(&r);
				map.normalize_coords(&br);
				Map::Index r_index = Map::get_index(r, mapwidth);
				r.field = &map[r_index];
				Map::Index br_index = Map::get_index(br, mapwidth);
				br.field = &map[br_index];
				FCoords tr, f;
				map.get_tln(r, &tr);
				map.get_ln(r, &f);
				bool r_is_border;
				uchar f_owner_number = f.field->get_owned_by();
				uchar r_owner_number;
				r_is_border = r.field->is_border();
				r_owner_number = r.field->get_owned_by();
				uchar br_owner_number = br.field->get_owned_by();
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

				int count = dx2;

				while (count--) {
					const FCoords l = f, bl = br;
					f = r;
					move_r(mapwidth, tr);
					move_r(mapwidth,  r,  r_index);
					move_r(mapwidth, br, br_index);
					const uchar tr_owner_number = tr.field->get_owned_by();
					const bool f_is_border = r_is_border;
					const uchar l_owner_number = f_owner_number;
					const uchar bl_owner_number = br_owner_number;
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
						const uint anim = owner.tribe().get_frontier_anim();
						drawanim(f_pos, anim, 0, &owner);
						if
							(r_owner_number == f_owner_number
							 and
							 (tr_owner_number == f_owner_number
							  xor
							  br_owner_number == f_owner_number))
							drawanim(middle(f_pos, r_pos), anim, 0, &owner);
						if
							(bl_owner_number == f_owner_number
							 and
							 (l_owner_number == f_owner_number
							  xor
							  br_owner_number == f_owner_number))
							drawanim(middle(f_pos, bl_pos), anim, 0, &owner);
						if
							(br_owner_number == f_owner_number
							 and
							 (r_owner_number == f_owner_number
							  xor
							  bl_owner_number == f_owner_number))
							drawanim(middle(f_pos, br_pos), anim, 0, &owner);
					}

					{// Render stuff that belongs to the node

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
							(Bob * bob = f.field->get_first_bob();
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

			{//  Draw things on the R-triangle.
				const int linear_fx = minfx;
				FCoords r(Coords(linear_fx, linear_fy2));
				FCoords b
					(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));
				int posx =
					(linear_fx - 1) * TRIANGLE_WIDTH
					+
					(row_is_forward2 + 1) * (TRIANGLE_WIDTH / 2)
					-
					viewofs.x;

				//  Calculate safe (bounded) field coordinates.
				map.normalize_coords(&r);
				map.normalize_coords(&b);

				//  Get field pointers.
				r.field = &map[Map::get_index(r, mapwidth)];
				b.field = &map[Map::get_index(b, mapwidth)];

				int count = dx2;

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

			{//  Draw things on the D-triangle.
				const int linear_fx = minfx;
				FCoords f(Coords(linear_fx - 1, linear_fy2));
				FCoords br(Coords(linear_fx - not row_is_forward2, linear_fy2 + 1));
				int posx =
					(linear_fx - 1) * TRIANGLE_WIDTH
					+
					row_is_forward2 * (TRIANGLE_WIDTH / 2)
					-
					viewofs.x;

				//  Calculate safe (bounded) field coordinates.
				map.normalize_coords(&f);
				map.normalize_coords(&br);

				//  Get field pointers.
				f.field  = &map[Map::get_index(f,  mapwidth)];
				br.field = &map[Map::get_index(br, mapwidth)];

				int count = dx2;

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
void RenderTarget::renderminimap(const Editor_Game_Base & egbase,
				 const Player * const player,
				 const Point viewpoint,
				 const uint flags)
{
	m_surface->draw_minimap
			(egbase, player, m_rect, viewpoint - m_offset, flags);
}

/**
 * Draws a frame of an animation at the given location
 * Plays sound effect that is registered with this frame (the Sound_Handler
 * decides if the fx really does get played)
 *
 * \par dstx, dsty
 * \par animation
 * \par time
 * \par player
 *
 * \todo Document this method's parameters
 * \todo Correctly calculate the stereo position for sound effects
 */
void RenderTarget::drawanim(Point dst, const uint animation, const uint time,
			    const Player * const player)
{
	const AnimationData* data = g_anim.get_animation(animation);
	AnimationGfx* gfx = g_gr->get_animation(animation);

	//TODO? assert(player);
	assert(data);
	assert(gfx);

	if (!data || !gfx) {
		log("WARNING: Animation %i doesn't exist\n", animation);
		return;
	}

	// Get the frame and its data
	Surface* frame;

	const uint framenumber = (time / data->frametime) % gfx->nr_frames();

	frame = gfx->get_frame
			(framenumber, player ? player->get_player_number() : 0, player);

	dst -= gfx->get_hotspot();

	Rect srcrc(Point(0, 0), frame->get_w(), frame->get_h());

	doblit(dst, frame, srcrc);

	// Look if there's a sound effect registered for this frame and trigger the effect
	uint stereo_position=128; //see Sound_Handler::stereo_position()

	g_anim.trigger_soundfx(animation, framenumber, stereo_position);
}

/**
 * Draws a part of a frame of an animation at the given location
 */
void RenderTarget::drawanimrect(Point dst, const uint animation,
				const uint time, const Player * const player,
				Rect srcrc)
{
	const AnimationData* data = g_anim.get_animation(animation);
	if (!data || !g_gr) {
		log("WARNING: Animation %i doesn't exist\n", animation);
		return;
	}

	// Get the frame and its data
	Surface * const frame = g_gr->get_animation(animation)->get_frame
			((time / data->frametime) % g_gr->nr_frames(animation),
			  player ? player->get_player_number() : 0,
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
		if (r.w <= static_cast<uint>(-r.x)) return false;

		r.w += r.x;

		r.x = 0;
	}

	if (r.x + r.w > m_rect.w) {
		if (static_cast<int>(m_rect.w) <= r.x) return false;
		r.w = m_rect.w - r.x;
	}

	if (r.y < 0) {
		if (r.h <= static_cast<uint>(-r.y)) return false;
		r.h += r.y;
		r.y = 0;
	}

	if (r.y + r.h > m_rect.h) {
		if (static_cast<int>(m_rect.h) <= r.y) return false;
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
		if (srcrc.w <= static_cast<uint>(-dst.x)) return;

		srcrc.x -= dst.x;

		srcrc.w += dst.x;

		dst.x = 0;
	}

	if (dst.x + srcrc.w > m_rect.w) {
		if (static_cast<int>(m_rect.w) <= dst.x) return;
		srcrc.w = m_rect.w - dst.x;
	}

	if (dst.y < 0) {
		if (srcrc.h <= static_cast<uint>(-dst.y)) return;
		srcrc.y -= dst.y;
		srcrc.h += dst.y;
		dst.y = 0;
	}

	if (dst.y + srcrc.h > m_rect.h) {
		if (static_cast<int>(m_rect.h) <= dst.y) return;
		srcrc.h = m_rect.h - dst.y;
	}

	dst += m_rect;

	// Draw it
	m_surface->blit(dst, src, srcrc);
}

///\todo Rename the _in_ parameter "result" to reflect it's real meaning
Sint8 RenderTarget::node_brightness(const Editor_Game_Base::Time gametime,
				    Editor_Game_Base::Time last_seen,
				    const Vision vision, Sint8 result)
{
	if      (vision == 0) result = -128;
	else if (vision == 1) {
		assert(last_seen <= gametime);
		const Editor_Game_Base::Duration time_ago = gametime - last_seen;
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
