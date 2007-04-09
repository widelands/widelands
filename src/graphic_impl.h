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

#ifndef included_graphic_impl_h
#define included_graphic_impl_h

#include <SDL.h>
#include <map>
#include <string>
#include <vector>
#include <png.h>
#include "animation.h"
#include "animation_gfx.h"
#include "constants.h"
#include "geometry.h"
#include "graphic.h"
#include "player.h"
#include "rendertargetimpl.h"
#include "rgbcolor.h"
#include "surface.h"
#include "texture.h"

#define MAX_RECTS 20

#define get_graphicimpl() (static_cast<GraphicImpl*>(g_gr))

SDL_Surface* LoadImage(const char * const filename);

/**
 * The 16-bit software renderer implementation of the Graphic interface.
*/
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
	virtual void get_picture_size(const uint pic, uint & w, uint & h);
	virtual uint create_surface(int w, int h);
	virtual void free_surface(uint pic);
	virtual RenderTarget* get_surface_renderer(uint pic);
	virtual void save_png(uint, FileWrite*);

	Surface* get_picture_surface(uint id);

	// Map textures
	virtual uint get_maptexture(const char & fnametempl, const uint frametime);
	virtual void animate_maptextures(uint time);
	virtual void reset_texture_animation_reminder( void );
	Texture* get_maptexture_data(uint id);

	// Road textures
	Surface* get_road_texture( int );

	// Animations
	virtual void load_animations();
	AnimationGfx* get_animation(const uint anim) const;
	virtual AnimationGfx::Index nr_frames(const uint anim) const;
	virtual void get_animation_size
		(const uint anim, const uint time, uint & w, uint & h);

	// Misc functions
	virtual void screenshot(const char & fname) const;

	virtual uint get_picture(const int mod, Surface &, const char * const = 0);
	virtual const char* get_maptexture_picture(uint id);

private:
	// Static function for png writing
	static void m_png_write_function( png_structp, png_bytep, png_size_t );

	struct Picture {
		int mod; //  0 if unused, -1 for surfaces, PicMod_* bitmask for pictures
		Surface*  surface;

		union {
			char             * fname;
			RenderTargetImpl * rendertarget;
		} u;

		Picture() { surface = 0; mod = 0; u.fname = 0; }
	};

	std::vector<Picture>::size_type find_free_picture();

	typedef std::map<std::string, std::vector<Picture>::size_type> picmap_t;

	Surface                     m_screen;
	RenderTargetImpl          * m_rendertarget;
	SDL_Rect                    m_update_rects[MAX_RECTS];
	int                         m_nr_update_rects;
	bool                        m_update_fullscreen;

	std::vector<Picture>        m_pictures;
	picmap_t m_picturemap; //  hash of filename/picture ID pairs

	Road_Textures*       m_roadtextures;
	std::vector<Texture      *> m_maptextures;
	std::vector<AnimationGfx *> m_animations;
};

#endif // included_graphic_impl_h
