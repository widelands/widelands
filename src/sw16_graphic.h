/*
 * Copyright (C) 2002 by the Wide Lands Development Team
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

#ifndef included_sw16_graphic_h
#define included_sw16_graphic_h

#include <SDL.h>

class AnimationData;
class EncodeData;


namespace Renderer_Software16
{

/*
class AnimationGfx

The graphics belonging to an animation
*/
struct AnimFrame {
	int		width;
	int		height;
	Point		hotspot;
	ushort*	data;
};

class AnimationGfx {
private:
	int			m_nrframes;
	AnimFrame*	m_frames;

public:
	AnimationGfx(const AnimationData* data);
	~AnimationGfx();
	
	inline int get_nrframes() const { return m_nrframes; }
	inline const AnimFrame* get_frame(int i) const
	{ assert(i>=0 && i<m_nrframes); return &m_frames[i]; }

private:
	void encode(AnimFrame* frame, SDL_Surface* bmp, const EncodeData* encdata);
};


/*
struct Bitmap

Represents a simple bitmap without managing its memory.
The rendering functions do NOT perform any clipping; this is up to the caller.
*/
struct Bitmap {
	ushort*	pixels;
	int		w, h;
	int		pitch;
	bool		hasclrkey;
	ushort	clrkey;

	void clear();
	void draw_rect(Rect rc, RGBColor clr);
	void fill_rect(Rect rc, RGBColor clr);
	void brighten_rect(Rect rc, int factor);
	
	void blit(Point dst, Bitmap* src, Rect srcrc);

	void draw_minimap(Point dst, const MapRenderInfo* mri, Rect rc, uint fx, uint fy);
	void draw_animframe(Point dst, const AnimFrame* frame, Rect rc, const RGBColor* plrclrs);
	
	// sw16_terrain.cc
	void draw_field(Field * const f, Field * const rf, Field * const fl, Field * const rfl,
	                const int posx, const int rposx, const int posy, 
	                const int blposx, const int rblposx, const int blposy, 
	                uchar roads, bool render_r, bool render_b);
};


/** class Colormap
*
* Colormap contains a palette and lookup table for use with ground textures.
*/
class Colormap {
//    friend class Texture;
    
private:
	SDL_Color palette[256];
	
	unsigned short *colormap;	// maps 8 bit color and brightness value to the shaded 16 bit color
		// NOTE: brightness is currently 8 bits. Restricting brightness
		// to 64 or less shades would greatly reduce the size of this
		// table, and thus improve memory cache impact inside the renderer.

public:
	Colormap (const SDL_Color *);
	~Colormap ();
	
	SDL_Color* get_palette() { return palette; }
	
	unsigned short *get_colormap () const { return colormap; }
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
   
public:
	Texture (const char* fnametempl, uint frametime);
	~Texture ();
	
	unsigned char *get_pixels () const { return m_pixels; }
	unsigned char* get_curpixels() const { return m_curframe; }
	unsigned short *get_colormap () const { return m_colormap->get_colormap(); }
	
	unsigned short get_minimap_color(char shade) const;
	
	void animate(uint time);
};


/*
struct GameIcons

Contains the indices of a number of default icons that are accessed by 
rendermap()
*/
struct GameIcons {
	uint	pics_roadb[3];	// green, yellow, red
	uint	pics_build[5];	// flag, small, medium, big, mine (same order as Overlay_Build_*)
	uint	pic_fieldsel;
};

/*
class RenderTargetImpl

The 16-bit software renderer implementation of the RenderTarget interface
*/
class RenderTargetImpl : public RenderTarget {
	Bitmap*	m_bitmap;		// the target surface
	Rect		m_rect;			// the current clip rectangle
	Point		m_offset;		// drawing offset
	
public:
	RenderTargetImpl(Bitmap* bmp);
	virtual ~RenderTargetImpl();
	
	void reset();
	
	virtual void get_window(Rect* rc, Point* ofs) const;
	virtual void set_window(const Rect& rc, const Point& ofs);
	virtual bool enter_window(const Rect& rc, Rect* previous, Point* prevofs);

	virtual int get_w() const;
	virtual int get_h() const;
	
	virtual void draw_rect(int x, int y, int w, int h, RGBColor clr);
   virtual void fill_rect(int x, int y, int w, int h, RGBColor clr);
   virtual void brighten_rect(int x, int y, int w, int h, int factor);
	virtual void clear();

	void doblit(Point dst, Bitmap* src, Rect srcrc);
	
	virtual void blit(int dstx, int dsty, uint picture);
	virtual void blitrect(int dstx, int dsty, uint picture,
	                      int srcx, int srcy, int w, int h);
	virtual void tile(int x, int y, int w, int h, uint picture, int ofsx, int ofsy);

	virtual void rendermap(const MapRenderInfo* mgi, Point viewofs);
	virtual void renderminimap(Point pt, const MapRenderInfo* mri, uint fx, uint fy);

	virtual void drawanim(int dstx, int dsty, uint animation, uint time, const RGBColor* plrclrs = 0);
};


/*
class GraphicImpl

The 16-bit software renderer implementation of the Graphic interface.
*/
#define MAX_RECTS 20

class GraphicImpl : public Graphic {
public:
	GraphicImpl(int w, int h, bool fullscreen);
	virtual ~GraphicImpl();

	// General management
	virtual int get_xres();
	virtual int get_yres();
	virtual RenderTarget* get_render_target();
	virtual void update_fullscreen();
	virtual void update_rectangle(int x, int y, int w, int h);
	virtual bool need_update();
	virtual void refresh();

	virtual void flush(int mod);
	
	// Pictures
	virtual uint get_picture(int mod, const char* fname);
	virtual uint get_picture(int mod, const char* fname, RGBColor clrkey);
	virtual void get_picture_size(uint pic, int* pw, int* ph);
	virtual uint create_surface(int w, int h);
	virtual uint create_surface(int w, int h, RGBColor clrkey);
	virtual void free_surface(uint pic);
	virtual RenderTarget* get_surface_renderer(uint pic);
	
	Bitmap* get_picture_bitmap(uint id);
	
	// Map textures
	virtual uint get_maptexture(const char* fnametempl, uint frametime);
	virtual void animate_maptextures(uint time);
	Texture* get_maptexture_data(uint id);
	
	// Animations
	virtual void load_animations();
	AnimationGfx* get_animation(uint anim);
	
	// Misc functions
	virtual void screenshot(const char* fname);

	void allocate_gameicons();
	inline const GameIcons* get_gameicons() const { return m_gameicons; }

	virtual uint get_picture(int mod, int w, int h, const ushort* data, RGBColor clrkey);

private:
	uint find_free_picture();

	struct Picture {
		int		mod; // 0 if unused, -1 for surfaces, PicMod_* bitmask for pictures
		Bitmap	bitmap;
		
		union {
			char*					fname;
			RenderTargetImpl*	rendertarget;
		} u;
		
		Picture() { mod = 0; u.fname = 0; bitmap.pixels = 0; }
	};
	typedef std::map<const char*, uint> picmap_t;
	
	SDL_Surface*			m_sdlsurface;
	Bitmap					m_screen;
	RenderTargetImpl*		m_rendertarget;
   SDL_Rect					m_update_rects[MAX_RECTS];
	int						m_nr_update_rects;
	bool						m_update_fullscreen;
	
	std::vector<Picture>	m_pictures;
	picmap_t					m_picturemap; // hash of filename/picture ID pairs
	
	std::vector<Texture*>		m_maptextures;
	std::vector<AnimationGfx*>	m_animations;

	GameIcons*		m_gameicons;
};

#define get_graphicimpl() (static_cast<GraphicImpl*>(g_gr))


};

#endif // included_sw16_graphic_h
