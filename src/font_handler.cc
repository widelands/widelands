/*
 * Copyright (C) 2002-5 by the Widelands Development Team
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

#include <SDL_image.h>
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
#include "text_parser.h"

// GRAPHIC_TODO: remove this include
#include "graphic_impl.h"

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
 */
// TODO: rename this to draw text 
void Font_Handler::draw_string(RenderTarget* dst, std::string font, int size, RGBColor fg, RGBColor bg, int dstx, int dsty,
      std::string text, Align align, int wrap, Widget_Cache widget_cache, uint *widget_cache_id, int caret) {
   TTF_Font* f = m_font_loader->get_font(font,size);
   //Width and height of text, needed for alignment
   int w,h;
   uint picid;
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
         picid = i->surface_id;
         w = i->w;
         h = i->h;
         if (i!=m_cache.begin()) {
            m_cache.push_front (*i);
            m_cache.erase (i);
         }
      }
      else {
         //not cached, create a new surface
         ci.surface_id = create_text_surface(f, fg, bg, text, align, wrap);
         // Now cache it
         g_gr->get_picture_size( ci.surface_id, &ci.w, &ci.h);
         ci.f = f;
         m_cache.push_front (ci);
   
         while( m_cache.size() > CACHE_ARRAY_SIZE) {
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
      g_gr->get_picture_size(*widget_cache_id,&w,&h);
      picid = *widget_cache_id;
   }
   //We need to (re)create the picid for the widget
   else {
      if (widget_cache == Widget_Cache_Update)
      	g_gr->free_surface(*widget_cache_id);
      *widget_cache_id = create_text_surface(f, fg, bg, text, align, wrap,caret);
      g_gr->get_picture_size(*widget_cache_id,&w,&h);
      picid = *widget_cache_id;
   }
   do_align(align,&dstx,&dsty,w,h);
   dst->blit(dstx, dsty, picid);
}

/*
* Creates a Widelands surface of the given text, checks if multiline or not
*/
uint Font_Handler::create_text_surface(TTF_Font* f, RGBColor fg, RGBColor bg, 
            std::string text, Align align, int wrap, int caret) {
   SDL_Surface *surface = (wrap > 0 ? create_static_long_text_surface(f, fg, bg, text, align, wrap, 0, caret)
                                    : create_single_line_text_surface(f, fg, bg, text, align, caret));
   return convert_sdl_surface(surface);
}

/*
 * This function renders a short (single line) text surface
 */
SDL_Surface* Font_Handler::create_single_line_text_surface(TTF_Font* f, RGBColor fg, RGBColor bg,
      std::string text, Align align, int caret) {
   // render this block in a SDL Surface
   SDL_Color sdl_fg = { fg.r(), fg.g(), fg.b(),0 };
   SDL_Color sdl_bg = { bg.r(), bg.g(), bg.b(),0 };

   SDL_Surface *surface;

   if( !text.size() ) 
      text = " ";

   if (!(surface = TTF_RenderUTF8_Shaded(f, text.c_str(), sdl_fg, sdl_bg))) {
      log("Font_Handler::create_single_line_text_surface, an error : %s\n", TTF_GetError());
      log("Text was: %s\n", text.c_str());
      return 0; // This will skip this line hopefully
   }

   if (caret != -1) {
		std::string text_caret_pos = text.substr(0,caret);
		render_caret(f,surface,text_caret_pos);
	}

   return surface;
}

/*
 * This function renders a longer (multiline) text passage, which should not change. 
 * If it changes, this function is highly unperformant.
 *
 * This function also completly ignores vertical alignement
 * Horizontal alignment is now recognized correctly
 */ 
SDL_Surface* Font_Handler::create_static_long_text_surface(TTF_Font* f, RGBColor fg, RGBColor bg, 
            std::string text, Align align, int wrap, int line_spacing, int caret) {
   assert( wrap > 0); 
   assert( text.size() > 0 );
   
   int global_surface_width  = wrap > 0 ? wrap : 0;
   int global_surface_height = 0;
   std::vector<SDL_Surface*> m_rendered_lines;
   std::vector<std::string> lines;
   
   text = word_wrap_text(f,text,wrap);
   split_string(text, &lines, "\n");
   
   SDL_Color sdl_fg = { fg.r(), fg.g(), fg.b(),0 };
   SDL_Color sdl_bg = { bg.r(), bg.g(), bg.b(),0 };
   
   uint cur_text_pos = 0;
   uint i = 0;
   
   for(std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); it++) {
      std::string line = *it;
      if (line.empty())
         line = " ";

      // render this block in a SDL Surface
      SDL_Surface *surface;

      if (!(surface = TTF_RenderUTF8_Shaded(f, line.c_str(),sdl_fg,sdl_bg))) {
         log("Font_Handler::create_static_long_text_surface, an error : %s\n", TTF_GetError());
         log("Text was: %s\n", text.c_str());
         continue; // Ignore this line
      }
      
      uint new_text_pos = cur_text_pos + line.size();
      if (caret != -1) {
			if (new_text_pos >= caret - i) {
				int caret_line_pos = caret - cur_text_pos - i;
				std::string text_caret_pos = line.substr(0,caret_line_pos);
				render_caret(f,surface,text_caret_pos);
				caret = -1;
			}
			else {
				cur_text_pos = new_text_pos;
			}
			i++;
		}
		
      m_rendered_lines.push_back(surface);
      global_surface_height += surface->h + line_spacing;
      if( global_surface_width < surface->w) 
         global_surface_width = surface->w;
   }
   
   // blit all this together in one Surface
   return join_sdl_surfaces( global_surface_width, global_surface_height, m_rendered_lines, align, line_spacing);

}

void Font_Handler::render_caret(TTF_Font *f, SDL_Surface *line, const std::string &text_caret_pos) {
	int caret_x,caret_y;
	
	TTF_SizeUTF8(f, text_caret_pos.c_str(), &caret_x, &caret_y);
	
	Surface* caret_surf = ((GraphicImpl*)(g_gr))->get_picture_surface( g_gr->get_picture( PicMod_Game, "pics/caret.png" ));
	SDL_Surface* caret_surf_sdl = caret_surf->m_surface;
	
	SDL_Rect r;
	r.x = caret_x - caret_surf_sdl->w;
	r.y = (caret_y - caret_surf_sdl->h) / 2;
	
	SDL_BlitSurface(caret_surf_sdl, 0, line, &r);
}

/*
* Renders a string into a SDL surface
* Richtext works with this method, because whole richtext content 
* is blit into one big surface by the richtext widget itself
*/
SDL_Surface* Font_Handler::draw_string_sdl_surface(std::string font, int size, RGBColor fg, RGBColor bg, std::string text, Align align, int wrap, int style, int line_spacing) {
   TTF_Font* f = m_font_loader->get_font(font,size);
   TTF_SetFontStyle(f,style);
   return create_sdl_text_surface(f,fg,bg,text,align,wrap,line_spacing);
}

/* 
* Creates the SDL surface, checks if multiline or not
*/
SDL_Surface* Font_Handler::create_sdl_text_surface(TTF_Font* f, RGBColor fg, RGBColor bg, 
            std::string text, Align align, int wrap, int line_spacing) {
   return (wrap > 0  ? create_static_long_text_surface(f, fg, bg, text, align, wrap, line_spacing)
                     : create_single_line_text_surface(f, fg, bg, text, align));
}

//draws richtext, specified by blocks
void Font_Handler::draw_richtext(RenderTarget* dst, RGBColor bg,int dstx, int dsty, std::string text, int wrap, Widget_Cache widget_cache, uint *widget_cache_id) {
   uint picid;
   int w,h;
   //Widget gave us an explicit 
   if (widget_cache == Widget_Cache_Use) {
      g_gr->get_picture_size(*widget_cache_id,&w,&h);
      picid = *widget_cache_id;
   }
   //We need to (re)create the picid for the widget
   else {
      if (widget_cache == Widget_Cache_Update)
         g_gr->free_surface(*widget_cache_id);
   
      std::vector<Text_Block> blocks;
      Text_Parser p;
      p.parse(text,&blocks);
      
      int global_height = 0;
      int h_space = 4;
      
      std::vector<SDL_Surface*> rendered_blocks;
      
      for(std::vector<Text_Block>::iterator cur = blocks.begin(); cur != blocks.end(); cur++) { 
         int start_x = 0;
         int max_x = wrap;
         int surf_h = 0;
         
         bool got_text = false;
         bool got_img = false;
        
         Surface* image = 0;
         SDL_Surface *text = 0;
         SDL_Rect img_pos,text_pos;
            
         if (cur->image.size()) {
            img_pos.x = start_x;
            img_pos.y = 0;
           
            image = ((GraphicImpl*)(g_gr))->get_picture_surface( g_gr->get_picture( PicMod_Game, cur->image.c_str() )); // Not Font, but game. 
            
            if (cur->image_align == Align_Right) {
               img_pos.x = max_x - image->get_w();
               max_x-=(image->get_h() + h_space);
            }
            else if (cur->image_align == Align_HCenter) {
               img_pos.x = (max_x - image->get_w()) / 2;
            	start_x+= img_pos.x + image->get_w() + h_space;
            }
            else 
               start_x+=image->get_w() + h_space;
            surf_h = image->get_h();
            got_img = true;
         }
         if (cur->text.size()) {
            text_pos.y = 0;
            text_pos.x = start_x;
            int font_style = TTF_STYLE_NORMAL;
            if (cur->font_weight == "bold")
               font_style |= TTF_STYLE_BOLD;
            if (cur->font_style == "italic")
               font_style |= TTF_STYLE_ITALIC;
            if (cur->font_decoration == "underline")
               font_style |= TTF_STYLE_UNDERLINE;
            text = draw_string_sdl_surface(cur->font_face,cur->font_size,cur->font_color,bg,cur->text,cur->text_align,(max_x-start_x),font_style,cur->line_spacing);
            if (text->h > surf_h)
               surf_h = text->h;
            got_text = true;
         }
         // blit all this together in one Surface
         SDL_Surface *block_surface = create_empty_sdl_surface(wrap,surf_h,text);
         if (got_text) {
            SDL_BlitSurface(text,0,block_surface,&text_pos);
            SDL_FreeSurface(text);
         }
         //blit image after text, so it can possibly cover an empty line
         if (got_img) {
            //log("img pos x: %i y: %i global_width: %i\n",img_pos.x,img_pos.y,max_x);
            SDL_Surface* sdlimage = image->m_surface;
            SDL_BlitSurface(sdlimage,0,block_surface,&img_pos);
         }
         if (!got_img && !got_text)
            throw wexception("Got empty block in draw_richtext!");
         rendered_blocks.push_back(block_surface);
         global_height+=surf_h;
      }
      SDL_Surface* global_surface = join_sdl_surfaces(wrap, global_height, rendered_blocks);
      picid = convert_sdl_surface(global_surface);
      *widget_cache_id = picid;
   }  
   dst->blit(dstx, dsty, picid);
}

//gets size of picid
void Font_Handler::get_size_from_cache(uint widget_cache_id, int *w, int *h) {
   g_gr->get_picture_size(widget_cache_id,w,h);
}

//creates an empty sdl surface of given size
SDL_Surface* Font_Handler::create_empty_sdl_surface(uint w, uint h, SDL_Surface *mask) {
   SDL_Surface* s = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h,
            16, 
            mask->format->Rmask,
            mask->format->Gmask,
            mask->format->Bmask,
            mask->format->Amask);
   return s;
}

//joins a vectror of surfaces in one big surface
SDL_Surface* Font_Handler::join_sdl_surfaces(uint w, uint h, std::vector<SDL_Surface*> surfaces, Align align, int line_spacing) {
   SDL_Surface* global_surface = create_empty_sdl_surface(w,h,surfaces[0]);
   assert(global_surface);
   
   int y = 0;
   
   for( uint i = 0; i < surfaces.size(); i++) {
      SDL_Surface* s = surfaces[i];
      SDL_Rect r;
      
      //Alignment for this line - vertical alignment is ignored
      int alignedX = 0;
      if (align & Align_HCenter)
         alignedX = (w - s->w) / 2;
      else if (align & Align_Right)
         alignedX += (w - s->w);
      
      r.x = alignedX;
      r.y = y;
      
      SDL_BlitSurface(s, 0, global_surface, &r);
      y += s->h + line_spacing;
      
      SDL_FreeSurface( s );
   }
   
   return global_surface;
}

/* 
 * Converts a SDLSurface in a widelands one
 */
uint Font_Handler::convert_sdl_surface( SDL_Surface* surface ) {
   Surface* surf = new Surface();
   SDL_SetColorKey( surface, SDL_SRCCOLORKEY, SDL_MapRGB( surface->format, 0, 0, 0 )); 
   surf->set_sdl_surface( surface );
   
   uint picid = g_gr->get_picture(PicMod_Font, surf );
   
   return picid;
}

//Sets dstx and dsty to values for a specified align
void Font_Handler::do_align(Align align, int *dstx, int *dsty, int w, int h) {
   //Vertical Align
   if (align & (Align_VCenter|Align_Bottom)) {
      if (align & Align_VCenter)
         *dsty -= (h+1)/2; // +1 for slight bias to top
      else
         *dsty -= h;
   }
   
   //Horizontal Align
   if ((align & Align_Horizontal) != Align_Left) {
      if (align & Align_HCenter)
         *dstx -= w/2;
      else if (align & Align_Right)
         *dstx -= w;
   }
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
//Deletes widget controlled surface
void Font_Handler::delete_widget_cache(uint widget_cache_id) { 
   g_gr->free_surface(widget_cache_id);
}

//Inserts linebreaks into a text, so it doesn't get bigger than max_width when rendered
//Method taken from Wesnoth.
//http://www.wesnoth.org
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
      wrapped_text += cur_line;
   }
   return wrapped_text;
}

std::string Font_Handler::word_wrap_text(std::string font, int size, const std::string &unwrapped_text,int max_width) {
	return word_wrap_text(m_font_loader->get_font(font,size),unwrapped_text,max_width);
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
      TTF_SizeUTF8(f, line.c_str(), &line_w, &line_h);
      
      if (*w < line_w)
         *w = line_w;
      *h+=line_h;
   }
}

//calcultes linewidth of a given text
int Font_Handler::calc_linewidth(TTF_Font* f, std::string &text) {
   int w,h;
   TTF_SizeUTF8(f, text.c_str(), &w, &h);
   return w;
}
