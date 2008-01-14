/*
 * Copyright (C) 2002-2004, 2007-2008 by the Widelands Development Team
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
Rendering functions of the 16-bit software renderer.
*/

#include "building.h"
#include "editor_game_base.h"
#include "filesystem.h"
#include "layered_filesystem.h"
#include "map.h"
#include "minimap.h"
#include "player.h"
#include "rgbcolor.h"
#include "graphic.h"
#include "transport.h"
#include "wexception.h"
#include "world.h"

#include "log.h"

#include "upcast.h"

#include <SDL.h>

using Widelands::Flag;
using Widelands::PlayerImmovable;
using Widelands::Road;

/*
 * Create a Surface from a surface
 */
Surface::Surface(const Surface& surf) {
   m_w = surf.m_w;
   m_h = surf.m_h;
   m_surface = SDL_DisplayFormat(surf.m_surface); // HACK: assume this should be picture format; there is no SDL_CopySurface
}

/*
 * Updating the whole Surface
 */
void Surface::update() {
   SDL_UpdateRect(m_surface, 0, 0, 0, 0);
}

/*
 * Save a bitmap
 */
void Surface::save_bmp(const char & fname) const {
   assert(m_surface);
	SDL_SaveBMP(m_surface, &fname);
}

/*
 * disable alpha for this surface. Needed
 * if you want to copy directly to the screen
 * by direct pixel access. For example for road
 * textures
 */
void Surface::force_disable_alpha() {
   SDL_Surface* newsur = SDL_DisplayFormat(m_surface);
   SDL_FreeSurface(m_surface);
   m_surface = newsur;
}

/*
===============
Surface::draw_rect

Draws the outline of a rectangle
===============
*/
void Surface::draw_rect(const Rect rc, const RGBColor clr) {
   assert(m_surface);
	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 1);
	assert(rc.h >= 1);
	const uint32_t color = clr.map(format());

	const Point bl = rc.bottom_left() - Point(1, 1);

	for (int32_t x = rc.x + 1; x < bl.x; ++x) {
		set_pixel(x, rc.y, color);
		set_pixel(x, bl.y, color);
	}
	for (int32_t y = rc.y; y <= bl.y; ++y) {
		set_pixel(rc.x, y, color);
		set_pixel(bl.x, y, color);
	}
}


/*
===============
Surface::fill_rect

Draws a filled rectangle
===============
*/
void Surface::fill_rect(const Rect rc, const RGBColor clr) {
   assert(m_surface);
	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 1);
	assert(rc.h >= 1);
	const uint32_t color = clr.map(format());

   SDL_Rect r = {rc.x, rc.y, rc.w, rc.h};
   SDL_FillRect(m_surface, &r, color);
}

/*
===============
Surface::brighten_rect

Change the brightness of the given rectangle
This function is slow as hell.
===============
*/
void Surface::brighten_rect(const Rect rc, const int32_t factor) {
	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 1);
	assert(rc.h >= 1);
	const Point bl = rc.bottom_left();
	for (int32_t y = rc.y; y < bl.y; ++y) for (int32_t x = rc.x; x < bl.x; ++x) {
         uint8_t gr, gg, gb;
         int16_t r, g, b;
         uint32_t clr = get_pixel(x, y);
         SDL_GetRGB(clr, m_surface->format, &gr, &gg, &gb);
         r = gr + factor;
         g = gg + factor;
         b = gb + factor;
		if (b & 0xFF00) b = (~b) >> 24;
		if (g & 0xFF00) g = (~g) >> 24;
		if (r & 0xFF00) r = (~r) >> 24;
         clr = SDL_MapRGB(m_surface->format, r, g, b);
         set_pixel(x, y, clr);
	}
}


/*
===============
Surface::clear

Clear the entire bitmap to black
===============
*/
void Surface::clear() {
	SDL_FillRect(m_surface, 0, 0);
}

/*
===============
Surface::blit

Blit this given source bitmap to this bitmap.
===============
*/
void Surface::blit(Point dst, Surface* src, Rect srcrc)
{
   SDL_Rect srcrect = {srcrc.x, srcrc.y, srcrc.w, srcrc.h};
   SDL_Rect dstrect = {dst.x, dst.y, 0, 0};

   SDL_BlitSurface(src->m_surface, &srcrect, m_surface, &dstrect);
}

/*
 * Fast blit, simply copy the source to the destination
 */
void Surface::fast_blit(Surface* src) {
   SDL_BlitSurface(src->m_surface, 0, m_surface, 0);
}

/*
 * Blend to colors; only needed for calc_minimap_color below
 */
inline static uint32_t blend_color
(const SDL_PixelFormat & format,
 const uint32_t clr1,
 const Uint8 r2, const Uint8 g2, const Uint8 b2)
{
	Uint8 r1, g1, b1;
	SDL_GetRGB(clr1, &const_cast<SDL_PixelFormat &>(format), &r1, &g1, &b1);
	return SDL_MapRGB
		(&const_cast<SDL_PixelFormat &>(format),
		 (r1 + r2) / 2, (g1 + g2) / 2, (b1 + b2) / 2);
}

/*
===============
calc_minimap_color

Return the color to be used in the minimap for the given field.
===============
*/
inline static uint32_t calc_minimap_color
(const SDL_PixelFormat & format,
 Widelands::Editor_Game_Base const & egbase,
 Widelands::FCoords          const   f,
 const uint32_t flags,
 Widelands::Player_Number    const owner,
 const bool see_details)
{
	uint32_t pixelcolor = 0;

	if (flags & MiniMap::Terrn) {
		pixelcolor =
			g_gr->
			get_maptexture_data
			(egbase.map().world()
			 .terrain_descr(f.field->terrain_d()).get_texture())
			->get_minimap_color(f.field->get_brightness());
	}

	if (flags & MiniMap::Owner) {
		if (0 < owner) { //  If owned, get the player's color...
			const RGBColor * const playercolors =
				egbase.player(owner).get_playercolor();

			//  ...and add the player's color to the old color.
			pixelcolor = blend_color
				(format,
				 pixelcolor,
				 playercolors[3].r(),  playercolors[3].g(), playercolors[3].b());
		}
	}

	if (see_details) {
	upcast(PlayerImmovable const, immovable, f.field->get_immovable());
	if (flags & MiniMap::Roads and dynamic_cast<const Road *>(immovable))
		pixelcolor = blend_color(format, pixelcolor, 255, 255, 255);
	if
		((flags & MiniMap::Flags and dynamic_cast<const Flag *>(immovable))
		 or
		 (flags & MiniMap::Bldns
		  and
		  dynamic_cast<const Widelands::Building *>(immovable)))
		pixelcolor = SDL_MapRGB
		(&const_cast<SDL_PixelFormat &>(format), 255, 255, 255);
	}

	return pixelcolor;

/*

*/
   return 0;
}

template<typename T>
static void draw_minimap_int
(Uint8 * const             pixels,
 const uint16_t              pitch,
 const SDL_PixelFormat   & format,
 const uint32_t                mapwidth,
 Widelands::Editor_Game_Base const &       egbase,
 Widelands::Player           const * const player,
 const Rect                rc,
 const Point               viewpoint,
 const uint32_t                flags)
{
	Widelands::Map const & map = egbase.map();
	if (not player or player->see_all()) for (uint32_t y = 0; y < rc.h; ++y) {
		Uint8 * pix = pixels + (rc.y + y) * pitch + rc.x * sizeof(T);
		Widelands::FCoords f(Widelands::Coords(viewpoint.x, viewpoint.y + y), 0);
		map.normalize_coords(&f);
		f.field = &map[f];
		Widelands::Map_Index i = Widelands::Map::get_index(f, mapwidth);
		for (uint32_t x = 0; x < rc.w; ++x, pix += sizeof(T)) {
			move_r(mapwidth, f, i);
			*reinterpret_cast<T *>(pix) = static_cast<T>
				(calc_minimap_color
				 (format, egbase, f, flags, f.field->get_owned_by(), true));
		}
	} else {
		Widelands::Player::Field const * const player_fields = player->fields();
		for (uint32_t y = 0; y < rc.h; ++y) {
		Uint8 * pix = pixels + (rc.y + y) * pitch + rc.x * sizeof(T);
		Widelands::FCoords f(Widelands::Coords(viewpoint.x, viewpoint.y + y), 0);
		map.normalize_coords(&f);
		f.field = &map[f];
		Widelands::Map_Index i = Widelands::Map::get_index(f, mapwidth);
		for (uint32_t x = 0; x < rc.w; ++x, pix += sizeof(T)) {
			move_r(mapwidth, f, i);
				const Widelands::Player::Field & player_field = player_fields[i];
				const Widelands::Vision vision = player_field.vision;
				*reinterpret_cast<T *>(pix) = static_cast<T>
					(vision ?
					 calc_minimap_color
					 (format, egbase, f, flags, player_field.owner, 1 < vision)
					 :
					 0);
			}
		}
	}
}

/*
===============
Draw a minimap into the given rectangle of the bitmap.
viewpt is the field at the top left of the rectangle.
===============
*/
void Surface::draw_minimap
(Widelands::Editor_Game_Base const &       egbase,
 Widelands::Player           const * const player,
 const Rect                rc,
 const Point               viewpt,
 const uint32_t                flags)
{
	//TODO: this const_cast is evil and should be exorcised.
	Uint8 * const pixels = const_cast<Uint8 *>(static_cast<const Uint8 *>(get_pixels()));
	const uint16_t pitch = get_pitch();
	Widelands::X_Coordinate const w = egbase.map().get_width();
	switch (format().BytesPerPixel) {
	case sizeof(Uint16):
		draw_minimap_int<Uint16>
			(pixels, pitch, format(), w, egbase, player, rc, viewpt, flags);
		break;
	case sizeof(Uint32):
		draw_minimap_int<Uint32>
			(pixels, pitch, format(), w, egbase, player, rc, viewpt, flags);
		break;
	default: assert (false);
	}
}

/*
==============================================================================

AnimationGfx -- contains graphics data for an animtion

==============================================================================
*/

/*
===============
AnimationGfx::AnimationGfx

Load the animation
===============
*/
static const uint32_t nextensions = 4;
static const char extensions[nextensions][5] = {".bmp", ".png", ".gif", ".jpg"};
AnimationGfx::AnimationGfx(const AnimationData* data)
{

   m_encodedata.hasplrclrs = data->encdata.hasplrclrs;
   m_encodedata.plrclr[0] = data->encdata.plrclr[0];
   m_encodedata.plrclr[1] = data->encdata.plrclr[1];
   m_encodedata.plrclr[2] = data->encdata.plrclr[2];
   m_encodedata.plrclr[3] = data->encdata.plrclr[3];

   m_hotspot = data->hotspot;
   m_plrframes = new std::vector<Surface*>[MAX_PLAYERS+1];

   std::vector<Surface*> frames;
	for (;;) {
      char fname[256];
      int32_t nr = frames.size();
      char *p;

      bool alldone=false;
      bool cycling=false;

		for (uint32_t i = 0; i < nextensions; ++i) {

         // create the file name by reverse-scanning for '?' and replacing
         nr=frames.size();
         snprintf(fname, sizeof(fname), "%s%s", data->picnametempl.c_str(), extensions[i]);
         p = fname + strlen(fname);
			while (p > fname) {
				if (*--p != '?')
               continue;

            cycling=true;

            *p = '0' + (nr % 10);
            nr = nr / 10;
			}

			if (nr) // cycled up to maximum possible frame number
            break;


         // is the frame actually there?
			if (not g_fs->FileExists(fname)) {
				if (i == nextensions - 1) {
					alldone = true;
					break;
				}
            continue;
			}

			try {
				SDL_Surface & bmp = *LoadImage(fname);

				// Get a new AnimFrame
				Surface & frame = *new Surface();
				frames.push_back(&frame);
				frame.set_sdl_surface(bmp);
			}
			catch (const std::exception & e) {
            log("WARNING: Couldn't load animation frame %s: %s\n", fname, e.what());
            continue;
			}


			if (!cycling)
				alldone = true;
			break;
		}

		if (alldone)
			break;
	}

   m_plrframes[0] = frames;

	if (!frames.size())
		throw wexception("Animation %s has no frames", data->picnametempl.c_str());

}


/*
===============
AnimationGfx::~AnimationGfx

Free all resources
===============
*/
AnimationGfx::~AnimationGfx()
{
	for (Widelands::Player_Number i = 0; i <= MAX_PLAYERS; ++i) {
      std::vector<Surface*>& frames = m_plrframes[i];
		for (uint32_t j = 0; j < frames.size(); ++j)
         delete frames[j];
	}
   delete[] m_plrframes;
}


/*
===============
AnimationGfx::encode

Encodes the given surface into a frame
===============
*/
void AnimationGfx::encode(uint8_t plr, const RGBColor* plrclrs)
{
   assert(m_encodedata.hasplrclrs);
   std::vector<Surface*>& frames = m_plrframes[plr];

	for (uint32_t i = 0; i < m_plrframes[0].size(); ++i) {
      // Copy the old surface
		Surface & origsurface = *m_plrframes[0][i];
		SDL_Surface & tempsurface = *SDL_ConvertSurface
			(origsurface.m_surface,
			 &const_cast<SDL_PixelFormat &>(origsurface.format()),
			 SDL_HWSURFACE | SDL_SRCALPHA);
		Surface & newsurface = *new Surface();
		newsurface.set_sdl_surface(tempsurface);
		const SDL_PixelFormat & format = newsurface.format();

		uint32_t plrclr1 = m_encodedata.plrclr[0].map(format);
		uint32_t plrclr2 = m_encodedata.plrclr[1].map(format);
		uint32_t plrclr3 = m_encodedata.plrclr[2].map(format);
		uint32_t plrclr4 = m_encodedata.plrclr[3].map(format);

		uint32_t new_plrclr1 = plrclrs[0].map(format);
		uint32_t new_plrclr2 = plrclrs[1].map(format);
		uint32_t new_plrclr3 = plrclrs[2].map(format);
		uint32_t new_plrclr4 = plrclrs[3].map(format);

      // Walk the surface, replace all playercolors
		for (uint32_t y = 0; y < newsurface.get_h(); ++y) {
			for (uint32_t x = 0; x < newsurface.get_w(); ++x) {
				const uint32_t clr = newsurface.get_pixel(x, y);
				if      (clr == plrclr1) newsurface.set_pixel(x, y, new_plrclr1);
				else if (clr == plrclr2) newsurface.set_pixel(x, y, new_plrclr2);
				else if (clr == plrclr3) newsurface.set_pixel(x, y, new_plrclr3);
				else if (clr == plrclr4) newsurface.set_pixel(x, y, new_plrclr4);
			}
		}

      // Add to the framse
		frames.push_back(&newsurface);
	}
}
