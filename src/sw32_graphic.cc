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
Management classes and functions of the 32-bit software renderer.
*/

#include <SDL_image.h>
#include <string>
#include "bob.h"
#include "editor_game_base.h"
#include "error.h"
#include "filesystem.h"
#include "map.h"
#include "maprenderinfo.h"
#include "player.h"
#include "sw32_graphic.h"
#include "tribe.h"
#include "wexception.h"
#include "overlay_manager.h"

#ifdef OPENGL_MODE
  #include <GL/gl.h>
#endif

namespace Renderer_Software32
{


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
RenderTargetImpl::RenderTargetImpl(Bitmap* bmp)
{
	m_bitmap = bmp;

	reset();
}


/*
===============
RenderTargetImpl::~RenderTargetImpl
===============
*/
RenderTargetImpl::~RenderTargetImpl()
{
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
	m_rect.w = m_bitmap->w;
	m_rect.h = m_bitmap->h;

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
	if (m_rect.x + m_rect.w > m_bitmap->w)
		m_rect.w = m_bitmap->w - m_rect.x;
	if (m_rect.w < 0)
		m_rect.w = 0;

	if (m_rect.y < 0) {
		m_offset.y += m_rect.y;
		m_rect.h += m_rect.y;
		m_rect.y = 0;
	}
	if (m_rect.y + m_rect.h > m_bitmap->h)
		m_rect.h = m_bitmap->h - m_rect.y;
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
	return m_bitmap->w;
}

int RenderTargetImpl::get_h() const
{
	return m_bitmap->h;
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

	m_bitmap->draw_rect(Rect(x + m_rect.x, y + m_rect.y, w, h), clr);
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

	m_bitmap->fill_rect(Rect(x + m_rect.x, y + m_rect.y, w, h), clr);
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

	m_bitmap->brighten_rect(Rect(x + m_rect.x, y + m_rect.y, w, h), factor);
}

void RenderTargetImpl::clear(void)
{
	if (!m_rect.x && !m_rect.y && m_rect.w == m_bitmap->w && m_rect.h == m_bitmap->h)
		m_bitmap->clear();
	else
		m_bitmap->fill_rect(Rect(m_rect.x, m_rect.y, m_rect.w, m_rect.h), RGBColor(0,0,0));
}


/*
===============
RenderTargetImpl::doblit

Clip against window and source bitmap, then call the Bitmap blit routine.
===============
*/
void RenderTargetImpl::doblit(Point dst, Bitmap* src, Rect srcrc)
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
	m_bitmap->blit(Point(dst.x + m_rect.x, dst.y + m_rect.y), src, srcrc);
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
	Bitmap* src = gfx->get_picture_bitmap(picture);

	if (src)
		doblit(Point(dstx, dsty), src, Rect(0, 0, src->w, src->h));
}

void RenderTargetImpl::blitrect(int dstx, int dsty, uint picture,
	                             int srcx, int srcy, int w, int h)
{
	GraphicImpl* gfx = get_graphicimpl();
	Bitmap* src = gfx->get_picture_bitmap(picture);

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
	Bitmap* src = gfx->get_picture_bitmap(picture);

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
	ofsx = ofsx % src->w;
	if (ofsx < 0)
		ofsx += src->w;

	ofsy = ofsy % src->h;
	if (ofsy < 0)
		ofsy += src->h;

	// Blit the picture into the rectangle
	int ty = 0;

	while(ty < h)
		{
		int tx = 0;
		int tofsx = ofsx;
		Rect srcrc;

		srcrc.y = ofsy;
		srcrc.h = src->h - ofsy;
		if (ty + srcrc.h > h)
			srcrc.h = h - ty;

		while(tx < w)
			{
			srcrc.x = tofsx;
			srcrc.w = src->w - tofsx;
			if (tx + srcrc.w > w)
				srcrc.w = w - tx;

			m_bitmap->blit(Point(m_rect.x + x + tx, m_rect.y + y + ty), src, srcrc);

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

Draw build help (buildings and roads) and the field sel
===============
*/
static void draw_overlays(RenderTargetImpl* dst, const MapRenderInfo* mri, FCoords fc, Point pos,
                 FCoords fcr, Point posr, FCoords fcbl, Point posbl, FCoords fcbr, Point posbr)
{
	// Render frontier
   uchar player;
   if((player=mri->egbase->get_map()->get_overlay_manager()->is_frontier_field(fc))) {
      Player *ownerplayer = mri->egbase->get_player(player); 
      uint anim = ownerplayer->get_tribe()->get_frontier_anim();
      const RGBColor* playercolors = ownerplayer->get_playercolor();

      dst->drawanim(pos.x, pos.y, anim, 0, playercolors);

      // check to the right
      if(mri->egbase->get_map()->get_overlay_manager()->draw_border_to_right(fc)) 
         dst->drawanim((pos.x+posr.x)/2, (pos.y+posr.y)/2, anim, 0, playercolors);
      // check to the bottom left 
      if(mri->egbase->get_map()->get_overlay_manager()->draw_border_to_bottom_left(fc)) 
         dst->drawanim((pos.x+posbl.x)/2, (pos.y+posbl.y)/2, anim, 0, playercolors);
      // check to the bottom right
      if(mri->egbase->get_map()->get_overlay_manager()->draw_border_to_right(fcbl)) 
         dst->drawanim((pos.x+posbr.x)/2, (pos.y+posbr.y)/2, anim, 0, playercolors);
   }
   
	// Draw normal buildhelp 
   Overlay_Manager::Overlay_Info overlay_info[MAX_OVERLAYS_PER_FIELD];
   int num_overlays=mri->egbase->get_map()->get_overlay_manager()->get_overlays(fc, overlay_info);

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
===============
*/
void RenderTargetImpl::rendermap(const MapRenderInfo* mri, Point viewofs)
{
	Bitmap dst;

	viewofs.x -= m_offset.x;
	viewofs.y -= m_offset.y;

	dst.pixels = &m_bitmap->pixels[m_rect.y * m_bitmap->pitch + m_rect.x];
	dst.pitch = m_bitmap->pitch;
	dst.w = m_rect.w;
	dst.h = m_rect.h;
	dst.hasclrkey = false; // should be irrelevant

	get_graphicimpl()->allocate_gameicons();

	// Completely clear the window
	//dst.clear();

	// Actually draw the map. Loop through fields row by row
	// For each field, draw ground textures, then roads, then immovables
	// (and borders), then bobs, then overlay stuff (build icons etc...)
	//Player *player = m_player->get_player();
	Map* map = mri->egbase->get_map();
	int mapwidth = map->get_width();
	int minfx, minfy;
	int maxfx, maxfy;

	minfx = (viewofs.x + (FIELD_WIDTH>>1)) / FIELD_WIDTH - 1; // hack to prevent negative numbers
	minfy = viewofs.y / (FIELD_HEIGHT>>1);
	maxfx = (viewofs.x + (FIELD_WIDTH>>1) + dst.w) / FIELD_WIDTH;
	maxfy = (viewofs.y + dst.h) / (FIELD_HEIGHT>>1);
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
			bool render_r=true;
			bool render_b=true;

			map->get_rn(f, &r);
			rposx = posx + FIELD_WIDTH;

			map->get_ln(f, &l);
			lposx = posx - FIELD_WIDTH;

			map->get_rn(bl, &br);
			brposx = blposx + FIELD_WIDTH;

			map->get_rn(tl, &tr);
			trposx = tlposx + FIELD_WIDTH;

			if (mri->visibility) {
				if (!(*mri->visibility)[f.y*mapwidth + f.x] ||
					 !(*mri->visibility)[br.y*mapwidth + br.x]) {
					render_r=false;
					render_b=false;
				} else {
					if(!(*mri->visibility)[bl.y*mapwidth + bl.x])
						render_b=false;
					if(!(*mri->visibility)[r.y*mapwidth + r.x])
						render_r=false;
				}
			}

			// Render stuff that belongs to ground triangles
			uchar roads = f.field->get_roads();

			roads |= mri->egbase->get_map()->get_overlay_manager()->get_road_overlay(f);

			dst.draw_field(f.field, r.field, bl.field, br.field, l.field, tr.field,
					posx, rposx, posy, blposx, brposx, bposy, roads, render_r, render_b);

         // Render stuff that belongs to the field node
			if (!mri->visibility || (*mri->visibility)[f.y*mapwidth + f.x])
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

            if (imm)
               imm->draw(mri->egbase, this, f, wh_pos);

            Bob *bob = f.field->get_first_bob();
            while(bob) {
               bob->draw(mri->egbase, this, wh_pos);
               bob = bob->get_next_bob();
            }

            // Draw buildhelp, road buildhelp and fieldsel
            draw_overlays(this, mri, f, wh_pos,
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
}


/*
===============
RenderTargetImpl::renderminimap

Renders a minimap into the current window.
The field at viewpt will be in the top-left corner of the window.
flags specifies what information to display (see Minimap_XXX enums).
===============
*/
void RenderTargetImpl::renderminimap(const MapRenderInfo* mri, Coords viewpt, uint flags)
{
	Rect rc;

	// The entire clipping rect will be used for drawing
	rc = m_rect;

	// Calculate the field at the top-left corner of the clipping rect
	viewpt.x -= m_offset.x;
	viewpt.y -= m_offset.y;

	m_bitmap->draw_minimap(mri, rc, viewpt, flags);
}


/*
===============
RenderTargetImpl::drawanim

Draws a frame of an animation at the given location
===============
*/
void RenderTargetImpl::drawanim(int dstx, int dsty, uint animation, uint time, const RGBColor* plrclrs)
{
	const AnimationData* data = g_anim.get_animation(animation);
	const AnimationGfx* gfx = get_graphicimpl()->get_animation(animation);
	const AnimFrame* frame;
	Rect rc;

	if (!data || !gfx) {
		log("WARNING: Animation %i doesn't exist\n", animation);
		return;
	}

	// Get the frame and its data
	frame = gfx->get_frame((time / data->frametime) % gfx->get_nrframes());
	dstx += m_offset.x;
	dsty += m_offset.y;

	dstx -= frame->hotspot.x;
	dsty -= frame->hotspot.y;

	rc.x = 0;
	rc.y = 0;
	rc.w = frame->width;
	rc.h = frame->height;


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
	m_bitmap->draw_animframe(Point(dstx + m_rect.x, dsty + m_rect.y), frame, rc, plrclrs);
}


/*
===============
RenderTargetImpl::drawanimrect

Draws a part of a frame of an animation at the given location
===============
*/
void RenderTargetImpl::drawanimrect(int dstx, int dsty, uint animation, uint time, const RGBColor* plrclrs,
												int srcx, int srcy, int w, int h)
{
	const AnimationData* data = g_anim.get_animation(animation);
	const AnimationGfx* gfx = get_graphicimpl()->get_animation(animation);
	const AnimFrame* frame;
	Rect rc;

	if (!data || !gfx) {
		log("WARNING: Animation %i doesn't exist\n", animation);
		return;
	}

	// Get the frame and its data
	frame = gfx->get_frame((time / data->frametime) % gfx->get_nrframes());
	dstx += m_offset.x;
	dsty += m_offset.y;

	dstx -= frame->hotspot.x;
	dsty -= frame->hotspot.y;

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
	if (rc.x + rc.w > frame->width)
		rc.w = frame->width - rc.x;
	if (rc.w <= 0)
		return;

	if (rc.y < 0) {
		dsty -= rc.y;
		rc.h += rc.y;
		rc.y = 0;
	}
	if (rc.y + rc.h > frame->height)
		rc.h = frame->height - rc.y;
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
	m_bitmap->draw_animframe(Point(dstx + m_rect.x, dsty + m_rect.y), frame, rc, plrclrs);
}


/*
===============================================================================

GraphicImpl -- 32 bit software implementation of main graphics interface

===============================================================================
*/

/*
===============
GraphicImpl::GraphicImpl

Initialize the SDL video mode.
===============
*/
GraphicImpl::GraphicImpl(int w, int h, bool fullscreen)
{
	m_nr_update_rects = 0;
	m_update_fullscreen = false;

	m_gameicons = 0;

	// Set video mode using SDL
	int flags;

#ifdef OPENGL_MODE
	flags = SDL_OPENGL;

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
#else
	flags = SDL_SWSURFACE | SDL_DOUBLEBUF;
#endif

	if (fullscreen)
		flags |= SDL_FULLSCREEN;

	m_sdlsurface = SDL_SetVideoMode(w, h, 32, flags);
	if (!m_sdlsurface)
		throw wexception("Couldn't set video mode: %s", SDL_GetError());

	SDL_WM_SetCaption("Widelands " VERSION, "Widelands");

	//log("Original flags: %08x, final flags: %08x\n", flags, m_sdlsurface->flags);

#ifdef OPENGL_MODE
	m_screen.pixels = (uint*)malloc(1024*1024*sizeof(uint));
	memset(m_screen.pixels, 128, 1024*512*sizeof(uint));

	glBindTexture(GL_TEXTURE_2D, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_BGRA, GL_UNSIGNED_BYTE, m_screen.pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	free(m_screen.pixels);

	m_screen.pixels = (uint*)malloc(w*h*sizeof(uint));
	m_screen.pitch = w;
#else
	m_screen.pixels = (uint*)m_sdlsurface->pixels;
	m_screen.pitch = m_sdlsurface->pitch / sizeof(uint);
#endif
	m_screen.w = w;
	m_screen.h = h;

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
#ifdef OPENGL_MODE
	free(m_screen.pixels);
#endif

	flush(0);

	delete m_rendertarget;
	SDL_FreeSurface(m_sdlsurface);
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
	return m_sdlsurface->w;
}

int GraphicImpl::get_yres()
{
	return m_sdlsurface->h;
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
#if 0
	// only refresh every 16 frames
	static int counter = 0;

	counter++;
	if (counter & 31)
		return;
#endif

#ifdef OPENGL_MODE
	glViewport(0, 0, m_screen.w, m_screen.h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_screen.w, m_screen.h, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_screen.w, m_screen.h,
			GL_BGRA, GL_UNSIGNED_BYTE, m_screen.pixels);


	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(0, 0);
		glTexCoord2f((float)m_screen.w / 1024.0, 0);
		glVertex2i(m_screen.w, 0);
		glTexCoord2f((float)m_screen.w / 1024.0, (float)m_screen.h / 1024.0);
		glVertex2i(m_screen.w, m_screen.h);
		glTexCoord2f(0, (float)m_screen.h / 1024.0);
		glVertex2i(0, m_screen.h);
	glEnd();

	SDL_GL_SwapBuffers();
#else
//	if (m_update_fullscreen)
		//SDL_UpdateRect(m_sdlsurface, 0, 0, 0, 0);
		SDL_Flip(m_sdlsurface);
//	else
//		{
//		SDL_UpdateRects(m_sdlsurface, m_nr_update_rects, m_update_rects);
//		}
#endif

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

		if (!pic->mod)
			continue;

		if (pic->mod < 0) {
			if (!mod)
				log("LEAK: SW32: flush(0): non-picture %i left.\n", i+1);
			continue;
		}

		pic->mod &= ~mod; // unmask the mods that should be flushed

		// Once the picture is no longer in any mods, free it
		if (!pic->mod) {
			m_picturemap.erase(pic->u.fname);

			if (pic->u.fname)
				free(pic->u.fname);
			free(pic->bitmap.pixels);
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

		if (m_gameicons) {
			delete m_gameicons;
			m_gameicons = 0;
		}
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
uint GraphicImpl::get_picture(int mod, const char* fname)
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
		}
		catch(std::exception& e)
		{
			log("WARNING: Couldn't open %s: %s\n", fname, e.what());
			return 0;
		}

		SDL_Surface* cv = SDL_ConvertSurface(bmp, m_sdlsurface->format, 0);

		// Fill in a free slot in the pictures array
		Picture* pic;

		id = find_free_picture();
		pic = &m_pictures[id];

		pic->mod = 0; // will be filled in by caller
		pic->u.fname = strdup(fname);
		pic->bitmap.pixels = (uint*)malloc(cv->w*cv->h*4);
		pic->bitmap.w = cv->w;
		pic->bitmap.h = cv->h;
		pic->bitmap.pitch = cv->w;
		pic->bitmap.hasclrkey = false;

		for(int y = 0; y < cv->h; y++) {
			uint* src = (uint*)((Uint8*)cv->pixels + y*cv->pitch);
			uint* dst = pic->bitmap.pixels + y*cv->w;

			for(int x = 0; x < cv->w; x++)
				dst[x] = src[x] & 0x00FFFFFF;
		}

		SDL_FreeSurface(cv);
		SDL_FreeSurface(bmp);

		m_picturemap[pic->u.fname] = id;
	}

	m_pictures[id].mod |= mod;
	return id;
}

uint GraphicImpl::get_picture(int mod, const char* fname, RGBColor clrkey)
{
	uint id = get_picture(mod, fname);

	if (id) {
		m_pictures[id].bitmap.hasclrkey = true;
		m_pictures[id].bitmap.clrkey = clrkey.pack32();
	}

	return id;
}

// TODO: get rid of this function (needs change of font format)
uint GraphicImpl::get_picture(int mod, int w, int h, const ushort* data, RGBColor clrkey)
{
	uint id = find_free_picture();
	Picture* pic = &m_pictures[id];

	pic->mod = mod;
	pic->u.fname = 0;
	pic->bitmap.pixels = (uint*)malloc(w*h*4);
	pic->bitmap.w = w;
	pic->bitmap.h = h;
	pic->bitmap.pitch = w;
	pic->bitmap.hasclrkey = true;
	pic->bitmap.clrkey = clrkey.pack32();

	for(int count = w*h, i = 0; i < count; ++i) {
		RGBColor clr;
		clr.unpack16(data[i]);
		pic->bitmap.pixels[i] = clr.pack32();
	}

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

	Bitmap* bmp = &m_pictures[pic].bitmap;

	*pw = bmp->w;
	*ph = bmp->h;
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

	pic->mod = -1; // mark as surface
	pic->bitmap.pixels = (uint*)malloc(w*h*sizeof(uint));
	pic->bitmap.w = w;
	pic->bitmap.h = h;
	pic->bitmap.pitch = w;
	pic->bitmap.hasclrkey = false;
	pic->u.rendertarget = new RenderTargetImpl(&pic->bitmap);

	return id;
}

uint GraphicImpl::create_surface(int w, int h, RGBColor clrkey)
{
	uint id = create_surface(w, h);
	Picture* pic = &m_pictures[id];

	pic->bitmap.hasclrkey = true;
	pic->bitmap.clrkey = clrkey.pack32();

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
	assert(picid < m_pictures.size() && m_pictures[picid].mod == -1);

	Picture* pic = &m_pictures[picid];

	delete pic->u.rendertarget;
	free(pic->bitmap.pixels);
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
Bitmap* GraphicImpl::get_picture_bitmap(uint id)
{
	if (id >= m_pictures.size())
		return 0;

	if (!m_pictures[id].mod)
		return 0;

	return &m_pictures[id].bitmap;
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
		Texture* tex = new Texture(fnametempl, frametime);

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
===============
GraphicImpl::get_animation_size

Return the size of the animation at the given time.
===============
*/
void GraphicImpl::get_animation_size(uint anim, uint time, int* pw, int* ph)
{
	const AnimationData* data = g_anim.get_animation(anim);
	const AnimationGfx* gfx = get_graphicimpl()->get_animation(anim);
	const AnimFrame* frame;
	int w, h;

	if (!data || !gfx)
	{
		log("WARNING: Animation %i doesn't exist\n", anim);
		w = h = 0;
	}
	else
	{
		// Get the frame and its data
		frame = gfx->get_frame((time / data->frametime) % gfx->get_nrframes());

		w = frame->width;
		h = frame->height;
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
   SDL_SaveBMP(m_sdlsurface, fname);
}


/*
===============
GraphicImpl::allocate_gameicons

Allocate the pictures used by rendermap()
===============
*/
void GraphicImpl::allocate_gameicons()
{
	static const char* roadb_names[3] = {
		"pics/roadb_green.png",
		"pics/roadb_yellow.png",
		"pics/roadb_red.png"
	};
	static const char* build_names[5] = {
		"pics/set_flag.png",
		"pics/small.png",
		"pics/medium.png",
		"pics/big.png",
		"pics/mine.png"
	};

	if (m_gameicons)
		return;

	m_gameicons = new GameIcons;

	for(int i = 0; i < 3; i++)
		m_gameicons->pics_roadb[i] = g_gr->get_picture(PicMod_Game, roadb_names[i], RGBColor(0,0,255));
	for(int i = 0; i < 5; i++)
		m_gameicons->pics_build[i] = g_gr->get_picture(PicMod_Game, build_names[i], RGBColor(0,0,255));
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


} // namespace Renderer_Software32


/*
===============
SW32_CreateGraphics

Factory function called by System code
===============
*/
Graphic* SW32_CreateGraphics(int w, int h, bool fullscreen)
{
	return new Renderer_Software32::GraphicImpl(w, h, fullscreen);
}
