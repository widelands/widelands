/*
 * Copyright (C) 2002 by Holger Rapp
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

#include "../src/pic.h"

#define DARKEST_PL_CLR  pack_rgb(0  , 0  , 165)
#define DARK_PL_CLR     pack_rgb(0  , 55 , 190)
#define MIDDLE_PL_CLR   pack_rgb(0  , 120, 215)
#define LIGHT_PL_CLR    pack_rgb(0  , 210, 245)

#define CMD_TRANS (1<<14)
#define CMD_PLCLR (2<<14)
#define CMD_SHAD  (3<<14) 

struct BobPicDescr {
   ushort size;
   char* data;
};

struct BobDescr {
   ushort npics;
   BobPicDescr* pics;
};

struct DBobsDescr {
   char name[30];
   char ends_in[30];
   ushort hsx; // hot spot x
   ushort hsy; // hot spot y
   ushort width; 
   ushort height;
   ushort stock;
   BobDescr* bob_idle;
};

struct GBobsDescr {
   char name[30];
   char ends_in[30];
   ushort hsx;
   ushort hsy;
   ushort width;
   ushort height;
   ushort growing_speed;
   BobDescr* bob_growing;
};

/** class Bob_Data_Pic_Descr 
 *
 * This class extends class Pic and can add itself to a bob information
 */
class Bob_Data_Pic_Descr : public Pic {
   public:
      Bob_Data_Pic_Descr(void) : Pic() { has_plclr=0; has_shadow=0; shadowclr=0; }
      ~Bob_Data_Pic_Descr(void) { }

      void set_shadowclr(ushort clr); 
      void set_plclr(ushort darkest, ushort dark, ushort middle, ushort light);

      uint construct_description(BobDescr*);

   private:
      ushort shadowclr;
      ushort play_clr[4];
      bool has_shadow;
      bool has_plclr;
};

 
#endif /* __S__BOB_DESCR_H */
