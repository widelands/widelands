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

#ifndef included_graphic_impl_h
#define included_graphic_impl_h

#include <SDL.h>
#include <map>
#include <string>
#include <vector>
#include <png.h>
#include "animation.h"
#include "constants.h"
#include "geometry.h"
#include "graphic.h"
#include "player.h"
#include "rendertarget.h"
#include "rgbcolor.h"

SDL_Surface* LoadImage(const char * const filename);

/*
class AnimationGfx

The graphics belonging to an animation
*/
class AnimationGfx {
private:
	std::vector<Surface*>* m_plrframes;
   EncodeData  m_encodedata;
   Point       m_hotspot;

public:
	AnimationGfx(const AnimationData* data);
	~AnimationGfx();

   inline const Point& get_hotspot( void ) { return m_hotspot; }
	inline int get_nrframes() const { assert((*m_plrframes)[0]); return m_plrframes[0].size(); }
	inline Surface* get_frame(int i, uchar plyr, const Player* player)
	{
      assert(i>=0 && i<get_nrframes() && plyr <= MAX_PLAYERS);
      if( !m_encodedata.hasplrclrs )
         return m_plrframes[0][i];

      assert( player );

      // Encode for this player
      if( !m_plrframes[plyr].size() )
         encode( plyr, player->get_playercolor() );
      return m_plrframes[plyr][i];
   }

private:
	void encode( uchar plyr, const RGBColor* );
};


/*
class Surface
const
This was formerly called struct Bitmap. But now it manages an SDL Surface as it's
core.

Represents a simple bitmap without managing its memory.
The rendering functions do NOT perform any clipping; this is up to the caller.
*/
class Surface {
   friend class AnimationGfx;
   friend class Font_Handler; // Needs m_surface for SDL_Blitting

   SDL_Surface* m_surface;
   int m_offsx;
   int m_offsy;
   uint m_w, m_h;

   public:
      Surface( void ) { m_surface = 0; m_offsy = m_offsx = 0; }
      Surface( const Surface& );
      ~Surface( void ) { if( m_surface ) SDL_FreeSurface( m_surface ); m_surface = 0; }

      // Set surface, only call once
      void set_sdl_surface( SDL_Surface* surface ) { m_surface = surface; m_w = m_surface->w; m_h = m_surface->h; }
	  SDL_Surface* get_sdl_surface() { return m_surface; }

      // Get width and height
      inline uint get_w( void ) { return m_w; }
      inline uint get_h( void ) { return m_h; }
      void update( void );

      // Save a bitmap of this to a file
      void save_bmp( const char* fname );

      // For the bravest: Direct Pixel access. Use carefully
      void force_disable_alpha( void ); // Needed if you want to blit directly to the screen by memcpy
      inline SDL_PixelFormat* get_format() { assert(m_surface); return m_surface->format; }
      inline ushort get_pitch( void ) { assert(m_surface); return m_surface->pitch; }
	void * get_pixels() {
		assert(m_surface);
		return
			static_cast<uchar * const>(m_surface->pixels) +
			m_offsy*m_surface->pitch + m_offsx*m_surface->format->BytesPerPixel;
	}

      // Lock
      inline void lock( void ) { if( SDL_MUSTLOCK( m_surface )) SDL_LockSurface( m_surface ); }
      inline void unlock( void ) { if( SDL_MUSTLOCK( m_surface )) SDL_UnlockSurface( m_surface ); }

      // For the slowest: Indirect pixel access
      inline ulong get_pixel( uint x, uint y ) {
         x+= m_offsx;
         y+= m_offsy;
         assert( x < get_w() && y < get_h() );
         assert( m_surface );
         // Locking not needed: reading only
			const uchar * pix =
				static_cast<const uchar * const>(m_surface->pixels) +
				y * m_surface->pitch + x * m_surface->format->BytesPerPixel;
         switch( m_surface->format->BytesPerPixel ) {
            case 1: return (*pix);
            case 2: return *reinterpret_cast<const ushort * const>(pix);
            case 3:
            case 4: return *reinterpret_cast<const ulong * const>(pix);
         }
         assert(0);
         return 0; // Should never be here
      }
      inline void set_pixel( uint x, uint y, ulong clr) {
         x+= m_offsx;
         y+= m_offsy;
         assert( x < get_w() && y < get_h() );
         assert( m_surface );
         if( SDL_MUSTLOCK( m_surface ))
            SDL_LockSurface( m_surface );
			uchar * pix =
				static_cast<uchar * const>(m_surface->pixels) +
				y * m_surface->pitch + x * m_surface->format->BytesPerPixel;
         switch( m_surface->format->BytesPerPixel ) {
            case 1: (*pix) = static_cast<const uchar>(clr); break;
            case 2: *reinterpret_cast<ushort * const>(pix) = static_cast<const ushort>(clr); break;
            case 4: *reinterpret_cast<ulong * const>(pix) = clr; break;
         }
         if( SDL_MUSTLOCK( m_surface ))
            SDL_UnlockSurface( m_surface );
      }

      void clear();
      void draw_rect(Rect rc, RGBColor clr);
      void fill_rect(Rect rc, RGBColor clr);
      void brighten_rect(Rect rc, int factor);

      void blit(Point dst, Surface* src, Rect srcrc);
      void fast_blit( Surface* src );

	void draw_minimap
		(const Editor_Game_Base & egbase,
		 const std::vector<bool> * const visibility,
		 const Rect rc,
		 const Coords viewpt,
		 const uint flags);


      // sw16_terrain.cc
      void draw_field(Rect&, Field * const f, Field * const rf, Field * const fl, Field * const rfl,
            Field * const lf, Field * const ft,
            const int posx, const int rposx, const int posy,
            const int blposx, const int rblposx, const int blposy,
            uchar roads, uchar darken, bool);


   private:
      inline void set_subwin( Rect r ) { m_offsx = r.x; m_offsy = r.y; m_w =r.w; m_h = r.h; }
      inline void unset_subwin( void ) { m_offsx = 0; m_offsy = 0; m_w = m_surface->w; m_h = m_surface->h; }
};


/** class Colormap
*
* Colormap contains a palette and lookup table for use with ground textures.
*/
class Colormap {
//    friend class Texture;

private:
	SDL_Color palette[256];
	void* colormap;	// maps 8 bit color and brightness value to the shaded color
		// NOTE: brightness is currently 8 bits. Restricting brightness
		// to 64 or less shades would greatly reduce the size of this
		// table, and thus improve memory cache impact inside the renderer.

public:
	Colormap (const SDL_Color*, SDL_PixelFormat* fmt);
	~Colormap ();

	SDL_Color* get_palette() { return palette; }

   void* get_colormap () const { return colormap; }
};


/** class Texture
*
* Texture represents are terrain texture, which is strictly
* TEXTURE_W by TEXTURE_H pixels in size. It uses 8 bit color, and a pointer
* to the corresponding palette and color lookup table is provided.
*
* Currently, this is initialized from a 16 bit bitmap. This should be
* changed to load 8 bit bitmaps directly.
*/
class Texture {
private:
	Colormap*			m_colormap;
	uint					m_nrframes;
	unsigned char*		m_pixels;
	uint					m_frametime;
	unsigned char*		m_curframe;
   std::string			m_texture_picture;
   bool              is_32bit;
   bool              m_was_animated;

public:
	Texture (const char* fnametempl, uint frametime, SDL_PixelFormat*);
	~Texture ();

   inline const char* get_texture_picture(void) { return m_texture_picture.c_str(); }

	unsigned char *get_pixels () const { return m_pixels; }
	unsigned char* get_curpixels() const { return m_curframe; }
	void* get_colormap () const { return m_colormap->get_colormap(); }

	ulong get_minimap_color(char shade);

	void animate(uint time);
   inline void reset_was_animated( void ) { m_was_animated = false; }
   inline bool was_animated( void ) { return m_was_animated; }
};

/*
 * This contains all the road textures needed to render roads
 */
struct Road_Textures {
   uint pic_road_normal;
   uint pic_road_busy;
};

/*
class RenderTargetImpl

The 16-bit software renderer implementation of the RenderTarget interface
*/
class RenderTargetImpl : public RenderTarget {
	Surface* m_ground_surface; // only needed, when this is a mapview
   Surface*	m_surface;		// the target surface
	Rect		m_rect;			// the current clip rectangle
	Point		m_offset;		// drawing offset

public:
	RenderTargetImpl(Surface* bmp);
	virtual ~RenderTargetImpl();

	void reset();

	virtual void get_window(Rect* rc, Point* ofs) const;
	virtual void set_window(const Rect& rc, const Point& ofs);
	virtual bool enter_window(const Rect& rc, Rect* previous, Point* prevofs);

	virtual int get_w() const;
	virtual int get_h() const;

   virtual void draw_line(int x1, int y1, int x2, int y2, RGBColor color);
	virtual void draw_rect(int x, int y, int w, int h, RGBColor clr);
	virtual void fill_rect(int x, int y, int w, int h, RGBColor clr);
	virtual void brighten_rect(int x, int y, int w, int h, int factor);
	virtual void clear();

	void doblit(Point dst, Surface* src, Rect srcrc);

	virtual void blit(int dstx, int dsty, uint picture);
	virtual void blitrect(int dstx, int dsty, uint picture,
	                      int srcx, int srcy, int w, int h);
	virtual void tile(int x, int y, int w, int h, uint picture, int ofsx, int ofsy);

	virtual void rendermap
		(const Editor_Game_Base &,
		 const std::vector<bool> * const visibility,
		 Point viewofs,
		 const bool draw_all);
	virtual void renderminimap
		(const Editor_Game_Base &,
		 const std::vector<bool> * const visibility,
		 Coords viewpoint,
		 const uint flags);

	virtual void drawanim(int dstx, int dsty, uint animation, uint time, const Player* plrclrs = 0);
	virtual void drawanimrect(int dstx, int dsty, uint animation, uint time,
									  const Player* plrclrs, int srcx, int srcy, int w, int h);
};


/*
class GraphicImpl

The 16-bit software renderer implementation of the Graphic interface.
*/
#define MAX_RECTS 20

class GraphicImpl : public Graphic {
public:
	GraphicImpl(int w, int h, int bpp, bool fullscreen);
	virtual ~GraphicImpl();

	// General management
	virtual int get_xres();
	virtual int get_yres();
	virtual RenderTarget* get_render_target();
	virtual void toggle_fullscreen();
	virtual void update_fullscreen();
	virtual void update_rectangle(int x, int y, int w, int h);
	virtual bool need_update();
	virtual void refresh();

	virtual void flush(int mod);

	// Pictures
	virtual uint get_picture(int mod, const char* fname);
   virtual void flush_picture(uint pic);
	virtual void get_picture_size(const uint pic, uint & w, uint & h);
	virtual uint create_surface(int w, int h);
	virtual void free_surface(uint pic);
	virtual RenderTarget* get_surface_renderer(uint pic);
   virtual void save_png(uint, FileWrite*);

	Surface* get_picture_surface(uint id);

	// Map textures
	virtual uint get_maptexture(const char* fnametempl, uint frametime);
	virtual void animate_maptextures(uint time);
   virtual void reset_texture_animation_reminder( void );
	Texture* get_maptexture_data(uint id);

   // Road textures
   Surface* get_road_texture( int );

	// Animations
	virtual void load_animations();
	AnimationGfx* get_animation(uint anim);
	virtual int get_animation_nr_frames(uint anim);
	virtual void get_animation_size(uint anim, uint time, int* w, int* h);

	// Misc functions
	virtual void screenshot(const char* fname);

	virtual uint get_picture(int mod, Surface* surf, const char* = 0 );
	virtual const char* get_maptexture_picture(uint id);

private:
   // Static function for png writing
   static void m_png_write_function( png_structp, png_bytep, png_size_t );

	struct Picture {
		int		mod; // 0 if unused, -1 for surfaces, PicMod_* bitmask for pictures
		Surface*  surface;

		union {
			char*					fname;
			RenderTargetImpl*	rendertarget;
		} u;

		Picture() { surface = 0; mod = 0; u.fname = 0; }
	};

	std::vector<Picture>::size_type find_free_picture();

	typedef std::map<std::string, std::vector<Picture>::size_type> picmap_t;

	Surface              m_screen;
	RenderTargetImpl*		m_rendertarget;
   SDL_Rect					m_update_rects[MAX_RECTS];
	int						m_nr_update_rects;
	bool						m_update_fullscreen;

	std::vector<Picture>	m_pictures;
	picmap_t					m_picturemap; // hash of filename/picture ID pairs

   Road_Textures*        m_roadtextures;
	std::vector<Texture*>		m_maptextures;
	std::vector<AnimationGfx*>	m_animations;
};

#define get_graphicimpl() (static_cast<GraphicImpl*>(g_gr))



#endif // included_graphic_impl_h
