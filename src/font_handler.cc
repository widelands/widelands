/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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
 * Some Methods taken from Wesnoth.
 * http://www.wesnoth.org
 */

#include "font_handler.h"

#include "io/filesystem/filesystem.h"
#include "font_loader.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "wexception.h"
#include "text_parser.h"

#include <SDL_image.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <iostream>

/**
 * Plain Constructor
 */
Font_Handler::Font_Handler() {
	if (TTF_Init() == -1)
		throw wexception
			("True Type library did not initialize: %s\n", TTF_GetError());
	m_font_loader = new Font_Loader();
	m_varcallback = 0;
	m_cbdata = 0;
}


Font_Handler::~Font_Handler() {
	delete m_font_loader;
	TTF_Quit();
}


/*
 * Returns the height of the font, in pixels.
*/
uint32_t Font_Handler::get_fontheight
	(std::string const & name, int32_t const size)
{
	TTF_Font * const f = m_font_loader->get_font(name, size);
	const int32_t fontheight = TTF_FontHeight(f);
	if (fontheight < 0)
		throw wexception
			("TTF_FontHeight returned a negative value, which does not have a "
			 "known meaning.");
	return fontheight;
}

/*
 * Draw this string, if it is not cached, create the cache for it.
 *
 * The whole text block is rendered in one Surface, this surface is cached
 * for reuse.
 * This is a really fast approach for static texts, but for text areas which keep changing
 * (like Multiline editboxes or chat windows, debug windows ...) this is the death, for a whole new
 * surface is rendered with everything that has been written so far.
 */
// TODO: rename this to draw text
void Font_Handler::draw_string
	(RenderTarget & dst,
	 std::string const & fontname,
	 int32_t const fontsize, RGBColor const fg, RGBColor const bg,
	 Point                     dstpoint,
	 std::string const &       text,
	 Align               const align,
	 int32_t             const wrap,
	 Widget_Cache        const widget_cache,
	 uint32_t          * const widget_cache_id,
	 int32_t             const caret,
	 bool                const transparent)
{
	TTF_Font & font = *m_font_loader->get_font(fontname, fontsize);
	//Width and height of text, needed for alignment
	uint32_t w, h;
	uint32_t picid;
	//Fontrender takes care of caching
	if (widget_cache == Widget_Cache_None) {
		// look if text is cached
		_Cache_Infos  ci = {0, text, &font, fg, bg, caret, 0, 0};

		std::list<_Cache_Infos>::iterator i =
			find(m_cache.begin(), m_cache.end(), ci);

		if (i != m_cache.end())  {
			// Ok, it is cached, blit it and done
			picid = i->surface_id;
			w = i->w;
			h = i->h;
			if (i != m_cache.begin()) {
				m_cache.push_front (*i);
				m_cache.erase (i);
			}
		}
		else {
			//not cached, create a new surface
			ci.surface_id =
				create_text_surface
				(font, fg, bg, text, align, wrap, caret, transparent);
			// Now cache it
			g_gr->get_picture_size(ci.surface_id, ci.w, ci.h);
			ci.f = &font;
			m_cache.push_front (ci);

			while (m_cache.size() > CACHE_ARRAY_SIZE) {
				g_gr->free_surface(m_cache.back().surface_id);
				m_cache.pop_back();
			}
			//Set for alignment and blitting
			picid = ci.surface_id;
			w = ci.w;
			h = ci.h;
		}
	}
	//Widget gave us an explicit picid
	else if (widget_cache == Widget_Cache_Use) {
		g_gr->get_picture_size(*widget_cache_id, w, h);
		picid = *widget_cache_id;
	}
	//We need to (re)create the picid for the widget
	else {
		if (widget_cache == Widget_Cache_Update)
			g_gr->free_surface(*widget_cache_id);
		*widget_cache_id =
			create_text_surface
				(font, fg, bg, text, align, wrap, caret, transparent);
		g_gr->get_picture_size(*widget_cache_id, w, h);
		picid = *widget_cache_id;
	}
	do_align(align, dstpoint.x, dstpoint.y, w, h);
	dst.blit(dstpoint, picid);
}

/*
* Creates a Widelands surface of the given text, checks if multiline or not
*/
uint32_t Font_Handler::create_text_surface
	(TTF_Font & f, RGBColor const fg, RGBColor const bg,
	 std::string const & text, Align const align, int32_t const wrap,
	 int32_t const caret, bool const transparent)
{
	return
		convert_sdl_surface
			(*
			 (wrap > 0 ?
			  create_static_long_text_surface
			  	(f, fg, bg, text, align, wrap, 0, caret)
			  :
			  create_single_line_text_surface(f, fg, bg, text, align, caret)),
			 bg, transparent);
}

/*
 * This function renders a short (single line) text surface
 */
SDL_Surface * Font_Handler::create_single_line_text_surface
	(TTF_Font & font,
	 RGBColor const fg, RGBColor const bg,
	 std::string       text,
	 Align,
	 int32_t     const caret)
{
	// render this block in a SDL Surface
	SDL_Color sdl_fg = {fg.r(), fg.g(), fg.b(), 0};
	SDL_Color sdl_bg = {bg.r(), bg.g(), bg.b(), 0};

	if (text.empty())
		text = " ";

	if
		(SDL_Surface * const surface =
		 	TTF_RenderUTF8_Shaded(&font, text.c_str(), sdl_fg, sdl_bg))
	{
		if (caret != -1) {
			std::string const text_caret_pos = text.substr(0, caret);
			render_caret(font, *surface, text_caret_pos);
		}
		return surface;
	} else {
		log
			("Font_Handler::create_single_line_text_surface, an error : %s\n",
			 TTF_GetError());
		log("Text was: '%s'\n", text.c_str());
		return 0; // This will skip this line hopefully
	}
}

/*
 * This function renders a longer (multiline) text passage, which should not change.
 * If it changes, this function is highly unperformant.
 *
 * This function also completely ignores vertical aligement.
 * Horizontal alignment is now recognized correctly
 */
SDL_Surface * Font_Handler::create_static_long_text_surface
	(TTF_Font & font,
	 RGBColor const fg, RGBColor const bg,
	 std::string       text,
	 Align       const align,
	 int32_t     const wrap,
	 int32_t     const line_spacing,
	 int32_t           caret)
{
	assert(wrap > 0);
	assert(text.size() > 0);

	int32_t global_surface_width  = wrap > 0 ? wrap : 0;
	int32_t global_surface_height = 0;
	std::vector<SDL_Surface *> m_rendered_lines;

	SDL_Color sdl_fg = {fg.r(), fg.g(), fg.b(), 0};
	SDL_Color sdl_bg = {bg.r(), bg.g(), bg.b(), 0};

	uint32_t cur_text_pos = 0;
	uint32_t i = 0;

	text = word_wrap_text(font, text, wrap);
	const std::vector<std::string> lines(split_string(text, "\n"));
	container_iterate_const(std::vector<std::string>, lines, j) {
		std::string const line(j.current->empty() ? " " : *j.current);

		// render this block in a SDL Surface
		if
			(SDL_Surface * const surface =
			 	TTF_RenderUTF8_Shaded(&font, line.c_str(), sdl_fg, sdl_bg))
		{
		uint32_t new_text_pos = cur_text_pos + line.size();
		if (caret != -1) {
			if (new_text_pos >= caret - i) {
				int32_t caret_line_pos = caret - cur_text_pos - i;
				std::string const text_caret_pos = line.substr(0, caret_line_pos);
				render_caret(font, *surface, text_caret_pos);
				caret = -1;
			}
			else {
				cur_text_pos = new_text_pos;
			}
			++i;
		}

		m_rendered_lines.push_back(surface);
		global_surface_height += surface->h + line_spacing;
		if (global_surface_width < surface->w)
			global_surface_width = surface->w;
		} else {
			log
				("Font_Handler::create_static_long_text_surface, an error : %s\n",
				 TTF_GetError());
			log("Text was: %s\n", text.c_str());
		}

	}

	// blit all this together in one Surface
	return
		join_sdl_surfaces
		(global_surface_width, global_surface_height,
		 m_rendered_lines, bg, align, line_spacing);

}

void Font_Handler::render_caret
	(TTF_Font          & font,
	 SDL_Surface       & line,
	 std::string const & text_caret_pos)
{
	int32_t caret_x, caret_y;

	TTF_SizeUTF8(&font, text_caret_pos.c_str(), &caret_x, &caret_y);

	Surface * const caret_surf =
		g_gr->get_picture_surface
			(g_gr->get_picture(PicMod_Game, "pics/caret.png"));
	SDL_Surface * const caret_surf_sdl = caret_surf->m_surface;

	SDL_Rect r;
	r.x = caret_x - caret_surf_sdl->w;
	r.y = (caret_y - caret_surf_sdl->h) / 2;

	SDL_BlitSurface(caret_surf_sdl, 0, &line, &r);
}

/*
* Renders a string into a SDL surface
* Richtext works with this method, because whole richtext content
* is blit into one big surface by the richtext widget itself
*/
SDL_Surface * Font_Handler::draw_string_sdl_surface
	(std::string const & fontname, int32_t const fontsize,
	 RGBColor const fg, RGBColor const bg,
	 std::string const &       text,
	 Align               const align,
	 int32_t             const wrap,
	 int32_t             const style,
	 int32_t             const line_spacing)
{
	TTF_Font & font = *m_font_loader->get_font(fontname, fontsize);
	TTF_SetFontStyle(&font, style);
	return
		create_sdl_text_surface(font, fg, bg, text, align, wrap, line_spacing);
}

/*
* Creates the SDL surface, checks if multiline or not
*/
SDL_Surface * Font_Handler::create_sdl_text_surface
	(TTF_Font & font, RGBColor const fg, RGBColor const bg,
	 std::string const & text,
	 Align const align, int32_t const wrap, int32_t const line_spacing)
{
	return
		(wrap > 0  ?
		 create_static_long_text_surface
		 	(font, fg, bg, text, align, wrap, line_spacing)
		 :
		 create_single_line_text_surface(font, fg, bg, text, align));
}

/*
 * draws richtext, specified by blocks
 *
 * If transparent is true, background is transparent,
 * and \param bg is used only for antialiasing
 */
void Font_Handler::draw_richtext
	(RenderTarget &       dst,
	 RGBColor       const bg,
	 Point          const dstpoint,
	 std::string          text,
	 int32_t              wrap,
	 Widget_Cache         widget_cache,
	 uint32_t     * const widget_cache_id,
	 bool           const transparent)
{
	uint32_t picid;
	if (widget_cache == Widget_Cache_Use) {
		//g_gr->get_picture_size(*widget_cache_id, &w, &h);
		picid = *widget_cache_id;
	}
	else {
		if (widget_cache == Widget_Cache_Update) {
			g_gr->free_surface(*widget_cache_id);
		}
		std::vector<Richtext_Block> blocks;
		Text_Parser p;
		p.parse(text, blocks, m_varcallback, m_cbdata);

		std::vector<SDL_Surface *> rend_blocks;
		int32_t global_h = 0;

		//Iterate over richtext blocks
		//Seems to be a problem with loading images, and freeing them
		//Refactor to using datastructure
		for
			(std::vector<Richtext_Block>::iterator richtext_it = blocks.begin();
			 richtext_it != blocks.end();
			 ++richtext_it)
		{
			int32_t cur_line_w = 0;
			int32_t cur_line_h = 0;
			int32_t block_h = 0;

			std::vector<Text_Block> cur_text_blocks = richtext_it->get_text_blocks();
			std::vector<std::string> cur_block_images = richtext_it->get_images();

			std::vector<SDL_Surface *> rend_lines;
			std::vector<SDL_Surface *> rend_cur_words;
			std::vector<SDL_Surface *> rend_cur_images;

			int32_t img_surf_h = 0;
			int32_t img_surf_w = 0;

			//First render all images of this richtext block
			for
				(std::vector<std::string>::const_iterator img_it =
				 cur_block_images.begin();
				 img_it != cur_block_images.end();
				 ++img_it)
			{
				SDL_Rect img_pos;
				img_pos.x = img_surf_w;
				img_pos.y = 0;
				if
					(Surface * const image =
					 g_gr->get_picture_surface //  Not Font, but Game.
					 	(g_gr->get_picture(PicMod_Game, img_it->c_str())))
				{
					img_surf_h =
						img_surf_h < static_cast<int32_t>(image->get_h()) ?
						image->get_h() : img_surf_h;
					img_surf_w = img_surf_w + image->get_w();
					rend_cur_images.push_back(image->m_surface);
				}
			}
			SDL_Surface * const block_images =
				rend_cur_images.size() ?
				join_sdl_surfaces
					(img_surf_w, img_surf_h,
					 rend_cur_images,
					 bg,
					 Align_Left,
					 0,
					 true,
					 true)
				:
				0;

			//Width that's left for text in this richtext block
			int32_t h_space = 3;
			int32_t text_width_left = (wrap - img_surf_w) - h_space;

			//Iterate over text blocks of current richtext block
			for
				(std::vector<Text_Block>::iterator text_it =
				 cur_text_blocks.begin();
				 text_it != cur_text_blocks.end();
				 ++text_it)
			{
				std::vector<std::string> words = text_it->get_words();
				std::vector<std::vector<std::string>::size_type> line_breaks =
					text_it->get_line_breaks();

				//Iterate over words of current text block
				uint32_t word_cnt = 0;
				for
					(std::vector<std::string>::iterator word_it = words.begin();
					 word_it != words.end();
					 ++word_it)
				{
					std::string str_word = *word_it;

					int32_t font_style = TTF_STYLE_NORMAL;
					if (text_it->get_font_weight() == "bold")
						font_style |= TTF_STYLE_BOLD;
					if (text_it->get_font_style() == "italic")
						font_style |= TTF_STYLE_ITALIC;
					if (text_it->get_font_decoration() == "underline")
						font_style |= TTF_STYLE_UNDERLINE;

					SDL_Surface * rend_word =
						draw_string_sdl_surface
							(text_it->get_font_face (),
							 text_it->get_font_size (),
							 text_it->get_font_color(),
							 bg,
							 str_word,
							 Align_Left,
							 -1,
							 font_style,
							 text_it->get_line_spacing());

					//is there a break before this word
					//TODO: comparison between signed and unsigned !
					bool const break_before =
						line_breaks.size() && line_breaks[0] == word_cnt;

					//Word doesn't fit into current line, or a break was inserted before
					if (((cur_line_w + rend_word->w) > text_width_left) || break_before) {
						SDL_Surface * const rend_line =
							join_sdl_surfaces
								(cur_line_w, cur_line_h,
								 rend_cur_words,
								 bg,
								 Align_Left,
								 0,
								 true);
						rend_lines.push_back(rend_line);
						block_h += cur_line_h;
						rend_cur_words.clear();

						//Ignore spaces on begin of the line, if another word follows
						if (str_word != " ")
							rend_cur_words.push_back(rend_word);

						//Setting line height and width of new word = first in new line
						cur_line_h = rend_word->h;
						cur_line_w = rend_word->w;

						if (break_before) {
							line_breaks.erase(line_breaks.begin());
							//Look for another break at before this word
							while (line_breaks.size() and line_breaks[0] == word_cnt)
							{
								SDL_Surface * const space = render_space(*text_it, bg);
								rend_lines.push_back(space);
								block_h += space->h;
								line_breaks.erase(line_breaks.begin());
							}
						}
					} else { //  word fits regularly in this line
						rend_cur_words.push_back(rend_word);
						cur_line_w += rend_word->w;
						cur_line_h =
							cur_line_h < rend_word->h ? rend_word->h : cur_line_h;
					}
					//Are there no more words but line breaks left
					if (word_it + 1 == words.end() && line_breaks.size()) {
						if (rend_cur_words.size()) {
							SDL_Surface * const rend_line =
								join_sdl_surfaces
									(cur_line_w, cur_line_h,
									 rend_cur_words,
									 bg,
									 richtext_it->get_text_align(),
									 0,
									 true);
							rend_lines.push_back(rend_line);
							block_h += cur_line_h;

							rend_cur_words.clear();
							cur_line_h = 0;
							cur_line_w = 0;
							line_breaks.erase(line_breaks.begin());
						}
						while (line_breaks.size()) {
							SDL_Surface *space = render_space(*text_it, bg);
							rend_lines.push_back(space);
							block_h += space->h;
							line_breaks.erase(line_breaks.begin());
						}
					}
					++word_cnt;
				}
			}
			//If there are some words left to blit
			if (rend_cur_words.size()) {
				SDL_Surface * const rend_line =
					join_sdl_surfaces
						(cur_line_w, cur_line_h,
						 rend_cur_words,
						 bg,
						 Align_Left,
						 0,
						 true);
				rend_lines.push_back(rend_line);
				rend_cur_words.clear();
				block_h += cur_line_h;
			}
			if (rend_lines.empty() && rend_cur_images.size()) {
				rend_lines.push_back(create_empty_sdl_surface(1, 1));
			}
			if (rend_lines.size()) {
				int32_t max_x = wrap;

				SDL_Rect img_pos;
				img_pos.x = 0;
				img_pos.y = 0;

				SDL_Rect text_pos;
				text_pos.x = 0;
				text_pos.y = 0;


				if (richtext_it->get_image_align() == Align_Right) {
					img_pos.x = wrap - img_surf_w;
					text_pos.x = 0;
				}
				else if (richtext_it->get_image_align() == Align_HCenter) {
					img_pos.x = (max_x - img_surf_w) / 2;
					text_pos.x = img_pos.x + img_surf_w + h_space;
				}
				else
					text_pos.x = img_surf_w + h_space;

				SDL_Surface * const block_surface =
					create_empty_sdl_surface
						(wrap, (block_h > img_surf_h ? block_h : img_surf_h));

				//  Set background to colorkey
				SDL_FillRect
					(block_surface,
					 0,
					 SDL_MapRGB(block_surface->format,  107, 87, 55));
				SDL_BlitSurface(block_images, 0, block_surface, &img_pos);

				{
					SDL_Surface * block_lines = join_sdl_surfaces
						(text_width_left,
						 block_h,
						 rend_lines,
						 bg,
						 richtext_it->get_text_align());
					SDL_BlitSurface(block_lines, 0, block_surface, &text_pos);
					SDL_FreeSurface(block_lines);
				}
				rend_blocks.push_back(block_surface);

				//If image is higher than text, set block height to image height
				block_h = (block_h < img_surf_h ? img_surf_h : block_h);

				global_h += block_h;

				rend_lines.clear();
			}
			SDL_FreeSurface(block_images);
		}
		picid = convert_sdl_surface
			(*join_sdl_surfaces(wrap, global_h, rend_blocks, bg),
			 bg, transparent);
		*widget_cache_id = picid;
	}
	dst.blit(dstpoint, picid);
}

SDL_Surface * Font_Handler::render_space
	(Text_Block & block, RGBColor const bg, int32_t const style)
{
	SDL_Surface * rend_space = 0;
	rend_space =
		draw_string_sdl_surface
			(block.get_font_face (),
			 block.get_font_size (),
			 block.get_font_color(),
			 bg,
			 " ",
			 Align_Left,
			 -1,
			 style,
			 block.get_line_spacing());
	return rend_space;
}

//gets size of picid
void Font_Handler::get_size_from_cache
	(uint32_t const widget_cache_id, uint32_t & w, uint32_t & h)
{
	g_gr->get_picture_size(widget_cache_id, w, h);
}

//creates an empty sdl surface of given size
SDL_Surface * Font_Handler::create_empty_sdl_surface(uint32_t w, uint32_t h) {
	SDL_Surface * const mask_surf =
		draw_string_sdl_surface
			("FreeSans.ttf", 10, RGBColor(0, 0, 0), RGBColor(0, 0, 0),
			 " ", Align_Left, -1);
	SDL_Surface * const surface =
		SDL_CreateRGBSurface
			(SDL_SWSURFACE, w, h, 16,
			 mask_surf->format->Rmask,
			 mask_surf->format->Gmask,
			 mask_surf->format->Bmask,
			 mask_surf->format->Amask);
	SDL_FreeSurface(mask_surf);
	return surface;
}

//joins a vectror of surfaces in one big surface
SDL_Surface * Font_Handler::join_sdl_surfaces
	(uint32_t const w, uint32_t const h,
	 std::vector<SDL_Surface *> const &       surfaces,
	 RGBColor                           const bg,
	 Align                              const align,
	 int32_t                            const spacing,
	 bool                               const vertical,
	 bool                               const keep_surfaces)
{
	SDL_Surface * global_surface = create_empty_sdl_surface
		(h ? w : 0, w ? h + spacing * surfaces.size() : 0);
	assert(global_surface);

	SDL_FillRect
		(global_surface,
		 0,
		 SDL_MapRGB(global_surface->format, bg.r(), bg.g(), bg.b()));

	int32_t y = 0;
	int32_t x = 0;

	for (uint32_t i = 0; i < surfaces.size(); ++i) {
		SDL_Surface * const s = surfaces[i];
		SDL_Rect r;

		if (vertical) {
			r.x = x;
			r.y = 0;
		} else {
			int32_t alignedX = 0;
			if (align & Align_HCenter)
				alignedX = (w - s->w) / 2;
			else if (align & Align_Right)
				alignedX += (w - s->w);

			r.x = alignedX;
			r.y = y;
		}
		SDL_BlitSurface(s, 0, global_surface, &r);
		y += s->h + spacing;
		x += s->w + (vertical ? spacing : 0);
		if (!keep_surfaces)
			SDL_FreeSurface(s);
	}
	return global_surface;
}

/*
 * Converts a SDLSurface in a widelands one
 *
 * If transparent is true, background is transparent
 */
uint32_t Font_Handler::convert_sdl_surface
	(SDL_Surface & surface, RGBColor const bg, bool const transparent)
{
	Surface & surf = *new Surface();

	if (transparent)
		SDL_SetColorKey
			(&surface, SDL_SRCCOLORKEY,
			 SDL_MapRGB(surface.format, bg.r(), bg.g(), bg.b()));

	surf.set_sdl_surface(surface);

	uint32_t picid = g_gr->get_picture(PicMod_Font, surf);
	return picid;
}

//Sets dstx and dsty to values for a specified align
void Font_Handler::do_align
	(Align const align,
	 int32_t & dstx, int32_t & dsty,
	 int32_t const w, int32_t const h)
{
	//Vertical Align
	if (align & (Align_VCenter|Align_Bottom)) {
		if (align & Align_VCenter)
			dsty -= (h + 1) / 2; //  +1 for slight bias to top
		else
			dsty -= h;
	}

	//Horizontal Align
	if ((align & Align_Horizontal) != Align_Left) {
		if (align & Align_HCenter)
			dstx -= w / 2;
		else if (align & Align_Right)
			dstx -= w;
	}
}

/*
 * Flushes the cached picture ids
 */
void Font_Handler::flush_cache() {
	while (!m_cache.empty()) {
		g_gr->free_surface (m_cache.front().surface_id);
		m_cache.pop_front();
	}
}
//Deletes widget controlled surface
void Font_Handler::delete_widget_cache(uint32_t const widget_cache_id) {
	g_gr->free_surface(widget_cache_id);
}

//Inserts linebreaks into a text, so it doesn't get bigger than max_width when rendered
//Method taken from Wesnoth.
//http://www.wesnoth.org
std::string Font_Handler::word_wrap_text
	(TTF_Font          &       font,
	 std::string const &       unwrapped_text,
	 int32_t             const max_width)
{
	//std::cerr << "Wrapping word " << unwrapped_text << "\n";

	std::string wrapped_text; // the final result

	size_t word_start_pos = 0;
	std::string cur_word; // including start-whitespace
	std::string cur_line; // the whole line so far

	for (size_t c = 0; c < unwrapped_text.length(); ++c) {
		// Find the next word
		bool forced_line_break = false;
		if (c == unwrapped_text.length() - 1) {
			cur_word =
				unwrapped_text.substr(word_start_pos, c + 1 - word_start_pos);
			word_start_pos = c + 1;
		}
		else if (unwrapped_text[c] == '\n') {
			cur_word =
				unwrapped_text.substr(word_start_pos, c + 1 - word_start_pos);
			word_start_pos = c + 1;
			forced_line_break = true;
		}
		else if (unwrapped_text[c] == ' ') {
			cur_word = unwrapped_text.substr(word_start_pos, c - word_start_pos);
			word_start_pos = c;
		}
		else {
			continue;
		}

		// Test if the line should be wrapped or not
		std::string tmp_str = cur_line + cur_word;
		if (calc_linewidth(font, tmp_str) > max_width) {
			if (calc_linewidth(font, cur_word) > (max_width /*/ 2*/)) {
				//  The last word is too big to fit in a nice way. Split it on a
				//  character basis.
				//std::vector<std::string> split_word = split_utf8_string(cur_word);
				for (uint32_t i = 0; i < cur_word.length(); ++i) {
					tmp_str = cur_line + cur_word[i];
					if (calc_linewidth(font, tmp_str) > max_width) {
						wrapped_text += cur_line + '\n';
						cur_line = "";
					}
					else {
						cur_line += cur_word[i];
					}
				}
			}
			else {
				// Split the line on a word basis
				wrapped_text += cur_line + '\n';
				cur_line = remove_first_space(cur_word);
			}
		}
		else {
			cur_line += cur_word;
		}

		if (forced_line_break) {
			wrapped_text += cur_line;
			cur_line = "";
			forced_line_break = false;
		}
	}

	// Don't forget to add the text left in cur_line
	if (cur_line != "") {
		wrapped_text += cur_line;
	}
	return wrapped_text;
}

std::string Font_Handler::word_wrap_text
	(std::string const & fontname,       int32_t const fontsize,
	 std::string const & unwrapped_text, int32_t const max_width)
{
	return
		word_wrap_text
			(*m_font_loader->get_font(fontname, fontsize),
			 unwrapped_text,
			 max_width);
}

//removes a leading spacer
//Method taken from Wesnoth.
//http://www.wesnoth.org
std::string Font_Handler::remove_first_space(const std::string &text) {
	if (text.length() > 0 && text[0] == ' ')
		return text.substr(1);
	return text;
}

//calculates size of a given text
void Font_Handler::get_size
	(std::string const & fontname, int32_t const fontsize,
	 std::string text,
	 int32_t * const w, int32_t * const h, int32_t const wrap)
{
	TTF_Font & font = *m_font_loader->get_font(fontname, fontsize);

	if (wrap > 0)
		text = word_wrap_text(font, text, wrap);

	*w = 0;
	*h = 0;
	const std::vector<std::string> lines(split_string(text, "\n"));
	container_iterate_const(std::vector<std::string>, lines, i) {
		std::string const line(i.current->empty() ? " " : *i.current);

		int32_t line_w, line_h;
		TTF_SizeUTF8(&font, line.c_str(), &line_w, &line_h);

		if (*w < line_w)
			*w = line_w;
		*h += line_h;
	}
}

//calcultes linewidth of a given text
int32_t Font_Handler::calc_linewidth(TTF_Font & font, std::string const & text)
{
	int32_t w, h;
	TTF_SizeText(&font, text.c_str(), &w, &h);
	return w;
}

/**
 * Registers the variable callback which is used (currently, 11.05)
 * for rendering map variables.
 */
void Font_Handler::register_variable_callback
	(Varibale_Callback cb, void * const cbdata)
{
	m_varcallback = cb;
	m_cbdata      = cbdata;
}
void Font_Handler::unregister_variable_callback() {
	m_varcallback = 0;
	m_cbdata = 0;
}
