/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include <string>
#include <SDL_ttf.h>
#include "font_handler.h"
#include "wexception.h"
#include "rgbcolor.h"
#include "graphic.h"
#include "error.h"
#include "filesystem.h"

Font_Handler* g_fh = 0; // the font handler

/*
===============================================================================

Font IMPLEMENTATION

===============================================================================
*/

/*
===============
Font::Font

Initialize a font and load it.
===============
*/
Font::Font(std::string name, int size, RGBColor fg, RGBColor bg)
{
	m_name = name;
   m_pointheight=size;

   m_fg=fg;
   m_bg=bg;

   m_pointheight=size;

   if (g_gr) // otherwise, reload_all() does it
		do_load();
}


/*
===============
Font::~Font

Free resources.
===============
*/
Font::~Font()
{
}

/*
===============
Font::do_load

Actually load the font from disk.
===============
*/
void Font::do_load()
{
   // Load the TrueType Font
   std::string filename="fonts/";
   filename+=m_name;
   FileRead fr;
   fr.Open(g_fs, filename.c_str());

   // Create RWOps function for SDL
   SDL_RWops* ops=SDL_RWFromMem(fr.Data(fr.GetSize()),fr.GetSize());
   TTF_Font* ttff=TTF_OpenFontRW(ops, true, get_point_height());
   if(!ttff) throw wexception("Couldn't load font!: %s\n", TTF_GetError());
   // For better readability (cause of Antialiasing)
   TTF_SetFontStyle(ttff, TTF_STYLE_BOLD);

   m_height = 0;

	// Read in the characters
	for(int i = 0; i < 96; i++) // 96
   {
//      log("Loading character %c\n", i+32);

      SDL_Color fg = { m_fg.r(), m_fg.g(), m_fg.b(),0 };
      SDL_Color bg = { m_bg.r(), m_bg.g(), m_bg.b(),0 };

      char c[2]= { i+32, '\0' };
      std::string str=c;
      SDL_Surface *surface=TTF_RenderText_Shaded(ttff, str.c_str(),fg,bg);

//      log("char '%c', surface->h: %i\n", i+32, surface->h);

      if(!m_height) m_height=surface->h;
      assert(m_height==surface->h) ;
      int w = surface->w;
      ushort *data = (ushort*)malloc(sizeof(ushort)*w*m_height);
      ushort* real_data=data;

      for(int y=0; y<m_height; y++) {
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


      m_pictures[i].width = w;
      m_pictures[i].pic = g_gr->get_picture(PicMod_UI, w, m_height, real_data, clrkey);
      SDL_FreeSurface(surface);
      free(real_data);
      }

   fr.Close();
   TTF_CloseFont(ttff);
}


