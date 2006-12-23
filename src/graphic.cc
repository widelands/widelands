/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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
/*
Management classes and functions of the 16-bit software renderer.
*/

#include <SDL_image.h>
#include "bob.h"
#include "editor_game_base.h"
#include "error.h"
#include "fileread.h"
#include "filewrite.h"
#include "font_handler.h"
#include "graphic_impl.h"
#include "map.h"
#include "mapviewpixelconstants.h"
#include "mapviewpixelfunctions.h"
#include "layered_filesystem.h"
#include "overlay_manager.h"
#include "player.h"
#include "tribe.h"

/*
 * Names of road terrains
 */
#define ROAD_NORMAL_PIC "pics/roadt_normal.png"
#define ROAD_BUSY_PIC   "pics/roadt_busy.png"

Graphic *g_gr;

/*
===============
LoadImage

Helper function wraps around SDL_image. Returns the given image file as a
surface.
Cannot return 0, throws an exception on error.
===============
*/
SDL_Surface* LoadImage(const char * const filename) {
	FileRead fr;
	SDL_Surface* surf;

	fr.Open(*g_fs, filename);

	surf = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);
	if (!surf)
		throw wexception("%s", IMG_GetError());

	return surf;
}


/*
==============================================================================

RenderTargetImpl -- wrapper around a Bitmap that can be rendered into

==============================================================================
*/

/*
===============
RenderTargetImpl::RenderTargetImpl

Build a render target for the given bitmap.
Note that the bitmap will not be owned by the renderer, i.e. it won't be
deleted by the destructor.
===============
*/
RenderTargetImpl::RenderTargetImpl(Surface* bmp)
{
	m_surface = bmp;
   m_ground_surface = 0;

	reset();
}


/*
===============
RenderTargetImpl::~RenderTargetImpl
===============
*/
RenderTargetImpl::~RenderTargetImpl()
{
   if(m_ground_surface)
      delete m_ground_surface;
}

/*
===============
RenderTargetImpl::reset

Called every time before the render target is handed out by the Graphic
implementation to start in a neutral state.
===============
*/
void RenderTargetImpl::reset()
{
	m_rect.x = m_rect.y = 0;
	m_rect.w = m_surface->get_w();
	m_rect.h = m_surface->get_h();

	m_offset.x = m_offset.y = 0;
}


/*
===============
RenderTargetImpl::get_window [const]

Retrieve the current window setting.
===============
*/
void RenderTargetImpl::get_window(Rect* rc, Point* ofs) const
{
	*rc = m_rect;
	*ofs = m_offset;
}


/*
===============
RenderTargetImpl::set_window

Sets an arbitrary drawing window.
===============
*/
void RenderTargetImpl::set_window(const Rect& rc, const Point& ofs)
{
	m_rect = rc;
	m_offset = ofs;

	// safeguards clipping against the bitmap itself
	if (m_rect.x < 0) {
		m_offset.x += m_rect.x;
		m_rect.w += m_rect.x;
		m_rect.x = 0;
	}
	if (m_rect.x + m_rect.w > (int)m_surface->get_w())
		m_rect.w = m_surface->get_w() - m_rect.x;
	if (m_rect.w < 0)
		m_rect.w = 0;

	if (m_rect.y < 0) {
		m_offset.y += m_rect.y;
		m_rect.h += m_rect.y;
		m_rect.y = 0;
	}
	if (m_rect.y + m_rect.h > (int)m_surface->get_h())
		m_rect.h = m_surface->get_h() - m_rect.y;
	if (m_rect.h < 0)
		m_rect.h = 0;
}


/*
===============
RenderTargetImpl::enter_window

Builds a subwindow. rc is relative to the current drawing window. The subwindow
will be clipped appropriately.
The previous window state is returned in previous and prevofs.

Returns false if the subwindow is invisible. In that case, the window state is
not changed at all. Otherwise, the function returns true.
===============
*/
bool RenderTargetImpl::enter_window(const Rect& rc, Rect* previous, Point* prevofs)
{
	Point newofs(0,0);
	Rect newrect;

	newrect.x = rc.x + m_offset.x;
	newrect.y = rc.y + m_offset.y;
	newrect.w = rc.w;
	newrect.h = rc.h;

	// Clipping
	if (newrect.x < 0) {
		newofs.x = newrect.x;
		newrect.w += newrect.x;
		newrect.x = 0;
	}
	if (newrect.x + newrect.w > m_rect.w)
		newrect.w = m_rect.w - newrect.x;
	if (newrect.w <= 0)
		return false;

	if (newrect.y < 0) {
		newofs.y = newrect.y;
		newrect.h += newrect.y;
		newrect.y = 0;
	}
	if (newrect.y + newrect.h > m_rect.h)
		newrect.h = m_rect.h - newrect.y;
	if (newrect.h <= 0)
		return false;

	newrect.x += m_rect.x;
	newrect.y += m_rect.y;

	// Apply the changes
	if (previous)
		*previous = m_rect;
	if (prevofs)
		*prevofs = m_offset;

	m_rect = newrect;
	m_offset = newofs;

	return true;
}


/*
===============
RenderTargetImpl::get_w [const]
RenderTargetImpl::get_h [const]

Returns the true size of the render target (ignoring the window settings).
===============
*/
int RenderTargetImpl::get_w() const
{
	return m_surface->get_w();
}

int RenderTargetImpl::get_h() const
{
	return m_surface->get_h();
}

/*
 * Render Target: draw line
 *
 * This functions draws a (not horizontal or vertical)
 * line in the target, using Bresenham's algorithm
 *
 * This function is still quite slow, since it draws
 * every pixel as a rectangle. So use it with care
 */
void RenderTargetImpl::draw_line(int x1, int y1, int x2, int y2, RGBColor color)
{
   int dx=x2-x1;      /* the horizontal distance of the line */
   int dy=y2-y1;      /* the vertical distance of the line */
   int dxabs=abs(dx);
   int dyabs=abs(dy);
   int sdx= dx < 0 ? -1 : 1;
   int sdy= dy < 0 ? -1 : 1;
   int x=dyabs>>1;
   int y=dxabs>>1;
   int px=x1;
   int py=y1;

   draw_rect(px,py,1,1,color);

   if (dxabs>=dyabs) /* the line is more horizontal than vertical */
   {
      for(int i=0;i<dxabs;i++)
      {
         y+=dyabs;
         if (y>=dxabs)
         {
            y-=dxabs;
            py+=sdy;
         }
         px+=sdx;
         draw_rect(px,py,1,1,color);
      }
   }
   else /* the line is more vertical than horizontal */
   {
      for(int i=0;i<dyabs;i++)
      {
         x+=dxabs;
         if (x>=dyabs)
         {
            x-=dyabs;
            px+=sdx;
         }
         py+=sdy;
         draw_rect(px,py,1,1,color);
      }
   }
}
/*
===============
RenderTargetImpl::draw_rect
RenderTargetImpl::fill_rect
RenderTargetImpl::brighten_rect
RenderTargetImpl::clear

Clip against window and pass those primitives along to the bitmap.
===============
*/
void RenderTargetImpl::draw_rect(int x, int y, int w, int h, RGBColor clr)
{
	x += m_offset.x;
	y += m_offset.y;

	if (x < 0) {
		w += x;
		x = 0;
	}
	if (x + w > m_rect.w)
		w = m_rect.w - x;
	if (w <= 0)
		return;

	if (y < 0) {
		h += y;
		y = 0;
	}
	if (y + h > m_rect.h)
		h = m_rect.h - y;
	if (h <= 0)
		return;

	m_surface->draw_rect(Rect(x + m_rect.x, y + m_rect.y, w, h), clr);
}

void RenderTargetImpl::fill_rect(int x, int y, int w, int h, RGBColor clr)
{
	x += m_offset.x;
	y += m_offset.y;

	if (x < 0) {
		w += x;
		x = 0;
	}
	if (x + w > m_rect.w)
		w = m_rect.w - x;
	if (w <= 0)
		return;

	if (y < 0) {
		h += y;
		y = 0;
	}
	if (y + h > m_rect.h)
		h = m_rect.h - y;
	if (h <= 0)
		return;

	m_surface->fill_rect(Rect(x + m_rect.x, y + m_rect.y, w, h), clr);
}

void RenderTargetImpl::brighten_rect(int x, int y, int w, int h, int factor)
{
	x += m_offset.x;
	y += m_offset.y;

	if (x < 0) {
		w += x;
		x = 0;
	}
	if (x + w > m_rect.w)
		w = m_rect.w - x;
	if (w <= 0)
		return;

	if (y < 0) {
		h += y;
		y = 0;
	}
	if (y + h > m_rect.h)
		h = m_rect.h - y;
	if (h <= 0)
		return;

	m_surface->brighten_rect(Rect(x + m_rect.x, y + m_rect.y, w, h), factor);
}

void RenderTargetImpl::clear(void)
{
	if (!m_rect.x && !m_rect.y && m_rect.w == (int)m_surface->get_w() && m_rect.h == (int)m_surface->get_h())
		m_surface->clear();
	else
		m_surface->fill_rect(Rect(m_rect.x, m_rect.y, m_rect.w, m_rect.h), RGBColor(0,0,0));
}


/*
===============
RenderTargetImpl::doblit

Clip against window and source bitmap, then call the Bitmap blit routine.
===============
*/
void RenderTargetImpl::doblit(Point dst, Surface* src, Rect srcrc)
{
	dst.x += m_offset.x;
	dst.y += m_offset.y;

	// Clipping
	if (dst.x < 0) {
		srcrc.x -= dst.x;
		srcrc.w += dst.x;
		dst.x = 0;
	}
	if (srcrc.x < 0) {
		dst.x -= srcrc.x;
		srcrc.w += srcrc.x;
		srcrc.x = 0;
	}
	if (dst.x + srcrc.w > m_rect.w)
		srcrc.w = m_rect.w - dst.x;
	if (srcrc.w <= 0)
		return;

	if (dst.y < 0) {
		srcrc.y -= dst.y;
		srcrc.h += dst.y;
		dst.y = 0;
	}
	if (srcrc.y < 0) {
		dst.y -= srcrc.y;
		srcrc.h += srcrc.y;
		srcrc.y = 0;
	}
	if (dst.y + srcrc.h > m_rect.h)
		srcrc.h = m_rect.h - dst.y;
	if (srcrc.h <= 0)
		return;

	// Draw it
	m_surface->blit(Point(dst.x + m_rect.x, dst.y + m_rect.y), src, srcrc);
}


/*
===============
RenderTargetImpl::blit
RenderTargetImpl::blitrect

Blits a blitsource into this bitmap
===============
*/
void RenderTargetImpl::blit(int dstx, int dsty, uint picture)
{
	GraphicImpl* gfx = get_graphicimpl();
	Surface* src = gfx->get_picture_surface(picture);

	if (src)
		doblit(Point(dstx, dsty), src, Rect(0, 0, src->get_w(), src->get_h()));
}

void RenderTargetImpl::blitrect(int dstx, int dsty, uint picture,
	                             int srcx, int srcy, int w, int h)
{
	GraphicImpl* gfx = get_graphicimpl();
	Surface* src = gfx->get_picture_surface(picture);

	if (src)
		doblit(Point(dstx, dsty), src, Rect(srcx, srcy, w, h));
}


/*
===============
RenderTargetImpl::tile

Fill the given rectangle with the given picture.
The pixel from (ofsx/ofsy) inside picture is placed at the top-left corner of
the filled rectangle.
===============
*/
void RenderTargetImpl::tile(int x, int y, int w, int h, uint picture, int ofsx, int ofsy)
{
	GraphicImpl* gfx = get_graphicimpl();
	Surface* src = gfx->get_picture_surface(picture);

	if (!src) {
		log("RenderTargetImpl::tile: bad picture %u\n", picture);
		return;
	}

	// Clipping
	x += m_offset.x;
	y += m_offset.y;

	if (x < 0) {
		w += x;
		x = 0;
	}
	if (x + w > m_rect.w)
		w = m_rect.w - x;
	if (w <= 0)
		return;

	if (y < 0) {
		h += y;
		y = 0;
	}
	if (y + h > m_rect.h)
		h = m_rect.h - y;
	if (h <= 0)
		return;

	// Make sure the offset is within bounds
	ofsx = ofsx % src->get_w();
	if (ofsx < 0)
		ofsx += src->get_w();

	ofsy = ofsy % src->get_h();
	if (ofsy < 0)
		ofsy += src->get_h();

	// Blit the picture into the rectangle
	int ty = 0;

	while(ty < h)
		{
		int tx = 0;
		int tofsx = ofsx;
		Rect srcrc;

		srcrc.y = ofsy;
		srcrc.h = src->get_h() - ofsy;
		if (ty + srcrc.h > h)
			srcrc.h = h - ty;

		while(tx < w)
			{
			srcrc.x = tofsx;
			srcrc.w = src->get_w() - tofsx;
			if (tx + srcrc.w > w)
				srcrc.w = w - tx;

			m_surface->blit(Point(m_rect.x + x + tx, m_rect.y + y + ty), src, srcrc);

			tx += srcrc.w;
			tofsx = 0;
			}

		ty += srcrc.h;
		ofsy = 0;
		}
}


/*
===============
RenderTargetImpl::rendermap

Render the map into the current drawing window.
viewofs is the offset of the upper left corner of the window into the map,
in pixels.

the last parameter, draw all, is only to check if the whole ground texture tiles have
to be redrawn or only the aniamted once. If no animation took place, the ground is not
redrawn at all.
===============
*/
void RenderTargetImpl::rendermap
(const Editor_Game_Base & egbase,
 const std::vector<bool> * const visibility,
 Point viewofs,
 const bool draw_all)
{
   // Check if we have the ground surface set up
   if( !m_ground_surface ) {
      m_ground_surface = new Surface( *m_surface );
   }
	viewofs.x -= m_offset.x;
	viewofs.y -= m_offset.y;


	// Completely clear the window ( this blinks )
	// m_ground_surface->clear();

	// Actually draw the map. Loop through fields row by row
	// For each field, draw ground textures, then roads, then immovables
	// (and borders), then bobs, then overlay stuff (build icons etc...)

	const Map & map = egbase.map();
	const Overlay_Manager & overlay_manager = map.get_overlay_manager();
	const uint mapwidth = map.get_width();
	int minfx, minfy;
	int maxfx, maxfy;

	minfx = (viewofs.x + (TRIANGLE_WIDTH>>1)) / TRIANGLE_WIDTH - 1; // hack to prevent negative numbers
	minfy = viewofs.y / TRIANGLE_HEIGHT;
	maxfx = (viewofs.x + (TRIANGLE_WIDTH>>1) + m_rect.w) / TRIANGLE_WIDTH;
	maxfy = (viewofs.y + m_rect.h) / TRIANGLE_HEIGHT;
	maxfx += 1; // because of big buildings
	maxfy += 10; // necessary because of heights

	//log("%i %i -> %i %i\n", minfx, minfy, maxfx, maxfy);
	int dx = maxfx - minfx + 1;
	int dy = maxfy - minfy + 1;
   int linear_fy = minfy;
	bool row_is_forward = linear_fy & 1;
	int b_posy = linear_fy * TRIANGLE_HEIGHT - viewofs.y;

	while(dy--) {
		const int posy = b_posy;
		b_posy += TRIANGLE_HEIGHT;
		const int linear_fx = minfx;
		FCoords r(linear_fx, linear_fy);
		FCoords br(linear_fx - (not row_is_forward), linear_fy + 1);
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

		uint count = dx;
		while (count--) {
			const Map::Index f_index = r_index, bl_index = br_index;
			const FCoords l = f, bl = br;
			f = r;
			const int f_posx = r_posx, bl_posx = br_posx;
			move_r(mapwidth, tr);
			move_r(mapwidth,  r,  r_index);
			move_r(mapwidth, br, br_index);
			r_posx  += TRIANGLE_WIDTH;
			br_posx += TRIANGLE_WIDTH;

			uchar darken = 0;
			if (visibility) {
				if (not (*visibility) [f_index]) darken |= 1;
				if (not (*visibility) [r_index]) darken |= 2;
				if (not (*visibility)[bl_index]) darken |= 4;
				if (not (*visibility)[br_index]) darken |= 8;
			}
			const uchar roads =
				f.field->get_roads() | overlay_manager.get_road_overlay(f);

			m_ground_surface->draw_field //  Render ground
				(m_rect, f.field, r.field, bl.field, br.field, l.field, tr.field,
				 f_posx, r_posx, posy, bl_posx, br_posx, b_posy,
				 roads, darken, draw_all);
		}

		++linear_fy;
		row_is_forward = not row_is_forward;
	}

   // Copy ground where it belongs: on the screen
   m_surface->blit(Point( m_rect.x, m_rect.y), m_ground_surface, m_rect);

	{
		const int dx2 = maxfx - minfx + 1;
		int dy2 = maxfy - minfy + 1;
		int linear_fy2 = minfy;
		bool row_is_forward2 = linear_fy2 & 1;
		int b_posy2 = linear_fy2 * TRIANGLE_HEIGHT - viewofs.y;

		while(dy2--) {
			const int posy = b_posy2;
			b_posy2 += TRIANGLE_HEIGHT;

			{//  Draw things on the node.
				const int linear_fx = minfx;
				FCoords r(linear_fx, linear_fy2);
				FCoords br(linear_fx - (not row_is_forward2), linear_fy2 + 1);

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
				bool  r_is_visible = not visibility or (*visibility) [r_index];
				bool br_is_visible = not visibility or (*visibility)[br_index];
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
					const bool f_is_visible = r_is_visible;
					const bool bl_is_visible = br_is_visible;
					r_is_visible = not visibility or (*visibility) [r_index];
					br_is_visible = not visibility or (*visibility)[br_index];
					const Point f_pos = r_pos, bl_pos = br_pos;
					r_pos = Point
						(r_pos.x + TRIANGLE_WIDTH,
						 posy - r.field->get_height() * HEIGHT_FACTOR);
					br_pos = Point
						(br_pos.x + TRIANGLE_WIDTH,
						 b_posy2 - br.field->get_height() * HEIGHT_FACTOR);

					//  Render border markes on and halfway between border nodes.
					if (f_is_border) {
						const Player & player = *egbase.get_player(f_owner_number);
						const uint anim = player.get_tribe()->get_frontier_anim();
						if (f_is_visible)
							drawanim(f_pos.x, f_pos.y, anim, 0, &player);
						if
							((f_is_visible or r_is_visible)
							 and
							 r_owner_number == f_owner_number
							 and
							 (tr_owner_number != f_owner_number
							  or
							  br_owner_number != f_owner_number))
							drawanim
							((f_pos.x +  r_pos.x) / 2, (f_pos.y +  r_pos.y) / 2, anim, 0, &player);
						if
							((f_is_visible or bl_is_visible)
							 and
							 bl_owner_number == f_owner_number
							 and
							 (l_owner_number != f_owner_number
							  or
							  br_owner_number != f_owner_number))
							drawanim
							((f_pos.x + bl_pos.x) / 2, (f_pos.y + bl_pos.y) / 2, anim, 0, &player);
						if
							((f_is_visible or br_is_visible)
							 and
							 br_owner_number == f_owner_number
							 and
							 (r_owner_number != f_owner_number
							  or
							  bl_owner_number != f_owner_number))
							drawanim
							((f_pos.x + br_pos.x) / 2, (f_pos.y + br_pos.y) / 2, anim, 0, &player);
					}

					if (f_is_visible) { // Render stuff that belongs to the node

						// Render bobs
						// TODO - rendering order?
						//  This must be defined somehow. Some bobs have a higher
						//  priority than others. Maybe this priority is a moving
						//  versus non-moving bobs thing? draw_ground implies that
						//  this doesn't render map objects. Are there any overdraw
						//  issues with the current rendering order?

						// Draw Map_Objects hooked to this field
						BaseImmovable * const imm = f.field->get_immovable();

						if (imm) imm->draw(egbase, *this, f, f_pos);

						Bob *bob = f.field->get_first_bob();
						while(bob) {
							bob->draw(egbase, *this, f_pos);
							bob = bob->get_next_bob();
						}

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
							blit
							(f_pos.x - it->hotspot_x, f_pos.y - it->hotspot_y,
							 it->picid);
					}
				}
			}
			{//  Draw things on the R-triangle.
				const int linear_fx = minfx;
				FCoords r(linear_fx, linear_fy2);
				FCoords b(linear_fx - (not row_is_forward2), linear_fy2 + 1);
				int posx =
					(linear_fx - 1) * TRIANGLE_WIDTH
					+
					(row_is_forward2 + 1) * (TRIANGLE_WIDTH / 2)
					-
					viewofs.x;

				//  Calculate safe (bounded) field coordinates.
				map.normalize_coords(&r);
				map.normalize_coords(&b);

				;//  Get field pointers.
				r.field = &map[Map::get_index(r, mapwidth)];
				b.field = &map[Map::get_index(b, mapwidth)];

				int count = dx2;

				;//  One less iteration than for nodes and D-triangles.
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
						const Overlay_Manager::Overlay_Info * const overlay_info_end =
							overlay_info
							+
							overlay_manager.get_overlays
							(TCoords(f, TCoords::R), overlay_info);
						for
							(const Overlay_Manager::Overlay_Info * it = overlay_info;
							 it < overlay_info_end;
							 ++it)
						{
							blit
								(posx - it->hotspot_x,
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
								 3
								 -
								 it->hotspot_y,
								 it->picid);
						}
					}
				}
			}
			{//  Draw things on the D-triangle.
				const int linear_fx = minfx;
				FCoords f(linear_fx - 1, linear_fy2);
				FCoords br(linear_fx - (not row_is_forward2), linear_fy2 + 1);
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

					{ //  FIXME Visibility check here.
						Overlay_Manager::Overlay_Info overlay_info
								[MAX_OVERLAYS_PER_TRIANGLE];
						const Overlay_Manager::Overlay_Info * const overlay_info_end =
							overlay_info
							+
							overlay_manager.get_overlays
							(TCoords(f, TCoords::D), overlay_info);
						for
							(const Overlay_Manager::Overlay_Info * it = overlay_info;
							 it < overlay_info_end;
							 ++it)
						{
							blit
								(posx - it->hotspot_x,
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
								 3
								 -
								 it->hotspot_y,
								 it->picid);
						}
					}
				}
			}

			++linear_fy2;
			row_is_forward2 = not row_is_forward2;
		}
	}

   g_gr->reset_texture_animation_reminder();
}


/*
===============
RenderTargetImpl::renderminimap

Renders a minimap into the current window.
The field at viewpt will be in the top-left corner of the window.
flags specifies what information to display (see Minimap_XXX enums).
===============
*/
void RenderTargetImpl::renderminimap
(const Editor_Game_Base & egbase,
 const std::vector<bool> * const visibility,
 Coords viewpt,
 const uint flags)
{

	// Calculate the field at the top-left corner of the clipping rect
	viewpt.x -= m_offset.x;
	viewpt.y -= m_offset.y;

	//  The entire clipping rect will be used for drawing.
	m_surface->draw_minimap(egbase, visibility, m_rect, viewpt, flags);
}


/**
 * Draws a frame of an animation at the given location
 * Plays sound effect that is registered with this frame (the \ref Sound_Handler
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
void RenderTargetImpl::drawanim(int dstx, int dsty, uint animation, uint time, const Player* player)
{
	const AnimationData* data = g_anim.get_animation(animation);
	AnimationGfx* gfx = get_graphicimpl()->get_animation(animation);
	Rect rc;

	//TODO? assert(player);
	assert(data);
	assert(gfx);

	if (!data || !gfx) {
		log("WARNING: Animation %i doesn't exist\n", animation);
		return;
	}

	// Get the frame and its data
	Surface* frame;
	int framenumber = (time / data->frametime) % gfx->get_nrframes();
   if( player )
      frame = gfx->get_frame(framenumber, player->get_player_number(), player);
   else
      frame = gfx->get_frame(framenumber, 0, 0);
	dstx += m_offset.x;
	dsty += m_offset.y;

	dstx -= gfx->get_hotspot().x;
	dsty -= gfx->get_hotspot().y;

	rc.x = 0;
	rc.y = 0;
	rc.w = frame->get_w();
	rc.h = frame->get_h();


	// Clipping
	if (dstx < 0) {
		rc.x -= dstx;
		rc.w += dstx;
		dstx = 0;
	}
	if (dstx + rc.w > m_rect.w)
		rc.w = m_rect.w - dstx;
	if (rc.w <= 0)
		return;

	if (dsty < 0) {
		rc.y -= dsty;
		rc.h += dsty;
		dsty = 0;
	}
	if (dsty + rc.h > m_rect.h)
		rc.h = m_rect.h - dsty;
	if (rc.h <= 0)
		return;

	// Draw it
	m_surface->blit(Point(dstx + m_rect.x, dsty + m_rect.y), frame, rc);

	// Look if there's a sound effect registered for this frame and trigger the effect
	uint stereo_position=128; //see Sound_Handler::stereo_position()
		g_anim.trigger_soundfx(animation, framenumber, stereo_position);
}


/*
===============
RenderTargetImpl::drawanimrect

Draws a part of a frame of an animation at the given location
===============
*/
void RenderTargetImpl::drawanimrect(int dstx, int dsty, uint animation, uint time, const Player* player,
												int srcx, int srcy, int w, int h)
{
	const AnimationData* data = g_anim.get_animation(animation);
	AnimationGfx* gfx = get_graphicimpl()->get_animation(animation);
	Surface* frame;
	Rect rc;

	if (!data || !gfx) {
		log("WARNING: Animation %i doesn't exist\n", animation);
		return;
	}

	// Get the frame and its data
   if( player )
      frame = gfx->get_frame((time / data->frametime) % gfx->get_nrframes(), player->get_player_number(), player);
   else
      frame = gfx->get_frame((time / data->frametime) % gfx->get_nrframes(), 0, 0);

   dstx += m_offset.x;
	dsty += m_offset.y;

	dstx -= gfx->get_hotspot().x;
	dsty -= gfx->get_hotspot().y;

	dstx += srcx;
	dsty += srcy;

	rc.x = srcx;
	rc.y = srcy;
	rc.w = w; //frame->width;
	rc.h = h; //frame->height;

	// Clipping against source
	if (rc.x < 0) {
		dstx -= rc.x;
		rc.w += rc.x;
		rc.x = 0;
	}
	if (rc.x + rc.w > (int)frame->get_w())
		rc.w = frame->get_w() - rc.x;
	if (rc.w <= 0)
		return;

	if (rc.y < 0) {
		dsty -= rc.y;
		rc.h += rc.y;
		rc.y = 0;
	}
	if (rc.y + rc.h > (int)frame->get_h())
		rc.h = frame->get_h() - rc.y;
	if (rc.h <= 0)
		return;

	// Clipping against destination
	if (dstx < 0) {
		rc.x -= dstx;
		rc.w += dstx;
		dstx = 0;
	}
	if (dstx + rc.w > m_rect.w)
		rc.w = m_rect.w - dstx;
	if (rc.w <= 0)
		return;

	if (dsty < 0) {
		rc.y -= dsty;
		rc.h += dsty;
		dsty = 0;
	}
	if (dsty + rc.h > m_rect.h)
		rc.h = m_rect.h - dsty;
	if (rc.h <= 0)
		return;


	// Draw it
	m_surface->blit(Point(dstx + m_rect.x, dsty + m_rect.y), frame, rc);
}


/*
===============================================================================

GraphicImpl -- 16 bit software implementation of main graphics interface

===============================================================================
*/

/*
===============
GraphicImpl::GraphicImpl

Initialize the SDL video mode.
===============
*/
GraphicImpl::GraphicImpl(int w, int h, int bpp, bool fullscreen)
{
	m_nr_update_rects = 0;
	m_update_fullscreen = false;
   m_roadtextures = 0;

	// Set video mode using SDL
	int flags = SDL_SWSURFACE;

	if (fullscreen)
		flags |= SDL_FULLSCREEN;

	SDL_Surface* sdlsurface = SDL_SetVideoMode(w, h, bpp, flags);
	if (!sdlsurface)
		throw wexception("Couldn't set video mode: %s", SDL_GetError());

	assert( sdlsurface->format->BytesPerPixel == 2 || sdlsurface->format->BytesPerPixel == 4 );

	SDL_WM_SetCaption("Widelands " VERSION, "Widelands");

   m_screen.set_sdl_surface( sdlsurface );
	m_rendertarget = new RenderTargetImpl(&m_screen);
}

/*
===============
GraphicImpl::~GraphicImpl

Free the surface
===============
*/
GraphicImpl::~GraphicImpl()
{
   flush(0);

   if(m_roadtextures) {
      delete m_roadtextures;
      m_roadtextures = 0;
   }

	delete m_rendertarget;
	flush(-1);
}

/*
===============
GraphicImpl::get_xres
GraphicImpl::get_yres

Return the screen resolution
===============
*/
int GraphicImpl::get_xres()
{
	return m_screen.get_w();
}

int GraphicImpl::get_yres()
{
	return m_screen.get_h();
}

/*
===============
GraphicImpl::get_render_target

Return a pointer to the RenderTarget representing the screen
===============
*/
RenderTarget* GraphicImpl::get_render_target()
{
	m_rendertarget->reset();

	return m_rendertarget;
}

/*
===============
GraphicImpl::toggle_fullscreen

Switch from fullscreen to windowed mode or vice-versa
===============
*/
void GraphicImpl::toggle_fullscreen()
{
	SDL_Surface *surface = m_screen.get_sdl_surface();
	SDL_WM_ToggleFullScreen(surface);
}

/*
===============
GraphicImpl::update_fullscreen

Mark the entire screen for refreshing
===============
*/
void GraphicImpl::update_fullscreen()
{
	m_update_fullscreen = true;
}

/*
===============
GraphicImpl::update_rectangle

Mark a rectangle for refreshing
===============
*/
void GraphicImpl::update_rectangle(int x, int y, int w, int h)
{
	if (m_nr_update_rects >= MAX_RECTS)
		{
		m_update_fullscreen = true;
		return;
		}

	m_update_rects[m_nr_update_rects].x = x;
	m_update_rects[m_nr_update_rects].y = y;
	m_update_rects[m_nr_update_rects].w = w;
	m_update_rects[m_nr_update_rects].h = h;
	++m_nr_update_rects;
}

/*
===============
GraphicImpl::need_update

Returns true if parts of the screen have been marked for refreshing.
===============
*/
bool GraphicImpl::need_update()
{
	return m_nr_update_rects || m_update_fullscreen;
}

/*
===============
GraphicImpl::refresh

Bring the screen uptodate.
===============
*/
void GraphicImpl::refresh()
{
	//if (m_update_fullscreen)
   m_screen.update();

	//else SDL_UpdateRects(m_sdlsurface, m_nr_update_rects, m_update_rects);

	m_update_fullscreen = false;
	m_nr_update_rects = 0;
}


/*
===============
GraphicImpl::flush

Remove all resources (currently pictures) from the given modules.
If mod is 0, all pictures are flushed. FIXME  this seems to be wrong - sigra
===============
*/
void GraphicImpl::flush(int mod)
{
   uint i;

   // Flush pictures
   for(i = 0; i < m_pictures.size(); i++) {
      Picture* pic = &m_pictures[i];

//      NoLog("Flushing picture: %i while flushing all!\n", i);
      if (!pic->mod)
         continue;


      if (pic->mod < 0) {
         if (!mod)
            log("LEAK: SW16: flush(0): non-picture %i left.\n", i+1);
         continue;
      }

      pic->mod &= ~mod; // unmask the mods that should be flushed

      // Once the picture is no longer in any mods, free it
      if (!pic->mod) {

         if (pic->u.fname) {
            m_picturemap.erase(pic->u.fname);
            free(pic->u.fname);
         }
         if(pic->surface)
            delete pic->surface;
      }
   }

   // Flush game items
   if (!mod || mod & PicMod_Game) {
      for(i = 0; i < m_maptextures.size(); i++)
         delete m_maptextures[i];
      m_maptextures.resize(0);

      for(i = 0; i < m_animations.size(); i++)
         delete m_animations[i];
      m_animations.resize(0);

      if( m_roadtextures ) {
         delete m_roadtextures;
         m_roadtextures = 0;
      }
   }
   if(!mod || mod & PicMod_UI) {
      // Flush the cached Fontdatas
      g_fh->flush_cache();
   }
}


/*
===============
GraphicImpl::get_picture

Retrieves the picture ID of the picture with the given filename.
If the picture has already been loaded, the old ID is reused.
The picture is placed into the module(s) given by mod.

Returns 0 (a null-picture) if the picture cannot be loaded.
===============
*/
uint GraphicImpl::get_picture(int mod, const char* fname )
{
	std::vector<Picture>::size_type id;

	//  Check if the picture is already loaded.
	const picmap_t::const_iterator it = m_picturemap.find(fname);

	if (it != m_picturemap.end())
	{
		id = it->second;
	}
	else
	{
		SDL_Surface* bmp;

		try
		{
			bmp = LoadImage(fname);
         //log( "GraphicImpl::get_picture(): loading picture '%s'\n", fname);
		}
		catch(std::exception& e)
		{
			log("WARNING: Couldn't open %s: %s\n", fname, e.what());
			return 0;
		}

      // Convert the surface accordingly
		SDL_Surface* use_surface = SDL_DisplayFormatAlpha( bmp );
		SDL_FreeSurface(bmp);

      if( !use_surface )
         throw wexception("GraphicImpl::get_picture(): no success in converting loaded surface!\n");

		// Fill in a free slot in the pictures array
		id = find_free_picture();
		Picture & pic = m_pictures[id];
		pic.mod       = 0; // will be filled in by caller
		pic.u.fname   = strdup(fname);
		assert(pic.u.fname); //  FIXME no proper check for NULL return value!
		pic.surface   = new Surface();
		pic.surface->set_sdl_surface(use_surface);

		m_picturemap[fname] = id;
   }

	m_pictures[id].mod |= mod;

	return id;
}

uint GraphicImpl::get_picture(int mod, Surface* surf, const char* fname )
{
	const std::vector<Picture>::size_type id = find_free_picture();
	Picture & pic = m_pictures[id];
   pic.mod       = mod;
   pic.surface   = surf;
   if( fname ) {
		pic.u.fname = strdup(fname);
		m_picturemap[fname] = id;
   } else pic.u.fname =  0;

	return id;
}


/*
===============
GraphicImpl::get_picture_size

Stores the picture size in pw and ph.
Throws an exception if the picture doesn't exist.
===============
*/
void GraphicImpl::get_picture_size(const uint pic, uint & w, uint & h)
{
	if (pic >= m_pictures.size() || !m_pictures[pic].mod)
		throw wexception("get_picture_size(%i): picture doesn't exist", pic);

	Surface* bmp = m_pictures[pic].surface;


	w = bmp->get_w();
	h = bmp->get_h();
}


/*
===============
GraphicImpl::create_surface

Create an offscreen surface that can be used both as target and as source for
rendering operations. The surface is put into a normal slot in the picture
array so the surface can be used in normal blit() operations.
A RenderTarget for the surface can be obtained using get_surface_renderer().
Note that surfaces do not belong to a module and must be freed explicitly.
===============
*/
uint GraphicImpl::create_surface(int w, int h)
{

   SDL_Surface* surf = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, m_screen.get_format()->BitsPerPixel,
         m_screen.get_format()->Rmask, m_screen.get_format()->Gmask, m_screen.get_format()->Bmask,
         m_screen.get_format()->Amask);

	const std::vector<Picture>::size_type id = find_free_picture();
	Picture & pic = m_pictures[id];
	pic.mod       = -1; // mark as surface
	pic.surface   = new Surface();
	pic.surface->set_sdl_surface(surf);
	pic.u.rendertarget = new RenderTargetImpl(pic.surface);

	return id;
}

/*
===============
GraphicImpl::free_surface

Free the given surface.
Unlike normal pictures, surfaces are not freed by flush().
===============
*/
void GraphicImpl::free_surface(uint picid)
{
	assert(picid < m_pictures.size() && ( m_pictures[picid].mod == -1 || m_pictures[picid].mod == PicMod_Font));

	Picture* pic = &m_pictures[picid];

	delete pic->u.rendertarget;
	delete pic->surface;
	pic->mod = 0;
}


/*
===============
GraphicImpl::get_surface_renderer

Returns the RenderTarget for the given surface
===============
*/
RenderTarget* GraphicImpl::get_surface_renderer(uint pic)
{
	assert(pic < m_pictures.size() && m_pictures[pic].mod == -1);

	RenderTargetImpl* rt = m_pictures[pic].u.rendertarget;

	rt->reset();

	return rt;
}


/*
===============
GraphicImpl::get_picture_bitmap

Returns the bitmap that belongs to the given picture ID.
May return 0 if the given picture does not exist.
===============
*/
Surface* GraphicImpl::get_picture_surface(uint id)
{
	if (id >= m_pictures.size())
		return 0;

	if (!m_pictures[id].mod)
		return 0;

	return m_pictures[id].surface;
}


/*
===============
GraphicImpl::get_maptexture

Creates a terrain texture.
fnametempl is a filename with possible wildcard characters '?'. The function
fills the wildcards with decimal numbers to get the different frames of a
texture animation. For example, if fnametempl is "foo_??.bmp", it tries
"foo_00.bmp", "foo_01.bmp" etc...
frametime is in milliseconds.

Returns 0 if the texture couldn't be loaded.

Note: Terrain textures are not reused, even if fnametempl matches.
      These textures are freed when PicMod_Game is flushed.
===============
*/
uint GraphicImpl::get_maptexture(const char* fnametempl, uint frametime)
{
	try {
		Texture* tex = new Texture(fnametempl, frametime, m_screen.get_format());

		m_maptextures.push_back(tex);

		return m_maptextures.size(); // ID 1 is at m_maptextures[0]
	} catch(std::exception& e) {
		log("Failed to load maptexture %s: %s\n", fnametempl, e.what());
		return 0;
	}
}


/*
===============
GraphicImpl::animate_maptextures

Advance frames for animated textures
===============
*/
void GraphicImpl::animate_maptextures(uint time)
{
	for(uint i = 0; i < m_maptextures.size(); i++)
		m_maptextures[i]->animate(time);
}

/*
 * REset that the map texture have been animated
 */
void GraphicImpl::reset_texture_animation_reminder( void ) {
	for(uint i = 0; i < m_maptextures.size(); i++)
		m_maptextures[i]->reset_was_animated();
}

/*
===============
GraphicImpl::get_maptexture_data

Return the actual texture data associated with the given ID.
===============
*/
Texture* GraphicImpl::get_maptexture_data(uint id)
{
	id--; // ID 1 is at m_maptextures[0]

	if (id < m_maptextures.size())
		return m_maptextures[id];
	else
		return 0;
}

/*
================
GraphicImp::get_road_textures

returns the road textures
================
*/
Surface* GraphicImpl::get_road_texture( int roadtex) {
   if(! m_roadtextures ) {
      // Load the road textures
      m_roadtextures = new Road_Textures();
      m_roadtextures->pic_road_normal = get_picture(PicMod_Game, ROAD_NORMAL_PIC);
      m_roadtextures->pic_road_busy   = get_picture(PicMod_Game, ROAD_BUSY_PIC  );
      get_picture_surface( m_roadtextures->pic_road_normal )->force_disable_alpha();
      get_picture_surface( m_roadtextures->pic_road_busy )->force_disable_alpha();
   }
   if(roadtex == Road_Normal)
      return get_picture_surface(m_roadtextures->pic_road_normal);
   else
      return get_picture_surface(m_roadtextures->pic_road_busy);
}

/*
==============
GraphicImp::get_texture_picture

Return Filename of texture of given ID.
==============
*/
const char* GraphicImpl::get_maptexture_picture(uint id) {
	Texture* tex = get_maptexture_data(id);
	if (tex)
		return tex->get_texture_picture();
	else
		return 0;
}


/*
===============
GraphicImpl::load_animations

Load all animations that are registered with the AnimationManager
===============
*/
void GraphicImpl::load_animations()
{
	assert(!m_animations.size());

	for(uint id = 1; id <= g_anim.get_nranimations(); id++)
		m_animations.push_back(new AnimationGfx(g_anim.get_animation(id)));
}


/*
===============
GraphicImpl::get_animation

Retrieve the animation graphics
===============
*/
AnimationGfx* GraphicImpl::get_animation(uint anim)
{
	if (!anim || anim > m_animations.size())
		return 0;

	return m_animations[anim-1];
}

/*
 * Return the number of frames in this animation
 */
int GraphicImpl::get_animation_nr_frames(uint anim) {
   AnimationGfx* gfx=get_animation(anim);
   return gfx->get_nrframes();
}

/*
===============
GraphicImpl::get_animation_size

Return the size of the animation at the given time.
===============
*/
void GraphicImpl::get_animation_size(uint anim, uint time, int* pw, int* ph)
{
	const AnimationData* data = g_anim.get_animation(anim);
	AnimationGfx* gfx = get_graphicimpl()->get_animation(anim);
	Surface* frame;
	int w, h;

	if (!data || !gfx)
	{
		log("WARNING: Animation %i doesn't exist\n", anim);
		w = h = 0;
	}
	else
	{
		// Get the frame and its data
		frame = gfx->get_frame((time / data->frametime) % gfx->get_nrframes(), 0, 0); // Ignore playerclrs

		w = frame->get_w();
		h = frame->get_h();
	}

	if (pw)
		*pw = w;
	if (ph)
		*ph = h;

	return;
}


/*
===============
GraphicImpl::screenshot

Save a screenshot in the given file.
===============
*/
void GraphicImpl::screenshot(const char* fname)
{
	// TODO: this is incorrect; it bypasses the files code
   m_screen.save_bmp(fname);
}



/*
===============
GraphicImpl::find_free_picture

Find a free picture slot and return it.
===============
*/
std::vector<GraphicImpl::Picture>::size_type GraphicImpl::find_free_picture() {
	const std::vector<Picture>::size_type pictures_size = m_pictures.size();
	std::vector<Picture>::size_type id = 1;
	for (; id < pictures_size; ++id) if (m_pictures[id].mod == 0) return id;

	m_pictures.resize(id+1);

	return id;
}

/*
 * Save and load pictures
 */
void GraphicImpl::m_png_write_function( png_structp png_ptr, png_bytep data, png_size_t length ) {
   FileWrite* fw = static_cast<FileWrite*>(png_get_io_ptr(png_ptr));
   fw->Data( data, length );
}
void GraphicImpl::save_png(uint pic_index, FileWrite* fw) {
   Surface* surf = get_picture_surface( pic_index );

   // Save a png
   png_structp png_ptr = png_create_write_struct
      (PNG_LIBPNG_VER_STRING, (png_voidp)0,
       0, 0);
   if (!png_ptr)
      throw wexception("GraphicImpl::save_png: Couldn't create png struct!\n");

   // Set another write function
   png_set_write_fn( png_ptr, fw, &GraphicImpl::m_png_write_function, 0);

   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
   {
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      throw wexception("GraphicImpl::save_png: Couldn't create png info struct!\n");
   }

   // Set jump for error
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      throw wexception("GraphicImpl::save_png: Couldn't set png setjmp!\n");
   }

   // Fill info struct
   png_set_IHDR(png_ptr, info_ptr, surf->get_w(), surf->get_h(),
         8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
         PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

   // png_set_strip_16(png_ptr) ;

   // Start writing
   png_write_info(png_ptr, info_ptr);

   // Strip data down
   png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
   png_set_packing( png_ptr );

   png_bytep row = new png_byte[4*surf->get_w()];
   // Write each row
   for( uint y = 0; y < surf->get_h(); y++ ) {
      uint i = 0;
      for( uint x = 0; x < surf->get_w(); x++ ) {
         uchar r, g, b, a;
         SDL_GetRGBA( surf->get_pixel(x,y), surf->get_format(), &r, &g, &b, &a);
         row[i+0] = r;
         row[i+1] = g;
         row[i+2] = b;
         row[i+3] = a;
         i += 4;
      }
      png_write_row( png_ptr, row );
   }
   delete row;

   // End write
   png_write_end(png_ptr, info_ptr );
   png_destroy_write_struct(&png_ptr, &info_ptr);
}

/**
 * Factory function
 * \todo const-ify
 */
Graphic* SW16_CreateGraphics(int w, int h, int bpp, bool fullscreen)
{
	return new GraphicImpl(w, h, bpp, fullscreen);
}
