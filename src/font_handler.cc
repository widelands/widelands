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

#include "font_handler.h"
#include "error.h"
#include "graphic.h"
#include "rendertarget.h"
#include "wexception.h"
#include <SDL_ttf.h>

/*
 * the next two functions search for a font and through an exeption
 * or not if it is not found
 */
Font* Font_Handler::find_correct_font(std::string name, int size) {
   // Find the correct font
   Font* f=0;
   std::vector<Font*>::iterator it;
   for(it = m_fonts.begin(); it != m_fonts.end(); it++) {
      if((*it)->get_name() == name && (*it)->get_point_height()==size) {
         assert(!f);
         f=*it;
      }
   }


   if(!f) throw wexception("Font unknown: %s, size: %i\n", name.c_str(), size);
   return f;
}

Font* Font_Handler::find_correct_font(std::string name, int size, RGBColor fg, RGBColor bg) {
   // Find the correct font
   Font* f=0;
   std::vector<Font*>::iterator it;
   for(it = m_fonts.begin(); it != m_fonts.end(); it++) {
      if((*it)->get_name() == name && (*it)->get_point_height()==size &&
            (*it)->get_fg_clr().r()==fg.r() && (*it)->get_fg_clr().g()==fg.g() && (*it)->get_fg_clr().b()==fg.b() &&
            (*it)->get_bg_clr().r()==bg.r() && (*it)->get_bg_clr().g()==bg.g() && (*it)->get_bg_clr().b()==bg.b()) {
         assert(!f);
         f=*it;
      }
   }

   if(!f) throw wexception("Font unknown: %s, size: %i, fg:(%i,%i,%i), bg:(%i,%i,%i)\n", name.c_str(), size, fg.r(),fg.g(),fg.b(), bg.r(),bg.g(),bg.b());
   return f;
}


/*
 * Draw a string directly into the destination bitmap with the desired alignment.
 * The function honours line-breaks.
 * If wrap is positive, the function will wrap a line after that many pixels.
 */
void Font_Handler::draw_string(RenderTarget* dst, std::string font, int size, RGBColor fg, RGBColor bg, int dstx, int dsty, const char* string,
      Align align, int wrap, int mark_char, int mark_value)
{
   Font* f;
   f=find_correct_font(font, size);

   // Adjust for vertical alignment
   if (align & (Align_VCenter|Align_Bottom))
   {
      int h;

      get_size(font, size, string, 0, &h, wrap);

      if (align & Align_VCenter)
         dsty -= (h+1)/2; // +1 for slight bias to top
      else
         dsty -= h;
   }

   int i=0;
   // Draw the string
   while(*string)
   {
      const char* nextline;
      int x = dstx;

      if (wrap <= 0 && (align & Align_Horizontal) == Align_Left)
      {
         // straightforward path with no alignment and no wrapping
         nextline = string + strcspn(string, "\n");
      }
      else
      {
         int width = calc_linewidth(font, size, string, wrap, &nextline);

         if (align & Align_HCenter)
            x -= width/2;
         else if (align & Align_Right)
            x -= width;
      }

      while(string < nextline)
      {
         uchar c = (uchar)*string;

         if (c == ' ' || c == '\t') // whitespace
         {
            int ch=0;
            int cw=0;
            f->get_char_size(0,&cw,&ch);
            if (c == '\t')
               cw *= 8;
            if(mark_char==i) {
               dst->brighten_rect(x,dsty,cw,ch,mark_value);
            }

            x += cw;
         }
         else if (c && c != '\n')
         {
            if (c < 32 || c > 127)
               c = 127;

            c -= 32;
            int w,h;
            f->get_char_size(c,&w,&h);
            if(mark_char==i) {
               dst->brighten_rect(x,dsty,w,h,mark_value);
            }
            dst->blit(x, dsty, f->get_char_pic(c));
            x += w;
         } else if(c=='\n') {
            if(mark_char==i) {
               int ch=0;
               int cw=0;
               f->get_char_size(0,&cw,&ch);
               if(mark_char==i) {
                  dst->brighten_rect(x,dsty,cw,ch,mark_value);
               }
            }
         }

         i++;
         string++;
      }

      dsty += f->get_pixel_height();
   }
}


/*
 * Calculate the size of the given string.
 * pw and ph may be NULL.
 * If wrap is positive, the function will wrap a line after that many pixels
 */
void Font_Handler::get_size(std::string font, int size, const char* string, int* pw, int* ph, int wrap)
{
   int maxw = 0; // width of widest line
   int maxh = 0; // total height

   Font* f=find_correct_font(font, size);

   while(*string)
   {
      const char* nextline;
      int width = calc_linewidth(font, size, string, wrap, &nextline);

      if (width > maxw)
         maxw = width;
      maxh += f->get_pixel_height();

      string = nextline;
   }

   if (pw)
      *pw = maxw;
   if (ph)
      *ph = maxh;
}



/*
 * Calculates the width of the given line (up to \n or NUL).
 * If wrap is positive, the function will wrap the line after that many pixels.
 * nextline will point to the first character on the next line. That is, it
 * points to:
 *   - the character after the \n if the line is ended by a line-break
 *   - the NUL if the string ends after the line
 *   - the first visible character on the next line in case the line is wrapped
 *
 *   The function returns the width of the line, in pixels.
 */
int Font_Handler::calc_linewidth(std::string name, int size, const char* line, int wrap, const char** nextline)
{
   const char* string = line; // current pointer into the string
   int width = 0; // width of line

   Font* f=find_correct_font(name, size);

   // Loop forward
   for(;;)
   {
      if (*string == ' ' || *string == '\t') // whitespace
      {
         int cw, ch;
         f->get_char_size(0, &cw,&ch);
         if (*string == '\t')
            cw *= 8;

         string++;

         if (wrap > 0 && width+cw > wrap)
            break;
         else
            width += cw;
      }
      else if (!*string || *string == '\n') // explicit end of line
      {
         if (*string)
            string++;
         break;
      }
      else // normal word
      {
         const char* p;
         int wordwidth = 0;

         for(p = string;; p++)
         {
            if (!*p || *p == ' ' || *p == '\t' || *p == '\n') // whitespace break
               break;

            uchar c = (uchar)*p;
            if (c < 32 || c > 127)
               c = 127;

            c -= 32;
            int cw, ch;
            f->get_char_size(c, &cw,&ch);
            wordwidth+=cw;

            if (*p == '-') // other character break
            {
               p++;
               break;
            }
         }

         if (wrap > 0 && width && width+wordwidth > wrap)
            break;
         else
         {
            string = p;
            width += wordwidth;
         }
      }
   }

   // That's it
   if (nextline)
      *nextline = string;

   return width;
}


/*
 * Plain Constructor
 */
Font_Handler::Font_Handler(void) {
   if(TTF_Init()==-1) throw wexception("True Type library did not initialize: %s\n", TTF_GetError());
}

/*
 * Plain Destructor
 */
Font_Handler::~Font_Handler(void) {
   TTF_Quit();

   std::vector<Font*>::iterator it;
   for(it = m_fonts.begin(); it != m_fonts.end(); it++) {
      delete *it;
   }
   m_fonts.resize(0);
}


/*
 * Wrapper around Font()
 * the constructor of Font does the main work
 */
void Font_Handler::load_font(std::string name, int size, RGBColor fg, RGBColor bg) {
   // log("Loading: %s, %i\n", name.c_str(), size);
   try {
      Font* f=find_correct_font(name,size,fg,bg);
      if(f) return; // already loaded
   } catch(wexception& exe) {
      // not yet loaded
      Font* f=new Font(name, size, fg, bg);
      m_fonts.push_back(f);
   }
}

/*
 * Returns the height of the font, in pixels.
*/
int Font_Handler::get_fontheight(std::string name, int size)
{
   Font* f=find_correct_font(name,size);
   return f->get_pixel_height();
}



/*
 * Unload all fonts with this name and size
 * if size == 0, delete all fonts with this name
 */
void Font_Handler::unload_font(std::string name, int size) {
  std::vector<Font*>::iterator it;

	for(it = m_fonts.begin(); it != m_fonts.end(); it++) {
      if((*it)->get_name() == name) {
         if(!size || (*it)->get_point_height()==size) {
            delete *it;
            m_fonts.erase(it);
         }
      }
   }
}


/*
 * Call the do_load() function for every Font.
*/
void Font_Handler::reload_all()
{
	std::vector<Font*>::iterator it;

	for(it = m_fonts.begin(); it != m_fonts.end(); it++)
		(*it)->do_load();
}

