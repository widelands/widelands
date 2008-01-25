/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "build_id.h"
#include "fileread.h"
#include "filesystem/layered_filesystem.h"
#include "streamwrite.h"
#include "font_handler.h"
#include "i18n.h"
#include "rendertarget.h"
#include "wexception.h"
#include "widelands_fileread.h"

#include "ui_progresswindow.h"

#include "log.h"

#include <SDL_image.h>
#include <SDL_rotozoom.h>

Graphic *g_gr;

/**
 * Helper function wraps around SDL_image. Returns the given image file as a
 * surface.
 * Cannot return 0, throws an exception on error.
 * \todo Get rid of this function
*/
SDL_Surface* LoadImage(const char * const filename)
{
	FileRead fr;
	SDL_Surface* surf;

	fr.Open(*g_fs, filename);

	surf = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);

	if (!surf)
		throw wexception("%s", IMG_GetError());

	return surf;
}

/**
 * Initialize the SDL video mode.
*/
Graphic::Graphic(int32_t w, int32_t h, int32_t bpp, bool fullscreen)
:
m_nr_update_rects   (0),
m_update_fullscreen(false),
m_roadtextures     (0)
{
	// Set video mode using SDL
	int32_t flags = SDL_SWSURFACE;

	if (fullscreen)
		flags |= SDL_FULLSCREEN;

	SDL_Surface* sdlsurface = SDL_SetVideoMode(w, h, bpp, flags);

	if (!sdlsurface)
		throw wexception("Couldn't set video mode: %s", SDL_GetError());

	assert(sdlsurface->format->BytesPerPixel == 2 || sdlsurface->format->BytesPerPixel == 4);

	SDL_WM_SetCaption("Widelands " BUILD_ID, "Widelands");

	m_screen.set_sdl_surface(*sdlsurface);

	m_rendertarget = new RenderTarget(&m_screen);
}

/**
 * Free the surface
*/
Graphic::~Graphic()
{
	flush(PicMod_UI | PicMod_Menu | PicMod_Game);
	delete m_roadtextures;
	delete m_rendertarget;

	std::vector<Picture>::const_iterator const pictures_end = m_pictures.end();
	for
		(std::vector<Picture>::const_iterator it = m_pictures.begin();
		 it != pictures_end;
		 ++it)
		if (it->module)
			log
				("WARNING: picture with module %u has not been freed "
				 "(u.{fname,rendertarget} = %p)\n",
				 it->module, it->u.fname);
}

/**
 * Return the screen x resolution
*/
int32_t Graphic::get_xres()
{
	return m_screen.get_w();
}

/**
 * Return the screen x resolution
*/
int32_t Graphic::get_yres()
{
	return m_screen.get_h();
}

/**
 * Return a pointer to the RenderTarget representing the screen
*/
RenderTarget* Graphic::get_render_target()
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
void Graphic::flush(uint8_t const module) {
	assert(not (module & (PicMod_Font | PicSurface)));
	// Flush pictures

	for (uint32_t i = 0; i < m_pictures.size(); ++i) {
		Picture & pic = m_pictures[i];

		//      NoLog("Flushing picture: %i while flushing all!\n", i);

		if (!pic.module)
			continue;


		if (pic.module == PicSurface) {
			if (!module)
				log("LEAK: SW16: flush(0): non-picture %i left.\n", i+1);

			continue;
		}

		pic.module &= ~module; // unmask the modules that should be flushed

		// Once the picture is no longer in any mods, free it

		if (!pic.module) {

			if (pic.u.fname) {
				m_picturemap.erase(pic.u.fname);
				free(pic.u.fname);
				pic.u.fname = 0;
			}

			delete pic.surface;
			pic.surface = 0;
		}
	}

	// Flush game items
	if (!module || module & PicMod_Game) {
		for (uint32_t i = 0; i < m_maptextures.size(); ++i)
			delete m_maptextures[i];
		m_maptextures.clear();

		for (uint32_t i = 0; i < m_animations.size(); ++i)
			delete m_animations[i];
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
uint32_t Graphic::get_picture(uint8_t const module, char const * const fname) {
	std::vector<Picture>::size_type id;

	//  Check if the picture is already loaded.
	const picmap_t::const_iterator it = m_picturemap.find(fname);

	if (it != m_picturemap.end()) {
		id = it->second;
	} else {
		SDL_Surface* bmp;

		try {
			bmp = LoadImage(fname);
			//log("Graphic::get_picture(): loading picture '%s'\n", fname);
		} catch (std::exception& e) {
			log("WARNING: Couldn't open %s: %s\n", fname, e.what());
			return 0;
		}

		// Convert the surface accordingly
		SDL_Surface & use_surface = *SDL_DisplayFormatAlpha(bmp);

		SDL_FreeSurface(bmp);

		// Fill in a free slot in the pictures array
		id = find_free_picture();

		Picture & pic = m_pictures[id];

		pic.module    = 0; // will be filled in by caller

		pic.u.fname   = strdup(fname);

		assert(pic.u.fname); //  FIXME no proper check for NULL return value!

		pic.surface   = new Surface();

		pic.surface->set_sdl_surface(use_surface);

		m_picturemap[fname] = id;
	}

	m_pictures[id].module |= module;

	return id;
}

uint32_t Graphic::get_picture
(uint32_t const module, Surface & surf, char const * const fname)
{
	const std::vector<Picture>::size_type id = find_free_picture();
	Picture & pic = m_pictures[id];
	pic.module    = module;
	pic.surface   = &surf;

	if (fname) {
		pic.u.fname = strdup(fname);
		m_picturemap[fname] = id;
	} else pic.u.fname =  0;

	return id;
}

/**
 * Produces a resized version of the specified picture
 *
 * Might return same id if dimensions are the same
 */
uint32_t Graphic::get_resized_picture
(const uint32_t index, const uint32_t w, const uint32_t h, ResizeMode mode)
{
	if (index >= m_pictures.size() or !m_pictures[index].module)
		throw wexception("get_resized_picture(%i): picture doesn't exist", index);

	Surface* orig = m_pictures[index].surface;
	if (orig->get_w() == w and orig->get_h() == h)
		return index;

	uint32_t width = w;
	uint32_t height = h;

	if (mode != ResizeMode_Loose) {
		const double ratio_x = double(w) / orig->get_w();
		const double ratio_y = double(h) / orig->get_h();

		//  if proportions are to be kept, recalculate width and height
		if (ratio_x != ratio_y) {
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

	const uint32_t pic = g_gr->create_surface(w, h);

	if (mode == ResizeMode_Loose || (width == w && height == h)) {
		SDL_Surface * const resized = resize(index, w, h);
		Surface* s = m_pictures[pic].surface;
		s->set_sdl_surface(*resized);
	} else {

		SDL_Surface * const resized = resize(index, width, height);
		Surface src;
		src.set_sdl_surface(*resized);

		// Get the rendertarget for this
		RenderTarget* target = g_gr->get_surface_renderer(pic);

		// apply rectangle by adjusted size
		Rect srcrc;
		srcrc.w = std::min(w, width);
		srcrc.h = std::min(h, height);
		srcrc.x = (width - srcrc.w) / 2;
		srcrc.y = (height - srcrc.h) / 2;

		target->blitrect
			(Point((w - srcrc.w) / 2, (h - srcrc.h) / 2),
			 get_picture(m_pictures[index].module, src), srcrc);
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
SDL_Surface* Graphic::resize(const uint32_t index, const uint32_t w, const uint32_t h)
{
	Surface *orig = g_gr->get_picture_surface(index);
	double zoomx = double(w) / orig->get_w();
	double zoomy = double(h) / orig->get_h();

	return zoomSurface(orig->get_sdl_surface(), zoomx, zoomy, 1);
}


/**
 * Stores the picture size in pw and ph.
 * Throws an exception if the picture doesn't exist.
*/
void Graphic::get_picture_size(const uint32_t pic, uint32_t & w, uint32_t & h)
{
	if (pic >= m_pictures.size() || !m_pictures[pic].module)
		throw wexception("get_picture_size(%i): picture doesn't exist", pic);

	Surface* bmp = m_pictures[pic].surface;

	w = bmp->get_w();

	h = bmp->get_h();
}

void Graphic::save_png(uint32_t pic_index, StreamWrite * const sw)
{
	Surface & surf = *get_picture_surface(pic_index);

	// Save a png
	png_structp png_ptr =
		png_create_write_struct
		(PNG_LIBPNG_VER_STRING, static_cast<png_voidp>(0), 0, 0);

	if (!png_ptr)
		throw wexception("Graphic::save_png: Couldn't create png struct!");

	// Set another write function
	png_set_write_fn(png_ptr, sw, &Graphic::m_png_write_function, 0);

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, static_cast<png_infopp>(NULL));
		throw wexception("Graphic::save_png: Couldn't create png info struct!");
	}

	// Set jump for error
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		throw wexception("Graphic::save_png: Couldn't set png setjmp!");
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
uint32_t Graphic::create_surface(int32_t w, int32_t h)
{
	const SDL_PixelFormat & format = m_screen.format();
	SDL_Surface & surf =
		*SDL_CreateRGBSurface
		(SDL_SWSURFACE,
		 w, h,
		 format.BitsPerPixel,
		 format.Rmask, format.Gmask, format.Bmask, format.Amask);

	const std::vector<Picture>::size_type id = find_free_picture();
	Picture & pic = m_pictures[id];
	pic.module    = PicSurface; // mark as surface
	pic.surface   = new Surface();
	pic.surface->set_sdl_surface(surf);
	pic.u.rendertarget = new RenderTarget(pic.surface);

	return id;
}

/**
 * Free the given surface.
 * Unlike normal pictures, surfaces are not freed by flush().
*/
void Graphic::free_surface(uint32_t const picid) {
	assert(picid < m_pictures.size());
	assert
		(m_pictures[picid].module == PicMod_Font
		 ||
		 m_pictures[picid].module == PicSurface);

	Picture & pic = m_pictures[picid];

	delete pic.u.rendertarget;
	pic.u.rendertarget = 0;
	delete pic.surface;
	pic.surface = 0;
	pic.module = 0;
}

/**
 * Returns the RenderTarget for the given surface
*/
RenderTarget* Graphic::get_surface_renderer(uint32_t pic)
{
	assert(pic < m_pictures.size());
	assert(m_pictures[pic].module == 0xff);

	RenderTarget* rt = m_pictures[pic].u.rendertarget;

	rt->reset();

	return rt;
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
uint32_t Graphic::get_maptexture(const char & fnametempl, const uint32_t frametime)
{
	try {
		m_maptextures.push_back
		(new Texture(fnametempl, frametime, m_screen.format()));
	} catch (std::exception& e) {
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
	assert(!m_animations.size());

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
(const uint32_t anim, const uint32_t time, uint32_t & w, uint32_t & h)
{
	const AnimationData* data = g_anim.get_animation(anim);
	AnimationGfx* gfx = get_animation(anim);
	Surface* frame;

	if (!data || !gfx) {
		log("WARNING: Animation %i doesn't exist\n", anim);
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
const char* Graphic::get_maptexture_picture(uint32_t id)
{
	Texture* tex = get_maptexture_data(id);

	if (tex)
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
 * Find a free picture slot and return it.
*/
std::vector<Picture>::size_type Graphic::find_free_picture()
{
	const std::vector<Picture>::size_type pictures_size = m_pictures.size();
	std::vector<Picture>::size_type id = 1;

	for (; id < pictures_size; ++id)
		if (m_pictures[id].module == 0)
			return id;

	m_pictures.resize(id+1);

	return id;
}

/**
 * Returns the bitmap that belongs to the given picture ID.
 * May return 0 if the given picture does not exist.
*/
Surface* Graphic::get_picture_surface(uint32_t id)
{
	if (id >= m_pictures.size())
		return 0;

	if (!m_pictures[id].module)
		return 0;

	return m_pictures[id].surface;
}

/**
 * Retrieve the animation graphics
*/
AnimationGfx* Graphic::get_animation(const uint32_t anim) const
{
	if (!anim || anim > m_animations.size())
		return 0;

	return m_animations[anim-1];
}

/**
 * \return the actual texture data associated with the given ID.
*/
Texture* Graphic::get_maptexture_data(uint32_t id)
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
Surface* Graphic::get_road_texture(int32_t roadtex)
{
	if (not m_roadtextures) {
		// Load the road textures
		m_roadtextures = new Road_Textures();
		m_roadtextures->pic_road_normal = get_picture(PicMod_Game, ROAD_NORMAL_PIC);
		m_roadtextures->pic_road_busy   = get_picture(PicMod_Game, ROAD_BUSY_PIC);
		get_picture_surface(m_roadtextures->pic_road_normal)->force_disable_alpha();
		get_picture_surface(m_roadtextures->pic_road_busy)->force_disable_alpha();
	}

	return get_picture_surface
		(roadtex == Widelands::Road_Normal ?
		 m_roadtextures->pic_road_normal : m_roadtextures->pic_road_busy);
}
