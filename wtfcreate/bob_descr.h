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

#ifndef __S__BOB_DESCR_H
#define __S__BOB_DESCR_H

#ifdef ERROR
#undef ERROR
#endif ERROR

#include "../src/pic.h"
#include "../src/myfile.h"

#define DARKEST_PL_CLR  pack_rgb(0  , 0  , 165)
#define DARK_PL_CLR     pack_rgb(0  , 55 , 190)
#define MIDDLE_PL_CLR   pack_rgb(0  , 120, 215)
#define LIGHT_PL_CLR    pack_rgb(0  , 210, 245)

#define CMD_TRANS (1<<14)
#define CMD_PLCLR (2<<14)
#define CMD_SHAD  (3<<14) 

class Bob_Data_Pic_Descr;

// ERROR is already #define'd in msvc. for now i will solve this like:
#ifdef ERROR
#undef ERROR
#endif
// this will work, but the one who wrote this file should change it.
// sft+

class Bob_Descr {
   public:
      enum {
         OK =0, 
         ERROR, 
         ERR_INVAL_FILE_NAMES,
         ERR_INVAL_DIMENSIONS,
         ERR_NOPICS,
         ERR_INVAL_HOT_SPOT
      };
      Bob_Descr(void);
      ~Bob_Descr(void);

      uint construct(const char* pfile_names, const char* dirname, const char* subdir, ushort clrkey, ushort shadowclr, ushort* w, ushort *h, bool pl_clr);

      void write(Binary_file* f);

      ushort get_npics(void) { return npics; } 

   private:
      ushort npics;
      Bob_Data_Pic_Descr** pics;
};

#endif /* __S__BOB_DESCR_H */
