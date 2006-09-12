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
Rendering functions of the 16-bit software renderer.
*/

#include <SDL.h>
#include "editor_game_base.h"
#include "error.h"
#include "filesystem.h"
#include "layeredfilesystem.h"
#include "map.h"
#include "minimap.h"
#include "player.h"
#include "rgbcolor.h"
#include "graphic_impl.h"
#include "wexception.h"
#include "world.h"

/*
==============================================================================

Surface

==============================================================================
*/


/*
 * Create a Surface from a surface
 */
Surface::Surface( const Surface& surf ) {
   m_w = surf.m_w;
   m_h = surf.m_h;
   m_surface = SDL_DisplayFormat( surf.m_surface ); // HACK: assume this should be picture format; there is no SDL_CopySurface
}

/*
 * Updating the whole Surface
 */
void Surface::update( void ) {
   SDL_UpdateRect(m_surface, 0, 0, 0, 0);
}

/*
 * Save a bitmap
 */
void Surface::save_bmp( const char* fname ) {
   assert( m_surface );
   SDL_SaveBMP( m_surface, fname );
}

/*
 * disable alpha for this surface. Needed
 * if you want to copy directly to the screen
 * by direct pixel access. For example for road
 * textures
 */
void Surface::force_disable_alpha( void ) {
   SDL_Surface* newsur = SDL_DisplayFormat( m_surface );
   SDL_FreeSurface( m_surface );
   m_surface = newsur;
}

/*
===============
Surface::draw_rect

Draws the outline of a rectangle
===============
*/
void Surface::draw_rect(Rect rc, RGBColor clr)
{
   assert( m_surface );
	ulong color = clr.map( get_format() );

	rc.w += rc.x-1;
	rc.h += rc.y-1;

	for (int x=rc.x+1; x<rc.w; x++)
	{
		set_pixel( x, rc.y, color );
		set_pixel( x, rc.h, color );
	}
	for (int y=rc.y; y<=rc.h; y++)
	{
		set_pixel( rc.x, y, color );
		set_pixel( rc.w, y, color );
	}
}


/*
===============
Surface::fill_rect

Draws a filled rectangle
===============
*/
void Surface::fill_rect(Rect rc, RGBColor clr)
{
   assert( m_surface );
	ulong color = clr.map( get_format() );

   SDL_Rect r = { rc.x, rc.y, rc.w, rc.h };
   SDL_FillRect( m_surface, &r, color);
}

/*
===============
Surface::brighten_rect

Change the brightness of the given rectangle
This function is slow as hell.
===============
*/
void Surface::brighten_rect(Rect rc, int factor)
{
   rc.w += rc.x;
   rc.h += rc.y;

   for (int y = rc.y; y < rc.h; y++)
   {
      for (int x = rc.x; x < rc.w; x++) {
         uchar gr, gg, gb;
         short r, g, b;
         ulong clr = get_pixel(x,y);
         SDL_GetRGB( clr, m_surface->format, &gr, &gg, &gb );
         r = gr + factor;
         g = gg + factor;
         b = gb + factor;
         if (b & 0xFF00) b = (~b) >> 24;
         if (g & 0xFF00) g = (~g) >> 24;
         if (r & 0xFF00) r = (~r) >> 24;
         clr = SDL_MapRGB( m_surface->format, r, g, b );
         set_pixel( x, y, clr );
      }
   }
}


/*
===============
Surface::clear

Clear the entire bitmap to black
===============
*/
void Surface::clear() {
	SDL_FillRect( m_surface, 0, 0 );
}

/*
===============
Surface::blit

Blit this given source bitmap to this bitmap.
===============
*/
void Surface::blit(Point dst, Surface* src, Rect srcrc)
{
   SDL_Rect srcrect = { srcrc.x, srcrc.y, srcrc.w, srcrc.h };
   SDL_Rect dstrect = { dst.x, dst.y, 0, 0 };

   SDL_BlitSurface( src->m_surface, &srcrect, m_surface, &dstrect );
}

/*
 * Fast blit, simply copy the source to the destination
 */
void Surface::fast_blit( Surface* src ) {
   SDL_BlitSurface( src->m_surface, 0, m_surface, 0 );
}

/*
 * Blend to colors; only needed for calc_minimap_color below
 */
static inline ulong blend_color( SDL_PixelFormat* fmt, ulong clr1, uchar r2, uchar g2, uchar b2 ) {
  uchar r1, g1, b1;

  SDL_GetRGB( clr1, fmt, &r1, &g1, &b1);

  return SDL_MapRGB( fmt, (r1 + r2) >> 1, ( g1 + g2 ) >> 1, ( b1 + b2 ) >> 1 );
}

/*
===============
calc_minimap_color

Return the color to be used in the minimap for the given field.
===============
*/
static inline ulong calc_minimap_color
(SDL_PixelFormat * const fmt,
 Map & map,
 const Editor_Game_Base & egbase,
 const FCoords f,
 const uint flags)
{
	ulong pixelcolor = 0;

	if (flags & Minimap_Terrain)
	{
		pixelcolor =
			get_graphicimpl()->
			get_maptexture_data(f.field->get_terd()->get_texture())->
			get_minimap_color(f.field->get_brightness());
	}

	if (flags & Minimap_PlayerColor) { //  show owner
		if (f.field->get_owned_by() > 0) { //  If owned, get the player's color...
			const RGBColor * const playercolors =
				egbase.get_player(f.field->get_owned_by())->get_playercolor();

			//  ...and add the player's color to the old color.
			pixelcolor = blend_color
				(fmt,
				 pixelcolor,
				 playercolors[3].r(),  playercolors[3].g(), playercolors[3].b());
		}
	}

	if (flags & Minimap_Roads) { // show roads
		if (map.find_immovables(f, 0, 0, FindImmovableType(Map_Object::ROAD)))
			pixelcolor = blend_color(fmt, pixelcolor, 255, 255, 255 );
	}

	if (flags & Minimap_Flags) { // show flags
		if (map.find_immovables(f, 0, 0, FindImmovableType(Map_Object::FLAG)))
			pixelcolor = SDL_MapRGB( fmt, 255, 255, 255 );
	}

	if (flags & Minimap_Buildings) { // show buildings
		if (map.find_immovables(f, 0, 0, FindImmovableType(Map_Object::BUILDING)))
			pixelcolor = SDL_MapRGB( fmt, 255, 255, 255 );
	}

	return pixelcolor;

/*

*/
   return 0;
}

template<typename T>
void draw_minimap_int
(Uint8 * const pixels,
 const ushort pitch,
 SDL_PixelFormat * const fmt,
 Map & map,
 const uint mapwidth,
 const Editor_Game_Base & egbase,
 const std::vector<bool> * const visibility,
 const Rect rc,
 const Coords viewpt,
 const uint flags)
{
	for (int y = 0; y < rc.h; ++y) {
		Uint8 * pix = pixels + (rc.y + y) * pitch + rc.x * sizeof(T);
		FCoords f(viewpt.x, viewpt.y + y, 0);
		map.normalize_coords(&f);
		f.field = map.get_field(f);
		Map::Index i = Map::get_index(f, mapwidth);
		for (int x = 0; x < rc.w; ++x, pix += sizeof(T)) {
			move_r(mapwidth, f, i);
			*reinterpret_cast<T * const>(pix) = static_cast<const T>
				(visibility and not (*visibility)[i] ?
				 0 : calc_minimap_color(fmt, map, egbase, f, flags));
		}
	}
}

/*
===============
Surface::draw_minimap

Draw a minimap into the given rectangle of the bitmap.
viewpt is the field at the top left of the rectangle.
===============
*/
void Surface::draw_minimap
(const Editor_Game_Base & egbase,
 const std::vector<bool> * const visibility,
 const Rect rc,
 const Coords viewpt,
 const uint flags)
{
	Uint8 * const pixels = static_cast<Uint8 * const>(get_pixels());
	const ushort pitch = get_pitch();
	SDL_PixelFormat * const fmt = get_format();
	Map & map = egbase.get_map();
	const uint w = map.get_width();
	switch (fmt->BytesPerPixel) {
	case 2:
		draw_minimap_int<Uint16>
			(pixels, pitch, fmt, map, w, egbase, visibility, rc, viewpt, flags);
		break;
	case 4:
		draw_minimap_int<Uint32>
			(pixels, pitch, fmt, map, w, egbase, visibility, rc, viewpt, flags);
		break;
	default:
		assert (0);
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
AnimationGfx::AnimationGfx(const AnimationData* data)
{
   const int nextensions=4;
   const char extensions[nextensions][5] = {
      ".bmp",
      ".png",
      ".gif",
      ".jpg"
   };

   m_encodedata.hasplrclrs = data->encdata.hasplrclrs;
   m_encodedata.plrclr[0] = data->encdata.plrclr[0];
   m_encodedata.plrclr[1] = data->encdata.plrclr[1];
   m_encodedata.plrclr[2] = data->encdata.plrclr[2];
   m_encodedata.plrclr[3] = data->encdata.plrclr[3];

   m_hotspot = data->hotspot;
   m_plrframes = new std::vector<Surface*>[MAX_PLAYERS+1];

   std::vector<Surface*> frames;
   for(;;) {
      char fname[256];
      int nr = frames.size();
      char *p;

      bool done=false;
      bool alldone=false;
      bool cycling=false;

      for(int i=0; i<nextensions; i++) {
         if(done) continue;

         // create the file name by reverse-scanning for '?' and replacing
         nr=frames.size();
         snprintf(fname, sizeof(fname), "%s%s", data->picnametempl.c_str(),extensions[i]);
         p = fname + strlen(fname);
         while(p > fname) {
            if (*--p != '?')
               continue;

            cycling=true;

            *p = '0' + (nr % 10);
            nr = nr / 10;
         }

         if (nr) // cycled up to maximum possible frame number
            break;


         // is the frame actually there?
         if (!g_fs->FileExists(fname)) {
            if(i==(nextensions-1)) { alldone=true; break; }
            continue;
         }

         // Load the image
         SDL_Surface* bmp = 0;

         try
         {
            bmp = LoadImage(fname);
         }
         catch(std::exception& e)
         {
            log("WARNING: Couldn't load animation frame %s: %s\n", fname, e.what());
            continue;
         }

         // Get a new AnimFrame
         Surface* frame = new Surface();
         frames.push_back( frame );
         frame->set_sdl_surface( bmp );

         done=true;
         if(!cycling) alldone=true;
      }

      if(alldone==true) break;
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
   for( uint i = 0; i <= MAX_PLAYERS; i++ ) {
      std::vector<Surface*>& frames = m_plrframes[i];
      for( uint j = 0; j < frames.size(); j++) {
         delete frames[j];
      }
   }
   delete[] m_plrframes;
}


/*
===============
AnimationGfx::encode

Encodes the given surface into a frame
===============
*/
void AnimationGfx::encode( uchar plr, const RGBColor* plrclrs )
{
   assert( m_encodedata.hasplrclrs );
   std::vector<Surface*>& frames = m_plrframes[plr];

   for( uint i = 0; i < m_plrframes[0].size(); i++ ) {
      // Copy the old surface
      Surface* origsurface = m_plrframes[0][i];
      SDL_Surface* tempsurface =  SDL_ConvertSurface(origsurface->m_surface,
         origsurface->get_format(), SDL_HWSURFACE | SDL_SRCALPHA );
      Surface* newsurface = new Surface();
      newsurface->set_sdl_surface( tempsurface );

      ulong plrclr1 = m_encodedata.plrclr[0].map( newsurface->get_format());
      ulong plrclr2 = m_encodedata.plrclr[1].map( newsurface->get_format());
      ulong plrclr3 = m_encodedata.plrclr[2].map( newsurface->get_format());
      ulong plrclr4 = m_encodedata.plrclr[3].map( newsurface->get_format());

      ulong new_plrclr1 = plrclrs[0].map( newsurface->get_format());
      ulong new_plrclr2 = plrclrs[1].map( newsurface->get_format());
      ulong new_plrclr3 = plrclrs[2].map( newsurface->get_format());
      ulong new_plrclr4 = plrclrs[3].map( newsurface->get_format());

      // Walk the surface, replace all playercolors
      for( uint y = 0; y < newsurface->get_h(); y++) {
         for( uint x = 0; x < newsurface->get_w(); x++) {
            ulong clr = newsurface->get_pixel(x,y);
            if( clr == plrclr1 )
               newsurface->set_pixel( x, y, new_plrclr1 );
            else if( clr == plrclr2 )
               newsurface->set_pixel( x, y, new_plrclr2 );
            else if( clr == plrclr3 )
               newsurface->set_pixel( x, y, new_plrclr3 );
            else if( clr == plrclr4 )
               newsurface->set_pixel( x, y, new_plrclr4 );
         }
      }

      // Add to the framse
      frames.push_back( newsurface );
   }
}
