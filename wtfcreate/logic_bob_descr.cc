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

#include "logic_bob_descr.h"
#include "../src/graphic.h"
#include "wtfcreate.h"
#include "need_list.h"

Fabric<Logic_Bob_Descr> bobf;

//
// Logic Bob Descr, base class for ALLL those bobs
// 
Logic_Bob_Descr::Logic_Bob_Descr(const char* gname) {
   zmem(name, sizeof(name));

   memcpy(name, gname, strlen(gname) < (sizeof(name)-1) ? strlen(gname) : sizeof(name)-1);
   to_lower(name);
   w=0;
   h=0;
   hsx=0;
   hsy=0;
}
Logic_Bob_Descr::~Logic_Bob_Descr(void) {
}
int Logic_Bob_Descr::construct(Profile* p, Section* s) {

   //   cerr << "Parsing Logic_Bob_Descr!" << endl;

   // Parse bob common information
   hsx=s->get_int("hot_spot_x", 0);
   if(!hsx) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"hot_spot_x");
      return KEY_MISSING;
   }
   hsy=s->get_int("hot_spot_y", 0);
   if(!hsy) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"hot_spot_y");
      return KEY_MISSING;
   }

   // parse bob
   uint retval;
   retval=create_bob(p, s, "_??.bmp", "anim", &bob_anim);
   if(retval) {
      return retval;
   }


   return OK;
}
int Logic_Bob_Descr::write(Binary_file* f) {
   // write name
   f->write(name, 30);

   // write dimension
   f->write(&w, sizeof(ushort));
   f->write(&h, sizeof(ushort));

   // write hot spot 
   f->write(&hsx, sizeof(ushort));
   f->write(&hsy, sizeof(ushort));

   // write animation
   bob_anim.write(f);
               
   return OK;
}

int Logic_Bob_Descr::create_bob(Profile* p, Section* s, const char* def_suffix, const char* key_name, Bob_Descr* bob) {
   const char* str;

   // get name of bob_idle file name
   str=s->get_string(key_name, 0);
   char* buf;
   if(!str) {
      buf = new char[strlen(get_name())+strlen(def_suffix)+1];
      strcpy(buf, get_name());
      strcat(buf, def_suffix);
   } else {
      buf= new char[strlen(str)+1];
      strcpy(buf, str);
   }

   // get colors
   uchar r, g, b;
   ushort clrkey, shadowclr;
   r=s->get_int("clrkey_r", 255); 
   g=s->get_int("clrkey_g", 255);
   b=s->get_int("clrkey_b", 255);
   clrkey=pack_rgb(r, g, b);
   r=s->get_int("shadowclr_r", 0);
   g=s->get_int("shadowclr_g", 0);
   b=s->get_int("shadowclr_b", 0);
   shadowclr=pack_rgb(r, g, b);

   char *subdir=new char[strlen(get_name())+strlen(BOBS_DIR)+2];
   strcpy(subdir, BOBS_DIR);
   strcat(subdir, get_name());
   strcat(subdir, SSEPERATOR);

   //   cerr << g_dirname << subdir << buf << endl;

   uint retval=bob->construct(buf, g_dirname, subdir, clrkey, shadowclr, &w, &h, 0); 
   if(retval) {
      switch (retval) {
         case Bob_Descr::ERROR:
         case Bob_Descr::ERR_INVAL_FILE_NAMES:
         case Bob_Descr::ERR_INVAL_DIMENSIONS:
         case Bob_Descr::ERR_NOPICS:
            strcpy(err_sec,s->get_name());
            strcpy(err_key,key_name);
            strcpy(err_msg, subdir);
            strcat(err_msg, buf);
            strcat(err_msg,": Some bob error. check if all got the same dimensions and if the picture names are valid!");
            delete[] buf;
            delete[] subdir;
            return ERROR;  
            break;
      }
   }

   if((hsx >= w) || (hsy >= h)) {
      return Bob_Descr::ERR_INVAL_HOT_SPOT;
   }

   return OK;  
}
//
// Growings
// 
Growing_Bob_Descr::Growing_Bob_Descr(const char* gname) : Logic_Bob_Descr(gname) {
   zmem(ends_in, sizeof(ends_in));
   growing_speed=0;
   needl.add_provide(get_name(), Tribe_File_Need_List::IS_GROWING_BOB);
}
Growing_Bob_Descr::~Growing_Bob_Descr(void) {
}
int Growing_Bob_Descr::construct(Profile* p, Section* s) {
   int retval;

   //   cerr << "Parsing Growing_Bob_Descr!" << endl;

   retval=Logic_Bob_Descr::construct(p,s);
   if(retval) return retval;

   const char* str;
   str=s->get_string("ends_in", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"ends_in");
      return KEY_MISSING;
   }
   memcpy(ends_in, str, strlen(str) < (sizeof(ends_in)-1) ? strlen(str) : sizeof(ends_in)-1);
   to_lower(ends_in);
   needl.add_need(ends_in, Tribe_File_Need_List::IS_DIMINISHING_BOB);


   growing_speed=s->get_int("growing_speed", 0);
   if(!growing_speed) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"growing_speed");
      return KEY_MISSING;
   }

   return OK;
}
int Growing_Bob_Descr::write(Binary_file* f) {
//   cerr << "Growing_Bob_Descr::write()!" << endl;

   // write type
   uchar type=BOB_GROWING;
   f->write(&type, sizeof(uchar));
   
   // write bob common informations
   Logic_Bob_Descr::write(f);
  
   // write ours 
   f->write(&growing_speed, sizeof(ushort));
   f->write(ends_in, 30);

    return OK;
}

// 
// Diminishing
// 
Diminishing_Bob_Descr::Diminishing_Bob_Descr(const char* gname) : Logic_Bob_Descr(gname) {
   zmem(ends_in, sizeof(ends_in));
   stock=0;
   needl.add_provide(get_name(), Tribe_File_Need_List::IS_DIMINISHING_BOB);
}
Diminishing_Bob_Descr::~Diminishing_Bob_Descr(void) {
}
int Diminishing_Bob_Descr::construct(Profile* p, Section* s) {
   int retval;

   // cerr << "Parsing Diminishing_Bob_Descr!" << endl;

   retval=Logic_Bob_Descr::construct(p,s);
   if(retval) return retval;


   const char* str;
   str=s->get_string("ends_in", 0);
   if(str) {
      memcpy(ends_in, str, strlen(str) < (sizeof(ends_in)-1) ? strlen(str) : sizeof(ends_in)-1);
      to_lower(ends_in);
      needl.add_need(ends_in, Tribe_File_Need_List::IS_SOME_BOB);
   }

   stock=s->get_int("stock", 0);
   if(!stock) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"stock");
      return KEY_MISSING;
   }


   return OK;
}
int Diminishing_Bob_Descr::write(Binary_file* f) {

  // cerr << "Diminishing_Bob_Descr::write()!" << endl;
  
   // write type
   uchar type=BOB_DIMINISHING;
   f->write(&type, sizeof(uchar));
   
   // write bob common informations
   Logic_Bob_Descr::write(f);
  
   // write ours 
   f->write(&stock, sizeof(ushort));
   f->write(ends_in, 30);

   return OK;
}


