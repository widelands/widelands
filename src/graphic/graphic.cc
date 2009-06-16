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

#include "graphic.h"

#include "build_info.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"
#include "font_handler.h"
#include "i18n.h"
#include "rendertarget.h"
#include "texture.h"
#include "wexception.h"
#include "widelands_fileread.h"

#include "ui_basic/progresswindow.h"

#include "log.h"

#include <SDL_image.h>
#include <SDL_rotozoom.h>
#if HAS_OPENGL
#include <SDL_opengl.h>
#endif
#include <cstring>
#include <iostream>

Graphic *g_gr;
#if HAS_OPENGL
bool g_opengl;
#endif

/**
 * Helper function wraps around SDL_image. Returns the given image file as a
 * surface.
 * Cannot return 0, throws an exception on error.
 * \todo Get rid of this function
*/
SDL_Surface * LoadImage(char const * const filename)
{
	FileRead fr;
	SDL_Surface * surf;

	//fastOpen tries to use mmap
	fr.fastOpen(*g_fs, filename);

	surf = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);

	if (!surf)
		throw wexception("%s", IMG_GetError());

	return surf;
}

static uint32_t luminance_table_r[0x100];
static uint32_t luminance_table_g[0x100];
static uint32_t luminance_table_b[0x100];

/**
 * Initialize the SDL video mode.
*/
#if HAS_OPENGL
Graphic::Graphic
	(int32_t const w, int32_t const h,
	 int32_t const bpp,
	 bool    const fullscreen,
	 bool    const hw_improvements,
	 bool    const double_buffer,
	 bool    const opengl)
#else
Graphic::Graphic
	(int32_t const w, int32_t const h,
	 int32_t const bpp,
	 bool    const fullscreen,
	 bool    const hw_improvements,
	 bool    const double_buffer)
#endif
	:
	m_rendertarget     (0),
	m_nr_update_rects  (0),
	m_update_fullscreen(false),
	m_roadtextures     (0)
{
	m_picturemap.resize(MaxModule);

	for
		(uint32_t i = 0, r = 0, g = 0, b = 0;
		 i < 0x100;
		 ++i, r += 5016388U, g += 9848226U, b += 1912603U)
	{
		luminance_table_r[i] = r;
		luminance_table_g[i] = g;
		luminance_table_b[i] = b;
	}

	// Set video mode using SDL
	int32_t flags = SDL_SWSURFACE;

	if (hw_improvements) {
		log("Graphics: Trying HW_SURFACE\n");
		flags = SDL_HWSURFACE; //  |SDL_HWACCEL|SDL_OPENGL;
	}
#if HAS_OPENGL
	if (hw_improvements && opengl) {
		log("Graphics: Trying opengl\n");
		flags = SDL_HWACCEL|SDL_OPENGL;//SDL_OPENGLBLIT;
		g_opengl = true;
	}
#endif
	if (hw_improvements && double_buffer) {
		flags |= SDL_DOUBLEBUF;
		log("Graphics: Trying DOUBLE BUFFERING\n");
	}
	if (fullscreen) {
		flags |= SDL_FULLSCREEN;
		log("Graphics: Trying FULLSCREEN\n");
	}

	SDL_Surface * sdlsurface = 0;

	sdlsurface = SDL_SetVideoMode(w, h, bpp, flags);

	if (!sdlsurface)
		throw wexception("could not set video mode: %s", SDL_GetError());

	if (0 != (sdlsurface->flags & SDL_HWSURFACE))
		log("Graphics: HW SURFACE ENABLED\n");
	if (0 != (sdlsurface->flags & SDL_DOUBLEBUF))
		log("Graphics: DOUBLE BUFFERING ENABLED\n");
	if (0 != (sdlsurface->flags & SDL_SWSURFACE))
		log("Graphics: SW SURFACE ENABLED\n");
	if (0 != (sdlsurface->flags & SDL_FULLSCREEN))
		log("Graphics: FULLSCREEN ENABLED\n");
#if HAS_OPENGL
	if (0 != (sdlsurface->flags & SDL_OPENGL))
	  log ("Graphics: OPENGL ENABLED\n");
#endif

	/* Information about the current video settings. */
	SDL_VideoInfo const * info = 0;

	info = SDL_GetVideoInfo();
	char videodrvused[16];
	SDL_VideoDriverName(videodrvused, 16);
	log
		("**** GRAPHICS REPORT ****\n"
		 " VIDEO DRIVER %s\n"
		 " hw surface possible %d\n"
		 " window manager available %d\n"
		 " blitz_hw %d\n"
		 " blitz_hw_CC %d\n"
		 " blitz_hw_A %d\n"
		 " blitz_sw %d\n"
		 " blitz_sw_CC %d\n"
		 " blitz_sw_A %d\n"
		 " blitz_fill %d\n"
		 " video_mem %d\n"
		 " vfmt %p\n"
		 " size %d %d\n"
		 "**** END GRAPHICS REPORT ****\n",
		 videodrvused,
		 info->hw_available,
		 info->wm_available,
		 info->blit_hw,
		 info->blit_hw_CC,
		 info->blit_hw_A,
		 info->blit_sw,
		 info->blit_sw_CC,
		 info->blit_sw_A,
		 info->blit_fill,
		 info->video_mem,
		 info->vfmt,
		 info->current_w, info->current_h);


	printf("\nhw avail:%d", info->hw_available);
	log("Graphics: flags: %x\n", sdlsurface->flags);

	assert
		(sdlsurface->format->BytesPerPixel == 2 ||
		 sdlsurface->format->BytesPerPixel == 4);

	SDL_WM_SetCaption
		(("Widelands " + build_id() + '(' + build_type() + ')').c_str(),
		 "Widelands");

#if HAS_OPENGL
	if (g_opengl) {
	  glMatrixMode(GL_PROJECTION);
	  glPushMatrix();
	  glLoadIdentity();
	  glOrtho(0, w, 0, h, -1, 1);

	  glMatrixMode(GL_MODELVIEW);
	  glPushMatrix();
	  glLoadIdentity();

	  glDisable(GL_DEPTH_TEST);
	  // glViewport(0, 0, w, h);
	  //gluOrtho2D(0, w, h, 0);
	  //glEnable(GL_TEXTURE_2D);
	  //glEnable(GL_BLEND);
	  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}
#endif

	m_screen.set_sdl_surface(*sdlsurface);

	//  FIXME do we not need to clean up the old render target?
	delete m_rendertarget;
	m_rendertarget = new RenderTarget(&m_screen);
}

/**
 * Free the surface
*/
Graphic::~Graphic()
{
	for (size_t i = 1; i < m_picturemap.size(); ++i)
		flush(static_cast<PicMod>(i));

	delete m_rendertarget;
	delete m_roadtextures;
}

/**
 * Return the screen x resolution
*/
int32_t Graphic::get_xres() const
{
	return m_screen.get_w();
}

/**
 * Return the screen x resolution
*/
int32_t Graphic::get_yres() const
{
	return m_screen.get_h();
}

/**
 * Return a pointer to the RenderTarget representing the screen
*/
RenderTarget * Graphic::get_render_target()
{
	m_rendertarget->reset();

	return m_rendertarget;
}

/**
 * Switch from fullscreen to windowed mode or vice-versa
*/
void Graphic::toggle_fullscreen()
{
	SDL_Surface *surface = m_screen.get_sdl_surface();
	SDL_WM_ToggleFullScreen(surface);
}

/**
 * Mark the entire screen for refreshing
*/
void Graphic::update_fullscreen()
{
	m_update_fullscreen = true;
}

/**
 * Mark a rectangle for refreshing
*/
void Graphic::update_rectangle(int32_t x, int32_t y, int32_t w, int32_t h)
{
	if (m_nr_update_rects >= MAX_RECTS) {
		m_update_fullscreen = true;
		return;
	}

	m_update_rects[m_nr_update_rects].x = x;
	m_update_rects[m_nr_update_rects].y = y;
	m_update_rects[m_nr_update_rects].w = w;
	m_update_rects[m_nr_update_rects].h = h;
	++m_nr_update_rects;
}

/**
 * Returns true if parts of the screen have been marked for refreshing.
*/
bool Graphic::need_update()
{
	return m_nr_update_rects || m_update_fullscreen;
}

/**
 * Bring the screen uptodate.
 *
 * \param force update whole screen
*/
void Graphic::refresh(bool force)
{
	if (force or m_update_fullscreen)
		m_screen.update();
	else
		SDL_UpdateRects
			(m_screen.get_sdl_surface(), m_nr_update_rects, m_update_rects);

	m_update_fullscreen = false;
	m_nr_update_rects = 0;
}

/**
 * Remove all resources (currently pictures) from the given modules.
 * \note flush(0) does nothing
*/
void Graphic::flush(PicMod const module) {
	// Flush pictures

	//pmit b, e = m_picturemap.end();
	for (size_t i = 0; i < m_picturemap.size(); ++i) {
		if (static_cast<PicMod>(i) == module) {
			m_picturemap[i].clear();
		}
	}

	if (!module || module & PicMod_Game) {
		container_iterate_const(std::vector<Texture *>, m_maptextures, i)
			delete *i.current;
		m_maptextures.clear();

		container_iterate_const(std::vector<AnimationGfx *>, m_animations, i)
			delete *i.current;
		m_animations.clear();

		delete m_roadtextures;
		m_roadtextures = 0;
	}

	if (not module or module & PicMod_UI) // Flush the cached Fontdatas
		g_fh->flush_cache();
}

/**
 * Retrieves the picture ID of the picture with the given filename.
 * If the picture has already been loaded, the old ID is reused.
 * The picture is placed into the module(s) given by mod.
 *
 * \return 0 (a null-picture) if the picture cannot be loaded.
*/
PictureID & Graphic::get_picture(PicMod const module, const std::string & fname)
{
	//  Check if the picture is already loaded.
	pmit it = m_picturemap[module].find(fname);

	if (it != m_picturemap[module].end()) {
	} else {
		SDL_Surface * bmp;

		try {
			bmp = LoadImage(fname.c_str());
			//log("Graphic::get_picture(): loading picture '%s'\n", fname);
		} catch (std::exception const & e) {
			log("WARNING: Could not open %s: %s\n", fname.c_str(), e.what());
			return get_no_picture();
		}
		// Convert the surface accordingly

		SDL_Surface & use_surface = *SDL_DisplayFormatAlpha(bmp);

		SDL_FreeSurface(bmp);

		// Fill in a free slot in the pictures array
		Picture & pic = * new Picture();
		PictureID id = PictureID(&pic);
		m_picturemap[module].insert(std::make_pair(fname, id));


		assert(pic.fname == 0);
		pic.fname   = strdup(fname.c_str());

		//  FIXME no proper check for NULL return value!
		assert(pic.fname != 0);

		pic.surface   = new Surface();

		pic.surface->set_sdl_surface(use_surface);
		it = m_picturemap[module].find(fname);
	}

	it->second->module = module;

	return it->second;
}

PictureID Graphic::get_picture
(PicMod const module, Surface & surf, const std::string & fname)
{
	Picture & pic = * new Picture();
	PictureID id = PictureID(&pic);
	m_picturemap[module].insert(std::make_pair(fname, id));

	pic.module    = module;
	pic.surface   = &surf;

	if (fname.size() != 0) {
		assert(pic.fname == 0);
		pic.fname = strdup(fname.c_str());
	} else
		pic.fname = 0;

	//return m_picturemap.find(fname);
	return id;
}

PictureID & Graphic::get_no_picture() const {
	static PictureID invalid = PictureID(new Picture());
	return invalid;
}

/**
 * Produces a resized version of the specified picture
 *
 * Might return same id if dimensions are the same
 */
PictureID Graphic::get_resized_picture
	(PictureID index,
	 uint32_t const w, uint32_t const h,
	 ResizeMode const mode)
{
	//if (index >= m_pictures.size() or !m_pictures[index].module)
	//throw wexception
	//("get_resized_picture(%i): picture does not exist", index);

	Surface * const orig = index->surface;
	if (orig->get_w() == w and orig->get_h() == h)
	  return index;

	uint32_t width = w;
	uint32_t height = h;

	if (mode != ResizeMode_Loose) {
		const double ratio_x = double(w) / orig->get_w();
		const double ratio_y = double(h) / orig->get_h();

		//  if proportions are to be kept, recalculate width and height
		if ((ratio_x - ratio_y) > 0.00001) { //  if not equal
			double ratio = 0;

			if (ResizeMode_Clip == mode)
				ratio = std::max (ratio_x, ratio_y);
			else if (ResizeMode_LeaveBorder == mode)
				ratio = std::max (ratio_x, ratio_y);
			else // average
				ratio = (ratio_x + ratio_y) / 2;

			width = uint32_t(orig->get_w() * ratio);
			height = uint32_t(orig->get_h() * ratio);
		}
	}

	PictureID pic = g_gr->create_surface(w, h);

	if (mode == ResizeMode_Loose || (width == w && height == h))
		pic->surface->set_sdl_surface(*resize(index, w, h));
	else {

		Surface src;
		src.set_sdl_surface(*resize(index, width, height));

		// apply rectangle by adjusted size
		Rect srcrc;
		srcrc.w = std::min(w, width);
		srcrc.h = std::min(h, height);
		srcrc.x = (width - srcrc.w) / 2;
		srcrc.y = (height - srcrc.h) / 2;


		g_gr->get_surface_renderer(pic)->blitrect //  Get the rendertarget
			(Point((w - srcrc.w) / 2, (h - srcrc.h) / 2),
			 get_picture(index->module, src), srcrc);
	}

	return pic;
}

/**
 * Produces a resized version of the specified picture
 *
 * \param index position of the source picture in the stack
 * \param w target width
 * \param h target height
 * \return resized version of picture
 */
SDL_Surface * Graphic::resize
	(const PictureID index, uint32_t const w, uint32_t const h)
{
	Surface & orig = *g_gr->get_picture_surface(index);

	return
		zoomSurface
			(orig.get_sdl_surface(),
			 double(w) / orig.get_w(), double(h) / orig.get_h(),
			 1);
}


/**
 * Stores the picture size in pw and ph.
 * Throws an exception if the picture doesn't exist.
*/
void Graphic::get_picture_size
	(const PictureID & pic, uint32_t & w, uint32_t & h) const
{
  //if (pic >= m_pictures.size() || !m_pictures[pic].module)
  //throw wexception("get_picture_size(%i): picture does not exist", pic);

	assert (pic->surface);
	Surface & bmp = *pic->surface;

	w = bmp.get_w();

	h = bmp.get_h();
}

void Graphic::save_png(const PictureID & pic_index, StreamWrite * const sw)
{
	Surface & surf = *get_picture_surface(pic_index);

	// Save a png
	png_structp png_ptr =
		png_create_write_struct
			(PNG_LIBPNG_VER_STRING, static_cast<png_voidp>(0), 0, 0);

	if (!png_ptr)
		throw wexception("Graphic::save_png: could not create png struct");

	//  Set another write function. This is potentially dangerouse because the
	//  flush function is internally called by png_write_end(), this will crash
	//  on newer libpngs. See here:
	//     https://bugs.freedesktop.org/show_bug.cgi?id=17212
	//
	//  Simple solution is to define a dummy flush function which I did here.
	png_set_write_fn
		(png_ptr,
		 sw,
		 &Graphic::m_png_write_function, &Graphic::m_png_flush_function);

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, static_cast<png_infopp>(0));
		throw wexception("Graphic::save_png: could not create png info struct");
	}

	// Set jump for error
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		throw wexception("Graphic::save_png: could not set png setjmp");
	}

	// Fill info struct
	png_set_IHDR
		(png_ptr, info_ptr, surf.get_w(), surf.get_h(),
		 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// png_set_strip_16(png_ptr) ;

	// Start writing
	png_write_info(png_ptr, info_ptr);

	// Strip data down
	png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

	png_set_packing(png_ptr);

	{
		uint32_t surf_w = surf.get_w();
		uint32_t surf_h = surf.get_h();
		uint32_t row_size = 4 * surf_w;
		SDL_PixelFormat & format = const_cast<SDL_PixelFormat &>(surf.format());

	// Write each row
		for (uint32_t y = 0; y < surf_h; ++y) {
			png_byte row[row_size];
			png_bytep rowp = row;
			for (uint32_t x = 0; x < surf_w; rowp += 4, ++x)
				SDL_GetRGBA
					(surf.get_pixel(x, y), &format,
					 rowp + 0, rowp + 1, rowp + 2, rowp + 3);
			png_write_row(png_ptr, row);
		}
	}

	// End write
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

/**
 * Create an offscreen surface that can be used both as target and as source for
 * rendering operations. The surface is put into a normal slot in the picture
 * array so the surface can be used in normal blit() operations.
 * A RenderTarget for the surface can be obtained using get_surface_renderer().
 * \note Surfaces do not belong to a module and must be freed explicitly.
*/
PictureID Graphic::create_surface(int32_t w, int32_t h)
{
	const SDL_PixelFormat & format = m_screen.format();
	SDL_Surface & surf =
		*SDL_CreateRGBSurface
			(SDL_SWSURFACE,
			 w, h,
			 format.BitsPerPixel,
			 format.Rmask, format.Gmask, format.Bmask, format.Amask);

	Picture & pic = *new Picture();
	PictureID id = PictureID(&pic);
	m_picturemap[PicSurface].insert(std::make_pair("", id));

	pic.module    = PicSurface; // mark as surface
	pic.surface   = new Surface();
	pic.surface->set_sdl_surface(surf);
	assert(pic.rendertarget == 0);
	pic.rendertarget = new RenderTarget(pic.surface);

	return id;
}

/**
 * Free the given surface.
 * Unlike normal pictures, surfaces are not freed by flush().
*/
void Graphic::free_surface(const PictureID & picid) {
	//assert(picid < m_pictures.size());
	if
		(picid->module != PicMod_Font &&
		 picid->module != PicSurface)
	{
		log
			("Graphic::free_surface ignoring free of %u %s\n",
			 picid->module, picid->fname);
		return;
	}
	assert
		(picid->module == PicMod_Font
		 ||
		 picid->module == PicSurface);

	if (picid->surface) {
	  delete picid->surface;
	  picid->surface = 0;
	}
	if (picid->rendertarget) {
	  delete picid->rendertarget;
	  picid->rendertarget = 0;
	}
	if (picid->fname) {
	  delete picid->fname;
	  picid->fname = 0;
	}
	//Picture & pic = m_pictures[picid];

	//delete picid->rendertarget;
	//pic.rendertarget = 0;
	//delete picid->fname;
	//pic.fname = 0;
	//delete pic->surface;
	//pic.surface = 0;
	//pic.module = 0;
	pmit b, e = m_picturemap[picid->module].end();
	for (b = m_picturemap[picid->module].begin(); b != e; ++b) {
		if (b->second == picid)
			break;
	}
	if (b != e) {
		m_picturemap[picid->module].erase(b);
	}
}


PictureID Graphic::create_grayed_out_pic(const PictureID & picid) {
	if (picid == get_no_picture()) {
		Surface & s = *new Surface(*get_picture_surface(picid));
		SDL_PixelFormat const & format = s.format();
		uint32_t const w = s.get_w(), h = s.get_h();
		for (uint32_t y = 0; y < h; ++y)
			for (uint32_t x = 0; x < w; ++x) {
				uint8_t r, g, b, a;

				//  FIXME need for const_cast is SDL bug #421
				SDL_GetRGBA
					(s.get_pixel(x, y),
					 &const_cast<SDL_PixelFormat &>(format), &r, &g, &b, &a);

				//  Halve the opacity to give some difference for pictures that are
				//  grayscale to begin with.
				a >>= 1;

				uint8_t const gray =
					(luminance_table_r[r] +
					 luminance_table_g[g] +
					 luminance_table_b[b] +
					 8388608U) //  compensate for truncation:  .5 * 2^24
					>> 24;

				// NOTE const_cast is needed for SDL-1.2 older than revision 3008
				s.set_pixel
					(x, y, SDL_MapRGBA
					 	(&const_cast<SDL_PixelFormat &>(format),
					 	 gray, gray, gray, a));

			}
		return get_picture(PicSurface, s);
	} else
		return get_no_picture();
}


/**
 * Returns the RenderTarget for the given surface
*/
RenderTarget * Graphic::get_surface_renderer(const PictureID & pic) {
	//assert(pic < m_pictures.size());
	//  assert(m_pictures[pic].module == 0xff); fails showing terrains in editor

	RenderTarget & rt = *pic->rendertarget;

	rt.reset();

	return &rt;
}

/**
 * Creates a terrain texture.
 *
 * fnametempl is a filename with possible wildcard characters '?'. The function
 * fills the wildcards with decimal numbers to get the different frames of a
 * texture animation. For example, if fnametempl is "foo_??.bmp", it tries
 * "foo_00.bmp", "foo_01.bmp" etc...
 * frametime is in milliseconds.
 * \return 0 if the texture couldn't be loaded.
 * \note Terrain textures are not reused, even if fnametempl matches.
 * These textures are freed when PicMod_Game is flushed.
*/
uint32_t Graphic::get_maptexture
	(const char & fnametempl, const uint32_t frametime)
{
	try {
		m_maptextures.push_back
			(new Texture(fnametempl, frametime, m_screen.format()));
	} catch (std::exception const & e) {
		log("Failed to load maptexture %s: %s\n", &fnametempl, e.what());
		return 0;
	}

	return m_maptextures.size(); // ID 1 is at m_maptextures[0]
}

/**
 * Advance frames for animated textures
*/
void Graphic::animate_maptextures(uint32_t time)
{
	for (uint32_t i = 0; i < m_maptextures.size(); ++i)
		m_maptextures[i]->animate(time);
}

/**
 * reset that the map texture have been animated
 */
void Graphic::reset_texture_animation_reminder()
{
	for (uint32_t i = 0; i < m_maptextures.size(); ++i)
		m_maptextures[i]->reset_was_animated();
}

/**
 * Load all animations that are registered with the AnimationManager
*/
void Graphic::load_animations(UI::ProgressWindow & loader_ui) {
	assert(m_animations.empty());

	const std::string step_description = _("Loading animations: %d%% complete");
	uint32_t last_shown = 100;
	const uint32_t nr_animations = g_anim.get_nranimations();
	for (uint32_t id = 0; id < nr_animations;) {
		const uint32_t percent = 100 * id / nr_animations;
		if (percent != last_shown) {
			last_shown = percent;
			loader_ui.stepf(step_description, percent);
		}
		++id;
		m_animations.push_back(new AnimationGfx(g_anim.get_animation(id)));
	}
}

/**
 * Return the number of frames in this animation
 */
AnimationGfx::Index Graphic::nr_frames(const uint32_t anim) const
{
	return get_animation(anim)->nr_frames();
}

/**
 * \return the size of the animation at the given time.
*/
void Graphic::get_animation_size
	(uint32_t const anim, uint32_t const time, uint32_t & w, uint32_t & h)
{
	AnimationData const * data = g_anim.get_animation(anim);
	AnimationGfx        * gfx  = get_animation(anim);
	Surface * frame;

	if (!data || !gfx) {
		log("WARNING: Animation %u does not exist\n", anim);
		w = h = 0;
	} else {
		// Get the frame and its data. Ignore playerclrs.
		frame =
			gfx->get_frame((time / data->frametime) % gfx->nr_frames(), 0, 0);

		w = frame->get_w();
		h = frame->get_h();
	}
}

/**
 * Save a screenshot in the given file.
*/
void Graphic::screenshot(const char & fname) const
{
	// TODO: this is incorrect; it bypasses the files code
	m_screen.save_bmp(fname);
}

/**
 * \return Filename of texture of given ID.
*/
char const * Graphic::get_maptexture_picture(uint32_t const id)
{
	if (Texture * const tex = get_maptexture_data(id))
		return tex->get_texture_picture();
	else
		return 0;
}

/**
 * Save and load pictures
 */
void Graphic::m_png_write_function
	(png_structp png_ptr, png_bytep data, png_size_t length)
{
	static_cast<StreamWrite *>(png_get_io_ptr(png_ptr))->Data(data, length);
}
/**
 * Flush function to avoid crashes with default libpng flush function
 */
void Graphic::m_png_flush_function
	(png_structp png_ptr)
{
	static_cast<StreamWrite *>(png_get_io_ptr(png_ptr))->Flush();
}

/**
 * Find a free picture slot and return it.
 * FIXME: remove this function
 */
/*
PictureID Graphic::find_free_picture()
{
	const PictureID pictures_size = m_pictures.end();
	PictureID id = m_pictures.begin();

	for (; id != pictures_size; ++id)
		if (id->module == 0)
			return id;

	m_pictures.resize(m_picture);

	return id;
}
*/
/**
 * Returns the bitmap that belongs to the given picture ID.
 * May return 0 if the given picture does not exist.
*/
Surface * Graphic::get_picture_surface(const PictureID & id)
{
	//assert(id != get_no_picture());

	return id->surface;
}

const Surface * Graphic::get_picture_surface(const PictureID & id) const
{
	return id->surface;
}

/**
 * Retrieve the animation graphics
*/
AnimationGfx * Graphic::get_animation(uint32_t const anim) const
{
	if (!anim || anim > m_animations.size())
		return 0;

	return m_animations[anim - 1];
}

/**
 * \return the actual texture data associated with the given ID.
*/
Texture * Graphic::get_maptexture_data(uint32_t id)
{
	--id; // ID 1 is at m_maptextures[0]

	if (id < m_maptextures.size())
		return m_maptextures[id];
	else
		return 0;
}

/**
 * \return The road textures
*/
Surface * Graphic::get_road_texture(int32_t const roadtex)
{
	if (not m_roadtextures) {

		// Load the road textures
		m_roadtextures = new Road_Textures();
		m_roadtextures->pic_road_normal =
			get_picture(PicMod_Game, ROAD_NORMAL_PIC);

		m_roadtextures->pic_road_busy = get_picture(PicMod_Game, ROAD_BUSY_PIC);

		get_picture_surface
			(m_roadtextures->pic_road_normal)->force_disable_alpha();

		get_picture_surface(m_roadtextures->pic_road_busy)->force_disable_alpha();
	}

	return
		get_picture_surface
			(roadtex == Widelands::Road_Normal ?
			 m_roadtextures->pic_road_normal : m_roadtextures->pic_road_busy);
}
