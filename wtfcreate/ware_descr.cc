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

#include <string.h>
#include "ware_descr.h"
#include "../src/helper.h"
#include "../src/graphic.h"
#include "wtfcreate.h"

Fabric<Ware_Descr> waref;

//
// class Ware_Descr
// 
Ware_Descr::Ware_Descr(const char* gname) {
   menu_pic=0;
   pic=0;
   zmem(name, sizeof(name));
   clrkey=0;
   
   memcpy(name, gname, strlen(gname) < (sizeof(name)-1) ? strlen(gname) : sizeof(name)-1);
   to_lower(name);
}

Ware_Descr::~Ware_Descr(void) {
   if(menu_pic) delete menu_pic;
   if(pic) delete pic;
}


int Ware_Descr::construct(Profile* p, Section *s) {
   // first of all, get default section
   uchar r=255;
   uchar g=255;
   uchar b=255;
   
   Section* def= p->get_section("defaults");
   if(def) {
      r=def->get_int("clrkey_r", r);
      g=def->get_int("clrkey_g", g);
      b=def->get_int("clrkey_b", b);
   }
   
   // Now, parse ware description itself
   if(s) {
      r=s->get_int("clrkey_r", r);
      g=s->get_int("clrkey_g", g);
      b=s->get_int("clrkey_b", b);
   }
   clrkey=pack_rgb(r, g, b);
   
   char* file= new char[strlen(g_dirname)+strlen(WARES_DIR)+strlen(PICS_DIR)+strlen(SSEPERATOR)+150]; // enough for filename
   // parse pic
   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, WARES_DIR);
   strcat(file, PICS_DIR);

   if(s) {
      const char* str;
      str=s->get_string("pic", 0);
      if(str) strcat(file, str);
      else {
         strcat(file, name);
         strcat(file, ".bmp");
      }
   } else {
      strcat(file, name);
      strcat(file, ".bmp");
   }
   
   pic=new Pic_Descr;
   if(pic->load(file)) {
      strcpy(err_sec, name);
      strcpy(err_key, "pic");
      strcpy(err_msg, "The picture \"");
      strcat(err_msg, file);
      strcat(err_msg, "\" could not be found!");
      return ERROR;
   }
 
   // parse menupic
   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, WARES_DIR);
   strcat(file, PICS_DIR);

   if(s) {
      const char* str;
      str=s->get_string("menupic", 0);
      if(str) strcat(file, str);
      else {
         strcat(file, name);
         strcat(file, "_m.bmp");
      }
   } else {
      strcat(file, name);
      strcat(file, "_m.bmp");
   }

   menu_pic=new Pic_Descr;
   if(menu_pic->load(file)) {
      strcpy(err_sec, name);
      strcpy(err_key, "menupic");
      strcpy(err_msg, "The picture \"");
      strcat(err_msg, file);
      strcat(err_msg, "\" could not be found!");
      return ERROR;
   }
   // check size
   if(menu_pic->get_w() != MENU_PIC_BL || 
         menu_pic->get_h() != MENU_PIC_BL) {
      strcpy(err_sec, name);
      strcpy(err_key, "menupic");
      strcpy(err_msg, "The picture \"");
      strcat(err_msg, file);
      strcat(err_msg, "\" has an invalid size for a menupic!");
      return ERROR;
   }

   return OK;
}

int Ware_Descr::write(Binary_file* f) {
  
   // write name
   f->write(name, sizeof(name));
      
   // write width and height of small pic + clrkey
   ushort temp;
   temp=pic->get_w();
   f->write(&temp, sizeof(ushort));
   temp=pic->get_h();
   f->write(&temp, sizeof(ushort));
   f->write(&clrkey, sizeof(ushort));
   
   // write menu_pic
   menu_pic->write(f);
   
   // write_small pic
   pic->write(f);

   // cerr << "Ware_Descr::write()" << endl;
   return OK;
}

