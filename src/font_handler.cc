/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include <algorithm>
#include <iostream>
#include <SDL_ttf.h>
#include "error.h"
#include "filesystem.h"
#include "font_handler.h"
#include "font_loader.h"
#include "graphic.h"
#include "rendertarget.h"
#include "util.h"
#include "wexception.h"


/*
 * Plain Constructor
 */
Font_Handler::Font_Handler(void) {
   if(TTF_Init()==-1) throw wexception("True Type library did not initialize: %s\n", TTF_GetError());
   m_font_loader = new Font_Loader();
}

/*
 * Plain Destructor
 */
Font_Handler::~Font_Handler(void) {
   delete m_font_loader;
   TTF_Quit();
}


/*
 * Returns the height of the font, in pixels.
*/
int Font_Handler::get_fontheight(std::string name, int size) {
   TTF_Font* f = m_font_loader->get_font(name,size);
   return TTF_FontHeight(f);;
}

/*
 * Draw this string, if it is not cached, create the cache for it.
 * 
 * The whole text block is rendered in one Surface, this surface is cached
 * for reuse.
 * This is a really fast approach for static texts, but for text areas which keep changing
 * (like Multiline editboxes or chat windows, debug windows ... ) this is the death, for a whole new
 * surface is rendered with everything that has been written so far.
 *
 * TODO: To solve this, these should use a linecache and state explicitly that they use one. This needs to be implemented 
 * for Multiline_Textarea, when the scrollmode is changed (for this is, when the area changes a lot) 
 * and for Multiline_Editbox.
 */
// TODO: rename this to draw text 
void Font_Handler::draw_string(RenderTarget* dst, std::string font, int size, RGBColor fg, RGBColor bg, int dstx, int dsty,
		std::string text, Align align, int wrap, Widget_Cache widget_cache, uint *widget_cache_id) {
	TTF_Font* f = m_font_loader->get_font(font,size);
	//Fontrender takes care of caching
	if (widget_cache == Widget_Cache_None) {
		// look if text is cached
		_Cache_Infos  ci = {
			0,
			text,
			f,
			fg,
			bg, 
			0,
			0,
		};
		
		std::list<_Cache_Infos>::iterator i=find(m_cache.begin(), m_cache.end(), ci);
			
		if (i!=m_cache.end())  {
			// Ok, it is cached, blit it and done
			do_blit(dst,i->surface_id,dstx,dsty,align,i->w,i->h);
			if (i!=m_cache.begin()) {
				m_cache.push_front (*i);
				m_cache.erase (i);
			}
			return;
		}
		
		// Not cached, we need to create this string
      if( wrap > 0 ) {
         // Assume Multiline 
         ci.surface_id = create_static_long_text_surface(dst, f, fg, bg, text, align, wrap);
      } else {
         // Singleline
         ci.surface_id = create_single_line_text_surface(dst, f, fg, bg, text, align);
      }

      // Now cache it
      g_gr->get_picture_size( ci.surface_id, &ci.w, &ci.h);
      ci.f = f;
      m_cache.push_front (ci);

      while( m_cache.size() > CACHE_ARRAY_SIZE) {
         g_gr->free_surface(m_cache.back().surface_id);
         m_cache.pop_back();
      }
 
      // Finally, blit it
      do_blit(dst,ci.surface_id,dstx,dsty,align,ci.w,ci.h);
	}
	//Widget gave us an explicit picid
	else if (widget_cache == Widget_Cache_Use) {
		int w,h;
		g_gr->get_picture_size(*widget_cache_id,&w,&h);
		do_blit(dst,*widget_cache_id,dstx,dsty,align,w,h);
		return;
	}
	//We need to (re)create the picid for the widget
	else {
		uint picid;
		int w,h;
		if (widget_cache == Widget_Cache_Update)
			g_gr->free_surface(*widget_cache_id);
		
		*widget_cache_id = (wrap > 0	? create_static_long_text_surface(dst, f, fg, bg, text, align, wrap)
												: create_single_line_text_surface(dst, f, fg, bg, text, align));
		g_gr->get_picture_size(*widget_cache_id,&w,&h);
      do_blit(dst,*widget_cache_id,dstx,dsty,align,w,h);
	}
}
         
/*
 * This function renders a short (single line) text surface
 */
uint Font_Handler::create_single_line_text_surface( RenderTarget* dst, TTF_Font* f, RGBColor fg, RGBColor bg,
      std::string text, Align align) {
   // render this block in a SDL Surface
   SDL_Color sdl_fg = { fg.r(), fg.g(), fg.b(),0 };
   SDL_Color sdl_bg = { bg.r(), bg.g(), bg.b(),0 };

   SDL_Surface *surface;

   if( !text.size() ) 
      text = " ";

   if (!(surface = TTF_RenderText_Shaded(f, text.c_str(), sdl_fg, sdl_bg))) {
      log("Font_Handler::create_single_line_text_surface, an error : %s\n", TTF_GetError());
      log("Text was: %s\n", text.c_str());
      return 0; // This will skip this line hopefully
   }

   return convert_sdl_surface( surface );
}

/*
 * This function renders a longer (multiline) text passage, which should not change. 
 * If it changes, this function is highly unperformant.
 *
 * This function also completly ignores vertical alignement
 */ 
uint Font_Handler::create_static_long_text_surface( RenderTarget* dst, TTF_Font* f, RGBColor fg, RGBColor bg, 
            std::string text, Align align, int wrap) {
   assert( wrap > 0); 
   assert( text.size() > 0 );
   
   int global_surface_width  = wrap > 0 ? wrap : 0;
   int global_surface_height = 0;
   std::vector<SDL_Surface*> m_rendered_lines;
	std::vector<std::string> lines;
   
   // TODO: remove format informations, before wrap
   text = word_wrap_text(f,text,wrap);
	split_string(text, &lines, "\n");
 
   for(std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); it++) {
		std::string line = *it;
		if (line.empty())
			line = " ";
    
      // TODO: break the block up into 
      // format specifications

      // render this block in a SDL Surface
      SDL_Color sdl_fg = { fg.r(), fg.g(), fg.b(),0 };
      SDL_Color sdl_bg = { bg.r(), bg.g(), bg.b(),0 };

      SDL_Surface *surface;

      if (!(surface = TTF_RenderText_Shaded(f, line.c_str(),sdl_fg,sdl_bg))) {
         log("Font_Handler::create_static_long_text_surface, an error : %s\n", TTF_GetError());
         log("Text was: %s\n", text.c_str());
         continue; // Ignore this line
      }

      // TODO: join the blocks together in one line
      
      // line is finished, save it
      m_rendered_lines.push_back(surface);
      global_surface_height += TTF_FontLineSkip( f );
      if( global_surface_width < surface->w) 
         global_surface_width = surface->w;
	}

   // blit all this together in one Surface
   SDL_Surface* global_surface = SDL_CreateRGBSurface( SDL_SWSURFACE, global_surface_width, global_surface_height,
         16, 
         m_rendered_lines[0]->format->Rmask,
         m_rendered_lines[0]->format->Gmask,
         m_rendered_lines[0]->format->Bmask,
         m_rendered_lines[0]->format->Amask);

   assert( global_surface );
   
   int x = 0, y = 0;
   for( uint i = 0; i < m_rendered_lines.size(); i++) {
      SDL_Surface* s = m_rendered_lines[i];
      
      // TODO: Alignement
      SDL_Rect r;
      r.x = x; r.y = y;
      SDL_BlitSurface(s, 0, global_surface, &r);
      y += s->h;
      SDL_FreeSurface( s );
   }
 
   return convert_sdl_surface( global_surface );
}

/* 
 * Converts a SDLSurface in a widelands one
 */
uint Font_Handler::convert_sdl_surface( SDL_Surface* surface ) {
   int w = surface->w;
   int h = surface->h;
   ushort *data = (ushort*)malloc(sizeof(ushort)*w*h);
	ushort* real_data=data;

	for(int y=0; y<h; y++) {
		for(int x=0; x<w; x++) {
			uchar* real_pixel= ((uchar*)surface->pixels) + surface->pitch*y +  (x*surface->format->BytesPerPixel);

			uchar r, g, b;
			ulong pixel=0;
			switch(surface->format->BytesPerPixel) {
				case 1: pixel=*((uchar*)real_pixel);    break;
				case 2: pixel=*((ushort*)real_pixel);    break;
				case 4: pixel=*((ulong*)real_pixel);    break;
			}

			SDL_GetRGB(pixel, surface->format, &r, &g, &b);
			*data=pack_rgb(r,g,b);
			data++;
		}
	}
	uchar r,g,b;
	unpack_rgb(*real_data, &r, &g, &b);
	RGBColor clrkey(r,g,b);
	uint picid = g_gr->get_picture(PicMod_Font, w, h, real_data, clrkey);
	free(real_data);
	SDL_FreeSurface(surface);
	
   return picid;
}

/* 
 * Flushes the cached picture ids
 */
void Font_Handler::flush_cache( void ) {
	while (!m_cache.empty()) {
		g_gr->free_surface (m_cache.front().surface_id);
		m_cache.pop_front();
	}
}

std::string Font_Handler::word_wrap_text(TTF_Font* f, const std::string &unwrapped_text, int max_width) {
	//std::cerr << "Wrapping word " << unwrapped_text << "\n";
	
	std::string wrapped_text; // the final result

	size_t word_start_pos = 0;
	std::string cur_word; // including start-whitespace
	std::string cur_line; // the whole line so far

	for(size_t c = 0; c < unwrapped_text.length(); c++) {
		// Find the next word
		bool forced_line_break = false;
		if (c == unwrapped_text.length() - 1) {
			cur_word = unwrapped_text.substr(word_start_pos, c + 1 - word_start_pos);
			word_start_pos = c + 1;
		} 
		else if (unwrapped_text[c] == '\n') {
			cur_word = unwrapped_text.substr(word_start_pos, c + 1 - word_start_pos);
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
		if (calc_linewidth(f,tmp_str) > max_width) {
			if (calc_linewidth(f,cur_word) > (max_width /*/ 2*/)) {
				// The last word is too big to fit in a nice way, split it on a char basis
				//std::vector<std::string> split_word = split_utf8_string(cur_word);
				for (uint i=0;i<cur_word.length();i++) {
					tmp_str = cur_line + cur_word[i];
					if (calc_linewidth(f, tmp_str) > max_width) {
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
		wrapped_text += cur_line + '\n';
	}
	return wrapped_text;
}
	
std::string Font_Handler::remove_first_space(const std::string &text) {
	if (text.length() > 0 && text[0] == ' ')
		return text.substr(1);
	return text;
}

void Font_Handler::get_size(std::string font, int size, std::string text, int *w, int *h, int wrap) {
	TTF_Font* f = m_font_loader->get_font(font,size);
	
	if (wrap > 0)
		text = word_wrap_text(f,text,wrap);
	std::vector<std::string> lines;
	split_string(text, &lines, "\n");
		
	*w = 0;
	*h = 0;
	for(std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); it++) {
		std::string line = *it;
		if (line.empty())
			line = " ";

		int line_w,line_h;
		TTF_SizeText(f, line.c_str(), &line_w, &line_h);
		
		if (*w < line_w)
			*w = line_w;
		*h+=line_h;
	}
}

int Font_Handler::calc_linewidth(TTF_Font* f, std::string &text) {
	int w,h;
	TTF_SizeText(f, text.c_str(), &w, &h);
	return w;
}

void Font_Handler::do_blit(RenderTarget *dst, uint picid, int dstx, int dsty, Align align, int w, int h) {
	//Vertical Align
	int y = dsty;
	if (align & (Align_VCenter|Align_Bottom)) {
		if (align & Align_VCenter)
			y -= (h+1)/2; // +1 for slight bias to top
		else
			y -= h;
	}
	
	//Horizontal Align
	int x = dstx;
	if ((align & Align_Horizontal) != Align_Left) {
		if (align & Align_HCenter)
			x -= w/2;
		else if (align & Align_Right)
			x -= w;
	}
	dst->blit(x, y, picid);
}
