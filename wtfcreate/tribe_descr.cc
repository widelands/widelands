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

#include "../src/widelands.h"
#include "../src/graphic.h"
#include "wtfcreate.h"
#include "tribe_descr.h"
//
// Tribe Header Class
//
Tribe_Header::Tribe_Header(void) {
      zmem(name, sizeof(name));
      zmem(author, sizeof(author));
      zmem(descr, sizeof(descr));
}
Tribe_Header::~Tribe_Header(void) {
}
int Tribe_Header::construct(Profile* p, Section* s) {
  
   const char* str;
   
   str=s->get_string("name", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"name");
      return KEY_MISSING;
   }
   memcpy(name, str, strlen(str) < sizeof(name) ? strlen(str) : sizeof(name)-1);
   to_lower(name);

   str=s->get_string("author", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"author");
      return KEY_MISSING;
   }
   memcpy(author, str, strlen(str) < sizeof(author) ? strlen(str) : sizeof(author)-1);

   str=s->get_string("descr", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"descr");
      return KEY_MISSING;
   }
   memcpy(descr, str, strlen(str) < sizeof(descr) ? strlen(str) : sizeof(descr)-1);

  
   return OK;
}
int Tribe_Header::write(Binary_file* f) {
   
   f->write(name, sizeof(name));
   f->write(author, sizeof(author));
   f->write(descr, sizeof(descr));
   
   return OK;
}

//
// Regent description class
// 
Regent_Descr::Regent_Descr(void) {
   zmem(name, sizeof(name));
   small_pic=0;
   big_pic=0;
}
Regent_Descr::~Regent_Descr(void) {
   if(small_pic) delete small_pic;
   if(big_pic) delete big_pic;
}
int Regent_Descr::construct(Profile* p, Section* s) {
   char* file= new char[strlen(g_dirname)+strlen(PICS_DIR)+strlen(SSEPERATOR)+150]; // enough for filename
   const char* str;
  
   // parse clrkey
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
   r=s->get_int("clrkey_r", r);
   g=s->get_int("clrkey_g", g);
   b=s->get_int("clrkey_b", b);
   clrkey=pack_rgb(r, g, b);

 
   // parse pic
   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, PICS_DIR);
   str=s->get_string("pic_big", "reg_big.bmp");
   strcat(file, str);

   big_pic=new Pic_Descr;
   if(big_pic->load(file)) {
      strcpy(err_sec, s->get_name());
      strcpy(err_key, "pic_big");
      strcpy(err_msg, "The picture \"");
      strcat(err_msg, file);
      strcat(err_msg, "\" could not be found!");
      delete[] file;
      return ERROR;
   }
   // check size
   if(big_pic->get_w() != REGENT_PB_W || 
         big_pic->get_h() != REGENT_PB_H) {
      strcpy(err_sec, s->get_name());
      strcpy(err_key, "pic_big");
      strcpy(err_msg, "The picture \"");
      strcat(err_msg, file);
      strcat(err_msg, "\" has an invalid size for a big regent pic!!");
      delete[] file;
      return ERROR;
   }
      
   // parse pic
   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, PICS_DIR);
   str=s->get_string("pic_small", "reg_small.bmp");
   strcat(file, str);

   small_pic=new Pic_Descr;
   if(small_pic->load(file)) {
      strcpy(err_sec, s->get_name());
      strcpy(err_key, "pic_small");
      strcpy(err_msg, "The picture \"");
      strcat(err_msg, file);
      strcat(err_msg, "\" could not be found!");
      delete[] file;
      return ERROR;
   }
   // check size
   if(small_pic->get_w() != REGENT_PS_W || 
         small_pic->get_h() != REGENT_PS_H) {
      strcpy(err_sec, s->get_name());
      strcpy(err_key, "pic_small");
      strcpy(err_msg, "The picture \"");
      strcat(err_msg, file);
      strcat(err_msg, "\" has an invalid size for a small regent pic!!");
      delete[] file;
      return ERROR;
   }
    delete[] file;

    str=s->get_string("name", 0);
    if(!str) {
       strcpy(err_sec,s->get_name());
       strcpy(err_key,"name");
       return KEY_MISSING;
    }
    memcpy(name, str, strlen(str) < sizeof(name) ? strlen(str) : sizeof(name)-1);
    to_lower(name);


    return OK;
}

int Regent_Descr::write(Binary_file* f) {
   
   // output magic
   f->write("Regent\0", 7);
   f->write(name, sizeof(name));
   small_pic->write(f);
   big_pic->write(f);

   return OK;
}

