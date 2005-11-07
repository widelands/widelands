/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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
#include "filesystem.h"
#include "font_handler.h"
#include "map.h"
#include "player.h"
#include "graphic_impl.h"
#include "tribe.h"
#include "overlay_manager.h"
#include "filesystem.h"
#include "system.h"

/*
 * Names of road terrains
 */
#define ROAD_NORMAL_PIC "pics/roadt_normal.png"
#define ROAD_BUSY_PIC   "pics/roadt_busy.png"


/*
===============
LoadImage

Helper function wraps around SDL_image. Returns the given image file as a
surface.
Cannot return 0, throws an exception on error.
===============
*/
SDL_Surface* LoadImage(std::string filename)
{
	FileRead fr;
	SDL_Surface* surf;

	fr.Open(g_fs, filename);

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
draw_overlays

Draw build help, frontier and registered overlays
===============
*/
static void draw_overlays(RenderTargetImpl* dst, Editor_Game_Base* egbase, const std::vector<bool>* visibility, FCoords fc, Point pos,
                 FCoords fcr, Point posr, FCoords fcbl, Point posbl, FCoords fcbr, Point posbr)
{
	// Render frontier
   uchar player;
   if((player=egbase->get_map()->get_overlay_manager()->is_frontier_field(fc))) {
      Player *ownerplayer = egbase->get_player(player);
      uint anim = ownerplayer->get_tribe()->get_frontier_anim();

      dst->drawanim(pos.x, pos.y, anim, 0, ownerplayer);

      // check to the right
      if(egbase->get_map()->get_overlay_manager()->draw_border_to_right(fc))
         dst->drawanim((pos.x+posr.x)/2, (pos.y+posr.y)/2, anim, 0, ownerplayer);
      // check to the bottom left
      if(egbase->get_map()->get_overlay_manager()->draw_border_to_bottom_left(fc))
         dst->drawanim((pos.x+posbl.x)/2, (pos.y+posbl.y)/2, anim, 0, ownerplayer);
      // check to the bottom right
      if(egbase->get_map()->get_overlay_manager()->draw_border_to_right(fcbl))
         dst->drawanim((pos.x+posbr.x)/2, (pos.y+posbr.y)/2, anim, 0, ownerplayer);
   }

	// Draw normal buildhelp
   Overlay_Manager::Overlay_Info overlay_info[MAX_OVERLAYS_PER_FIELD];
   int num_overlays=egbase->get_map()->get_overlay_manager()->get_overlays(fc, overlay_info);

   int i;
   for(i=0; i<num_overlays; i++) {
      int x = pos.x - overlay_info[i].hotspot_x;
      int y = pos.y - overlay_info[i].hotspot_y;

      dst->blit(x,y,overlay_info[i].picid);
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
void RenderTargetImpl::rendermap(Editor_Game_Base* egbase, const std::vector<bool>* visibility, Point viewofs, bool draw_all)
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
	//Player *player = m_player->get_player();
	Map* map = egbase->get_map();
	int mapwidth = map->get_width();
	int minfx, minfy;
	int maxfx, maxfy;

	minfx = (viewofs.x + (FIELD_WIDTH>>1)) / FIELD_WIDTH - 1; // hack to prevent negative numbers
	minfy = viewofs.y / (FIELD_HEIGHT>>1);
	maxfx = (viewofs.x + (FIELD_WIDTH>>1) + m_rect.w) / FIELD_WIDTH;
	maxfy = (viewofs.y + m_rect.h) / (FIELD_HEIGHT>>1);
	maxfx += 1; // because of big buildings
	maxfy += 10; // necessary because of heights

	//log("%i %i -> %i %i\n", minfx, minfy, maxfx, maxfy);
	int dx = maxfx - minfx + 1;
	int dy = maxfy - minfy + 1;
   int linear_fy = minfy;

	while(dy--) {
		int linear_fx = minfx;
		FCoords f, bl, tl;
		int posx, posy;
		int blposx, bposy;
		int tlposx, tposy;

		// Use linear (non-wrapped) coordinates to calculate on-screen pos
		map->get_basepix(Coords(linear_fx, linear_fy), &posx, &posy);
		posx -= viewofs.x;
		posy -= viewofs.y;

		// Get linear bottom-left coordinate
		bl.y = linear_fy+1;
		bl.x = linear_fx - (bl.y&1);

		map->get_basepix(bl, &blposx, &bposy);
		blposx -= viewofs.x;
		bposy -= viewofs.y;

		// Get linear top-left coordinates
		tl.y = linear_fy-1;
		tl.x = linear_fx - (tl.y&1);

		map->get_basepix(tl, &tlposx, &tposy);
		tlposx -= viewofs.x;
		tposy -= viewofs.y;

		// Calculate safe (bounded) field coordinates and get field pointers
		f.x = linear_fx;
		f.y = linear_fy;
		map->normalize_coords(&f);
		map->normalize_coords(&bl);
		map->normalize_coords(&tl);

		f.field = map->get_field(f);
		bl.field = map->get_field(bl);
		tl.field = map->get_field(tl);

		int count = dx;
		while(count--) {
			FCoords br, r, l, tr;
			int rposx, brposx, lposx, trposx;
			uchar darken=0;

			map->get_rn(f, &r);
			rposx = posx + FIELD_WIDTH;

			map->get_ln(f, &l);
			lposx = posx - FIELD_WIDTH;

			map->get_rn(bl, &br);
			brposx = blposx + FIELD_WIDTH;

			map->get_rn(tl, &tr);
			trposx = tlposx + FIELD_WIDTH;

			if (visibility) {
				if (!(*visibility)[f.y*mapwidth + f.x]) darken|=1;
				if (!(*visibility)[r.y*mapwidth + r.x]) darken|=2;
				if (!(*visibility)[bl.y*mapwidth + bl.x]) darken|=4;
				if (!(*visibility)[br.y*mapwidth + br.x]) darken|=8;
			}

			// Render ground
			uchar roads = f.field->get_roads();

			roads |= egbase->get_map()->get_overlay_manager()->get_road_overlay(f);

			m_ground_surface->draw_field(m_rect, f.field, r.field, bl.field, br.field, l.field, tr.field,
				posx, rposx, posy, blposx, brposx, bposy, roads, darken, draw_all);
      
			// Advance to next field in row
			bl = br;
			blposx = brposx;

			f = r;
			posx = rposx;

			tl = tr;
			tlposx = trposx;
		}

		linear_fy++;
	}
   
   // Copy ground where it belongs: on the screen
   
   m_surface->blit(Point( m_rect.x, m_rect.y), m_ground_surface, m_rect); 
   
   dx = maxfx - minfx + 1;
	dy = maxfy - minfy + 1;
   linear_fy = minfy;

	while(dy--) {
		int linear_fx = minfx;
		FCoords f, bl, tl;
		int posx, posy;
		int blposx, bposy;
		int tlposx, tposy;

		// Use linear (non-wrapped) coordinates to calculate on-screen pos
		map->get_basepix(Coords(linear_fx, linear_fy), &posx, &posy);
		posx -= viewofs.x;
		posy -= viewofs.y;

		// Get linear bottom-left coordinate
		bl.y = linear_fy+1;
		bl.x = linear_fx - (bl.y&1);

		map->get_basepix(bl, &blposx, &bposy);
		blposx -= viewofs.x;
		bposy -= viewofs.y;

		// Get linear top-left coordinates
		tl.y = linear_fy-1;
		tl.x = linear_fx - (tl.y&1);

		map->get_basepix(tl, &tlposx, &tposy);
		tlposx -= viewofs.x;
		tposy -= viewofs.y;

		// Calculate safe (bounded) field coordinates and get field pointers
		f.x = linear_fx;
		f.y = linear_fy;
		map->normalize_coords(&f);
		map->normalize_coords(&bl);
		map->normalize_coords(&tl);

		f.field = map->get_field(f);
		bl.field = map->get_field(bl);
		tl.field = map->get_field(tl);

		int count = dx;
		while(count--) {
			FCoords br, r, l, tr;
			int rposx, brposx, lposx, trposx;
			uchar darken=0;

			map->get_rn(f, &r);
			rposx = posx + FIELD_WIDTH;

			map->get_ln(f, &l);
			lposx = posx - FIELD_WIDTH;

			map->get_rn(bl, &br);
			brposx = blposx + FIELD_WIDTH;

			map->get_rn(tl, &tr);
			trposx = tlposx + FIELD_WIDTH;

			if (visibility) {
				if (!(*visibility)[f.y*mapwidth + f.x]) darken|=1;
				if (!(*visibility)[r.y*mapwidth + r.x]) darken|=2;
				if (!(*visibility)[bl.y*mapwidth + bl.x]) darken|=4;
				if (!(*visibility)[br.y*mapwidth + br.x]) darken|=8;
			}

         
         // Render stuff that belongs to the field node
			if (!visibility || (*visibility)[f.y*mapwidth + f.x])
         {
            Point wh_pos(posx, posy - MULTIPLY_WITH_HEIGHT_FACTOR(f.field->get_height()));

            // Render bobs
            // TODO - rendering order?
            // This must be defined somewho. some bobs have a higher priority than others
            //  ^-- maybe this priority is a moving vs. non-moving bobs thing?
            // draw_ground implies that this doesn't render map objects.
            // are there any overdraw issues with the current rendering order?

            // Draw Map_Objects hooked to this field
            BaseImmovable *imm = f.field->get_immovable();

            assert(egbase);

            if (imm)
               imm->draw(egbase, this, f, wh_pos);

            Bob *bob = f.field->get_first_bob();
            while(bob) {
               bob->draw(egbase, this, wh_pos);
               bob = bob->get_next_bob();
            }

            // Draw buildhelp, road buildhelp
            draw_overlays(this, egbase, visibility, f, wh_pos,
                  r, Point(rposx, posy-MULTIPLY_WITH_HEIGHT_FACTOR(r.field->get_height())),
                  bl, Point(blposx, bposy-MULTIPLY_WITH_HEIGHT_FACTOR(bl.field->get_height())),
                  br, Point(brposx, bposy-MULTIPLY_WITH_HEIGHT_FACTOR(br.field->get_height())));
         }

			// Advance to next field in row
			bl = br;
			blposx = brposx;

			f = r;
			posx = rposx;

			tl = tr;
			tlposx = trposx;
		}

		linear_fy++;
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
void RenderTargetImpl::renderminimap(Editor_Game_Base* egbase, const std::vector<bool>* visibility, Coords viewpt, uint flags)
{
	Rect rc;

	// The entire clipping rect will be used for drawing
	rc = m_rect;

	// Calculate the field at the top-left corner of the clipping rect
	viewpt.x -= m_offset.x;
	viewpt.y -= m_offset.y;

	m_surface->draw_minimap(egbase, visibility, rc, viewpt, flags);
}


/*
===============
RenderTargetImpl::drawanim

Draws a frame of an animation at the given location
===============
*/
void RenderTargetImpl::drawanim(int dstx, int dsty, uint animation, uint time, const Player* player)
{
	const AnimationData* data = g_anim.get_animation(animation);
	AnimationGfx* gfx = get_graphicimpl()->get_animation(animation);
	Rect rc;

	if (!data || !gfx) {
		log("WARNING: Animation %i doesn't exist\n", animation);
		return;
	}

	// Get the frame and its data
	Surface* frame;
   if( player ) 
      frame = gfx->get_frame((time / data->frametime) % gfx->get_nrframes(), player->get_player_number(), player);
   else
      frame = gfx->get_frame((time / data->frametime) % gfx->get_nrframes(), 0, 0);
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
   
   assert( sdlsurface->format->BytesPerPixel == 2 || sdlsurface->format->BytesPerPixel == 4 );

	if (!sdlsurface)
		throw wexception("Couldn't set video mode: %s", SDL_GetError());

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
//	if (m_update_fullscreen)
   m_screen.update();

//	else
//		{
//		SDL_UpdateRects(m_sdlsurface, m_nr_update_rects, m_update_rects);
//		}

	m_update_fullscreen = false;
	m_nr_update_rects = 0;
}


/*
===============
GraphicImpl::flush

Remove all resources (currently pictures) from the given modules.
If mod is 0, all pictures are flushed.
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
	uint id;

	// Check if the picture's already loaded
	picmap_t::iterator it = m_picturemap.find(fname);

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

		// Fill in a free slot in the pictures array
		Picture* pic;

		id = find_free_picture();
		pic = &m_pictures[id];

      // Convert the surface accordingly
      SDL_Surface* use_surface = SDL_DisplayFormatAlpha( bmp );

      if( !use_surface ) 
         throw wexception("GraphicImpl::get_picture(): no success in converting loaded surface!\n");

      SDL_FreeSurface( bmp );

		pic->mod = 0; // will be filled in by caller
		pic->u.fname = strdup(fname);
		pic->surface = new Surface();
      pic->surface->set_sdl_surface( use_surface );

		m_picturemap[pic->u.fname] = id;
   }

	m_pictures[id].mod |= mod;

	return id;
}

uint GraphicImpl::get_picture(int mod, Surface* surf, const char* fname ) 
{
	uint id = find_free_picture();
	Picture* pic = &m_pictures[id];
   pic->mod = mod;
   pic->surface = surf;
   if( fname ) {
      pic->u.fname = strdup( fname );
		m_picturemap[pic->u.fname] = id;
   } else
      pic->u.fname =  0;

	return id;
}


/*
===============
GraphicImpl::get_picture_size

Stores the picture size in pw and ph.
Throws an exception if the picture doesn't exist.
===============
*/
void GraphicImpl::get_picture_size(uint pic, int* pw, int* ph)
{
	if (pic >= m_pictures.size() || !m_pictures[pic].mod)
		throw wexception("get_picture_size(%i): picture doesn't exist", pic);

	Surface* bmp = m_pictures[pic].surface;
   

	*pw = bmp->get_w();
	*ph = bmp->get_h();
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
	uint id = find_free_picture();
	Picture* pic = &m_pictures[id];
      
   SDL_Surface* surf = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, m_screen.get_format()->BitsPerPixel, 
         m_screen.get_format()->Rmask, m_screen.get_format()->Gmask, m_screen.get_format()->Bmask, 
         m_screen.get_format()->Amask);

	pic->mod = -1; // mark as surface
	pic->surface = new Surface();
   pic->surface->set_sdl_surface( surf ); 
	pic->u.rendertarget = new RenderTargetImpl(pic->surface);

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
uint GraphicImpl::find_free_picture()
{
	uint id;

	for(id = 1; id < m_pictures.size(); id++)
		if (!m_pictures[id].mod)
			return id;

	m_pictures.resize(id+1);

	return id;
}

/*
 * GraphicImpl::flush_picture(int)
 */
void GraphicImpl::flush_picture(uint pic_index) {
   Picture* pic = &m_pictures[pic_index];


   if (pic->u.fname) {
      m_picturemap.erase(pic->u.fname);
      free(pic->u.fname);
      pic->u.fname=0;
   }
   delete pic->surface;
   pic->surface = 0;
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

/*
===============
SW16_CreateGraphics

Factory function called by System code
===============
*/
Graphic* SW16_CreateGraphics(int w, int h, int bpp, bool fullscreen)
{
	return new GraphicImpl(w, h, bpp, fullscreen);
}
