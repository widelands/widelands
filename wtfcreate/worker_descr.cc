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

#define KEEP_STANDART_ASSERT 1 // no graphical assert in this file

#include "../src/widelands.h"
#include <string.h>
#include <assert.h>
#include "worker_descr.h"
#include "../src/helper.h"
#include "../src/graphic.h"
#include "../src/tribedata.h"

Worker_Fabric workerf;

// Overwritten factory function
Sit_Dig_Base_Descr* Fabric<Sit_Dig_Base_Descr>::get(const char* name) {
   uint i;

   for(i=0; i<nitems; i++) {
      if(!strcasecmp(name, items[i]->get_name())) return items[i];
   }

   nitems++;
   if(nitems==1) {
      items=(Sit_Dig_Base_Descr**) malloc(sizeof(Sit_Dig_Base_Descr*)*nitems);
   } else {
      items=(Sit_Dig_Base_Descr**) realloc(items, sizeof(Sit_Dig_Base_Descr*)*nitems);
   } 
   items[nitems-1]= new Sit_Dig_Descr(name);

   return items[nitems-1]; 
}

// 
// Worker_Fabric 
// 
Worker_Fabric::Worker_Fabric(void) {
   cur_fabric=0;
}

Worker_Fabric::~Worker_Fabric(void) {
}

Worker_Descr* Worker_Fabric::start_enum(void) {
   cur_fabric=0;
   Worker_Descr* retval=sit_digf.start_enum();
   if(retval) return retval;
   return get_nitem();
}

Worker_Descr* Worker_Fabric::get_nitem(void) {
   Worker_Descr* retval;

   retval=sit_digf.get_nitem();
   if(retval) return retval;

   if(cur_fabric==0) {
      cur_fabric=1;
      retval=searcherf.start_enum();
      if(retval) return retval;
   }
   retval=searcherf.get_nitem();
   if(retval) return retval;

   if(cur_fabric==1) {
      cur_fabric=2;
      retval=scientistf.start_enum();
      if(retval) return retval;
   }
   retval=scientistf.get_nitem();
   if(retval) return retval;

   if(cur_fabric==2) {
      cur_fabric=3;
      retval=growerf.start_enum();
      if(retval) return retval;
   }
   retval=growerf.get_nitem();
   if(retval) return retval;

   if(cur_fabric==3) {
      cur_fabric=4;
      retval=planterf.start_enum();
      if(retval) return retval;
   }
   retval=planterf.get_nitem();
   if(retval) return retval;

   return 0;
}

ushort Worker_Fabric::get_nitems(void) {
   ushort retval=scientistf.get_nitems() +
      searcherf.get_nitems() +
      planterf.get_nitems() +
      growerf.get_nitems() +
      sit_digf.get_nitems();

   return retval;
}

Scientist_Descr*  Worker_Fabric::get_scientist(const char* name) {
   if(sit_digf.exists(name)  || searcherf.exists(name) ||
         planterf.exists(name) || growerf.exists(name))  return NULL;

   return scientistf.get(name);
}

Searcher_Descr* Worker_Fabric::get_searcher(const char* name) {
   if(sit_digf.exists(name)  || scientistf.exists(name) ||
         planterf.exists(name) || growerf.exists(name))  return NULL;

   return searcherf.get(name);
}

Planter_Descr* Worker_Fabric::get_planter(const char* name) {
   if(sit_digf.exists(name)  || searcherf.exists(name) ||
         scientistf.exists(name) || growerf.exists(name))  return NULL;

   //cerr << name << endl;
   //cerr << searcherf.exists(name) << endl;

   return planterf.get(name);
}

Grower_Descr* Worker_Fabric::get_grower(const char* name) {
   if(sit_digf.exists(name)  || searcherf.exists(name) ||
         planterf.exists(name) || scientistf.exists(name))  return NULL;

   return growerf.get(name);
}

Sit_Dig_Base_Descr* Worker_Fabric::get_sit_dig(const char* name) {
   if(scientistf.exists(name)  || searcherf.exists(name) ||
         planterf.exists(name) || growerf.exists(name))  return NULL;

   return sit_digf.get(name);
}

ushort Worker_Fabric::get_index(const char* name) {
   uint i=0;

   Worker_Descr* w=start_enum();
   while(w) {
      if(!strcasecmp(w->get_name(), name)) return i;
      i++;
      w=get_nitem();
   }

   assert(0);
   return 0;
}

//
// class Worker_Descr
// 
Worker_Descr::Worker_Descr(const char* gname) {
   zmem(name, sizeof(name));

   memcpy(name, gname, strlen(gname) < (sizeof(name)-1) ? strlen(gname) : sizeof(name)-1);
   to_lower(name);

   walking_speed=0;
   hsx=0;
   hsy=0;
   is_enabled=true;
   nneeds=0; // -1 for no produce
   needs=0;
   w=0;
   h=0;
}

Worker_Descr::~Worker_Descr(void) {
   if(nneeds>0) free(needs);
}

int Worker_Descr::create_bob(Profile* p, Section* s, const char* def_suffix, const char* key_name, Bob_Descr* bob) {
   const char* str;

   Section* def=p->get_section("defaults");
   if(!def) def=s;

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
   r=s->get_int("clrkey_r", def->get_int("clrkey_r", 255));
   g=s->get_int("clrkey_g", def->get_int("clrkey_g", 255));
   b=s->get_int("clrkey_b", def->get_int("clrkey_b", 255));
   clrkey=pack_rgb(r, g, b);
   r=s->get_int("shadowclr_r", def->get_int("shadowclr_r", 0));
   g=s->get_int("shadowclr_g", def->get_int("shadowclr_g", 0));
   b=s->get_int("shadowclr_b", def->get_int("shadowclr_b", 0));
   shadowclr=pack_rgb(r, g, b);

   char *subdir=new char[strlen(get_name())+strlen(BOBS_DIR)+strlen(get_subdir())+2];
   strcpy(subdir, get_subdir());
   strcat(subdir, BOBS_DIR);
   strcat(subdir, get_name());
   strcat(subdir, SSEPERATOR);

   //   cerr << g_dirname << subdir << buf << endl;

   uint retval=bob->construct(buf, g_dirname, subdir, clrkey, shadowclr, &w, &h, 1); 
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

int Worker_Descr::construct(Profile* p, Section *s) {
   Section *def=p->get_section("defaults");
   if(!def) def=s;

   // begining with walking_speed
   walking_speed=s->get_int("walking_speed", def->get_int("walking_speed", 5));

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

   // parse is_enabled
   is_enabled=s->get_boolean("is_enabled", true);

   // parse needs
   const char* str=s->get_string("needs", 0);
   if(!str) {
      // section not listed. no needs for this
      needs=0;
      nneeds=0;
   } else if(!strcasecmp(str,"0")) {
      // this can only be build
      needs=0;
      nneeds=-1;
   } else {
      // normal needs, parse it
      int i=0;
      nneeds=1;
      for(i=0; ((uint)i)<strlen(str); i++) 
         if(str[i]==',') nneeds++;

      needs=(Ware_List*) malloc(sizeof(Ware_List)*nneeds);
      ushort n=0, z=0;
      char temp[50];
      for(i=0; i<nneeds; i++) {
         while(str[n]!='*') {
            temp[z]=str[n];
            n++; z++;
         }
         temp[z]='\0'; 
         z=0;
         needs[i].num=atoi(temp);

         n++;
         while(str[n]==' ' || str[n]=='\t') n++;
         while(str[n]!=',' && str[n]!='\n' && str[n]!=' ' && str[n]!='\t' & str[n]!='\0') {
            temp[z]=str[n];
            n++; z++;
         }
         temp[z]='\0';
         z=0;
         if((int)i!=nneeds-1) while(str[n]!=',') n++;
         n++;

         // name of ware is now in temp, get index
         needs[i].ware=waref.exists(temp);
         if(!needs[i].ware) {
            strcpy(err_sec,s->get_name());
            strcpy(err_key,"needs");
            strcpy(err_msg,"worker depends on ware \"");
            strcat(err_msg, temp);
            strcat(err_msg, "\" but no building produces this!");
            return ERROR;
         }
         z=0;
      }

      //      for(i=0; i<nneeds; i++) 
      //       cerr << get_name() << ":" << needs[i].num << "*" << needs[i].ware->get_name() << endl;
   }

   // parse walking bobs
   uint retval;
   retval=create_bob(p, s, "_walk_ne_??.bmp",  "walk_ne_anim", &bob_walk_ne);
   if(retval) return retval;
   retval=create_bob(p, s, "_walk_e_??.bmp",  "walk_e_anim", &bob_walk_e);
   if(retval) return retval;
   retval=create_bob(p, s, "_walk_se_??.bmp",  "walk_se_anim", &bob_walk_se);
   if(retval) return retval;
   retval=create_bob(p, s, "_walk_sw_??.bmp",  "walk_sw_anim", &bob_walk_sw);
   if(retval) return retval;
   retval=create_bob(p, s, "_walk_w_??.bmp",  "walk_w_anim", &bob_walk_w);
   if(retval) return retval;
   retval=create_bob(p, s, "_walk_nw_??.bmp",  "walk_nw_anim", &bob_walk_nw);
   if(retval) return retval;


   // cerr << "Parsing <" << get_name() << ">  Worker_Descr" << endl;


   return OK;
}
int Worker_Descr::write(Binary_file* f) {

   f->write(name, sizeof(name));
   uchar temp=is_enabled;
   f->write(&temp, sizeof(uchar));
   f->write(&walking_speed, sizeof(ushort));

   // write need list
   f->write(&nneeds, sizeof(short));
   int i;
   ushort temp1;
   for(i=0; i<nneeds; i++) {
      temp1=needs[i].num;
      f->write(&temp1, sizeof(ushort));
      temp1=waref.get_index(needs[i].ware->get_name());
      f->write(&temp1, sizeof(ushort));
   }

   f->write(&w, sizeof(ushort));
   f->write(&h, sizeof(ushort));
   f->write(&hsx, sizeof(ushort));
   f->write(&hsy, sizeof(ushort));

   // Write bobs
   bob_walk_ne.write(f);
   bob_walk_e.write(f);
   bob_walk_se.write(f);
   bob_walk_sw.write(f);
   bob_walk_w.write(f);
   bob_walk_nw.write(f);

   //cerr << "Worker_Descr_Descr::write()" << endl.write(f);
   return OK;
}


//
// Menu Worker_Descr
// 
Menu_Worker_Descr::Menu_Worker_Descr(const char* name) : Worker_Descr(name) {
   menu_pic=0;
   clrkey=0;
}
Menu_Worker_Descr::~Menu_Worker_Descr(void) {
   if(menu_pic) delete menu_pic;
}
int Menu_Worker_Descr::construct(Profile* p, Section *s) {

   //   cerr << "Parsing <" << get_name() << "> Menu worker description!" << endl;

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


   char* file= new char[strlen(g_dirname)+strlen(get_subdir())+strlen(BOBS_DIR)+strlen(get_name())+strlen(SSEPERATOR)+150]; // enough for filename
   // parse pic
   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, get_subdir());
   strcat(file, BOBS_DIR);
   strcat(file, get_name());
   strcat(file, SSEPERATOR);

   //   cerr << file << endl;

   const char* str;
   str=s->get_string("menupic", 0);
   if(str) strcat(file, str);
   else {
      strcat(file, get_name());
      strcat(file, "_m.bmp");
   }

   menu_pic=new Pic_Descr;
   if(menu_pic->load(file)) {
      strcpy(err_sec, get_name());
      strcpy(err_key, "menupic");
      strcpy(err_msg, "The picture \"");
      strcat(err_msg, file);
      strcat(err_msg, "\" could not be found!");
      delete[] file;
      return ERROR;
   }
   // check size
   if(menu_pic->get_w() != MENU_PIC_BL || 
         menu_pic->get_h() != MENU_PIC_BL) {
      strcpy(err_sec, get_name());
      strcpy(err_key, "menupic");
      strcpy(err_msg, "The picture \"");
      strcat(err_msg, file);
      strcat(err_msg, "\" has an invalid size for a menupic!");
      delete[] file;
      return ERROR;
   }

   delete[] file;

   return OK;
}
int Menu_Worker_Descr::write(Binary_file* f) {

   // write clrkey and pic
   f->write(&clrkey, sizeof(ushort));
   menu_pic->write(f);

   // cerr << "Menu_Worker_Descr::write()" << endl;
   return OK;
}

//
// Has_Working_Worker_Descr
// 
Has_Working_Worker_Descr::Has_Working_Worker_Descr(const char* name) : Worker_Descr(name) {
}
Has_Working_Worker_Descr::~Has_Working_Worker_Descr(void) {
}
int Has_Working_Worker_Descr::construct(Profile* p, Section *s) {

   // cerr << "Parsing <" << get_name() << "> Has_Working_Worker_Descr!" << endl;
   uint retval;
   retval=create_bob(p, s, "_work_??.bmp",  "working_anim", &bob_working);
   if(retval) return retval;

   return OK;
}
int Has_Working_Worker_Descr::write(Binary_file* f) {
      
   bob_working.write(f);

   // cerr << "Has_Working_Worker_Descr::write()" << endl;
   return OK;
}


//
// Has_Working1_Worker_Descr
// 
Has_Working1_Worker_Descr::Has_Working1_Worker_Descr(const char* name) : Worker_Descr(name) {
}
Has_Working1_Worker_Descr::~Has_Working1_Worker_Descr(void) {
}
int Has_Working1_Worker_Descr::construct(Profile* p, Section *s) {

   // cerr << "Parsing <" << get_name() << "> Has_Working1_Worker_Descr!" << endl;
   uint retval;
   retval=create_bob(p, s, "_work1_??.bmp",  "working_anim1", &bob_working1);
   if(retval) return retval;

   return OK;
}
int Has_Working1_Worker_Descr::write(Binary_file* f) {

   // cerr << "Has_Working1_Worker_Descr::write()" << endl;
      
   bob_working1.write(f);
   
   
   return OK;
}

//
// Has_Walk1_Worker_Descr
// 
Has_Walk1_Worker_Descr::Has_Walk1_Worker_Descr(const char* name) : Worker_Descr(name) {
}
Has_Walk1_Worker_Descr::~Has_Walk1_Worker_Descr(void) {
}
int Has_Walk1_Worker_Descr::construct(Profile* p, Section *s) {

   //   cerr << "Parsing <" << get_name() << "> Has_Walk1_Worker_Descr!" << endl;

   uint retval;
   retval=create_bob(p, s, "_walk_ne1_??.bmp",  "walk_ne_anim1", &bob_walk_ne1);
   if(retval) return retval;
   retval=create_bob(p, s, "_walk_e1_??.bmp",  "walk_e_anim1", &bob_walk_e1);
   if(retval) return retval;
   retval=create_bob(p, s, "_walk_se1_??.bmp",  "walk_se_anim1", &bob_walk_se1);
   if(retval) return retval;
   retval=create_bob(p, s, "_walk_sw1_??.bmp",  "walk_sw_anim1", &bob_walk_sw1);
   if(retval) return retval;
   retval=create_bob(p, s, "_walk_w1_??.bmp",  "walk_w_anim1", &bob_walk_w1);
   if(retval) return retval;
   retval=create_bob(p, s, "_walk_nw1_??.bmp",  "walk_nw_anim1", &bob_walk_nw1);
   if(retval) return retval;



   return OK;
}
int Has_Walk1_Worker_Descr::write(Binary_file* f) {
   bob_walk_ne1.write(f);
   bob_walk_e1.write(f);
   bob_walk_se1.write(f);
   bob_walk_sw1.write(f);
   bob_walk_w1.write(f);
   bob_walk_nw1.write(f);

//   cerr << "Has_Walk1_Worker_Descr::write()" << endl;
   return OK;
}




// 
// Scientist description
// 
Scientist_Descr::Scientist_Descr(const char* gname) : Worker_Descr(gname), Menu_Worker_Descr(gname)  {
}

Scientist_Descr::~Scientist_Descr(void) {
}

int Scientist_Descr::construct(Profile* p, Section* s) {
   int retval;

   //   cerr << "Parsing <" << get_name() << ">  Scientist_Descr" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;

   return OK;
}
int Scientist_Descr::write(Binary_file* f) {
   
   uchar id=SCIENTIST;
   f->write(&id, sizeof(uchar));

   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   
  // cerr << "Scientist_Descr::write()!" << endl;
   return OK;
}

// 
// Searcher description
// 
Searcher_Descr::Searcher_Descr(const char* gname) : Worker_Descr(gname), Menu_Worker_Descr(gname), 
   Has_Walk1_Worker_Descr(gname), Has_Working_Worker_Descr(gname) {
   }

Searcher_Descr::~Searcher_Descr(void) {
}

int Searcher_Descr::construct(Profile* p, Section* s) {
   int retval;

   //  cerr << "Parsing <" << get_name() << ">  Searcher_Descr" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Walk1_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working_Worker_Descr::construct(p,s);
   if(retval) return retval;

   return OK;
}
int Searcher_Descr::write(Binary_file* f) {
  
   uchar id=SEARCHER;
   f->write(&id, sizeof(uchar));

   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   Has_Walk1_Worker_Descr::write(f);
   Has_Working_Worker_Descr::write(f);

   // nothing additional 
//   cerr << "Searcher_Descr::write()" << endl;
   return OK;
}

// 
// Grower description
// 
Grower_Descr::Grower_Descr(const char* gname) : Worker_Descr(gname), Menu_Worker_Descr(gname), Has_Walk1_Worker_Descr(gname),
   Has_Working_Worker_Descr(gname), Has_Working1_Worker_Descr(gname) {
   }

Grower_Descr::~Grower_Descr(void) {
}

int Grower_Descr::construct(Profile* p, Section* s) {
   int retval;

   // cerr << "Parsing <" << get_name() << ">  Grower_Descr" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Walk1_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working1_Worker_Descr::construct(p,s);
   if(retval) return retval;

   return OK;
}
int Grower_Descr::write(Binary_file* f) {
   
   uchar id=GROWER;
   f->write(&id, sizeof(uchar));
   
   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   Has_Walk1_Worker_Descr::write(f);
   Has_Working_Worker_Descr::write(f);
   Has_Working1_Worker_Descr::write(f);

   // cerr << "Grower_Descr::write()!" << endl;
   return OK;
}

// 
// Planter description
// 
Planter_Descr::Planter_Descr(const char* gname) : Worker_Descr(gname), Menu_Worker_Descr(gname), Has_Walk1_Worker_Descr(gname),
   Has_Working_Worker_Descr(gname) {
   }

Planter_Descr::~Planter_Descr(void) {
}

int Planter_Descr::construct(Profile* p, Section* s) {
   int retval;

   //   cerr << "Parsing <" << get_name() << ">  Planter_Descr" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Walk1_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working_Worker_Descr::construct(p,s);
   if(retval) return retval;

   return OK;
}
int Planter_Descr::write(Binary_file* f) {

   uchar id=PLANTER;
   f->write(&id, sizeof(uchar));

   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   Has_Walk1_Worker_Descr::write(f);
   Has_Working_Worker_Descr::write(f);
   
   // cerr << "Planter_Descr::write()!" << endl;
   return OK;
}

// 
// Sitter and digger base description description
// 
Sit_Dig_Base_Descr::Sit_Dig_Base_Descr(const char* gname) : Worker_Descr(gname) {
}

Sit_Dig_Base_Descr::~Sit_Dig_Base_Descr(void) {
}

int Sit_Dig_Base_Descr::construct(Profile* p, Section* s) {

   //   cerr << "Parsing <" << get_name() << ">  Sit_Dig_Base_Descr" << endl;

   return OK;
}
int Sit_Dig_Base_Descr::write(Binary_file* f) {
  
   // Nothing to do
   
   //cerr << "Sit_Dig_Base_Descr::write()!" << endl;
   return OK;
}

//
// Sitter and digger description
//
Sit_Dig_Descr::Sit_Dig_Descr(const char* gname) : Worker_Descr(gname), Menu_Worker_Descr(gname), Sit_Dig_Base_Descr(gname) {
}
Sit_Dig_Descr::~Sit_Dig_Descr(void) {
}
int Sit_Dig_Descr::construct(Profile* p, Section* s) {
   int retval;

   //   cerr <<  "Parsing <" << get_name() << ">  Sit_Dig_Descr" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Sit_Dig_Base_Descr::construct(p,s);
   if(retval) return retval;

   return OK;
}
int Sit_Dig_Descr::write(Binary_file* f) {
   
   // write id
   uchar id=SITDIG;
   f->write(&id, sizeof(uchar));

   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   Sit_Dig_Base_Descr::write(f);
   
   // Nothing of our own stuff to write
//   cerr << "Sit_Dig_Descr::write()" << endl;
   
   return OK;
}

//
// Special Workers
// 

//
// Carrier_Descr
// 
Carrier_Descr::Carrier_Descr(const char* gname) : Worker_Descr(gname), Sit_Dig_Base_Descr(gname), Has_Walk1_Worker_Descr(gname), 
   Has_Working_Worker_Descr(gname), Has_Working1_Worker_Descr(gname) {
   }
Carrier_Descr::~Carrier_Descr(void) {
}
int Carrier_Descr::construct(Profile* p, Section *s) {

   //   cerr << "Parsing <" << get_name() << ">  Carrier_Descr" << endl;

   return OK;
}
int Carrier_Descr::write(Binary_file* f) {

   //cerr << "Carrier_Descr::write()" << endl;
   // nothing to do
   return OK;
}

//
// Def_Carrier_Descr
// 
Def_Carrier_Descr::Def_Carrier_Descr(const char* gname) : Worker_Descr(gname), Sit_Dig_Base_Descr(gname), Carrier_Descr(gname), 
   Has_Walk1_Worker_Descr(gname), Has_Working_Worker_Descr(gname), Has_Working1_Worker_Descr(gname) {
   }
Def_Carrier_Descr::~Def_Carrier_Descr(void) {
}
int Def_Carrier_Descr::construct(Profile* p, Section *s) {

   int retval;

   //   cerr << "Parsing <" << get_name() << ">  Def_Carrier_Descr" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Sit_Dig_Base_Descr::construct(p,s);
   if(retval) return retval;
   retval=Carrier_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Walk1_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working1_Worker_Descr::construct(p,s);
   if(retval) return retval;


   return OK;
}
int Def_Carrier_Descr::write(Binary_file* f) {
   
   uchar id=SPEC_DEF_CARRIER;
   f->write(&id, sizeof(uchar));

   Worker_Descr::write(f);
   Sit_Dig_Base_Descr::write(f);
   Carrier_Descr::write(f);
   Has_Walk1_Worker_Descr::write(f);
   Has_Working_Worker_Descr::write(f);
   Has_Working1_Worker_Descr::write(f);

   // cerr << "Def_Carrier_Descr::write()" << endl;
   return OK;
}

//
// Add_Carrier_Descr
// 
Add_Carrier_Descr::Add_Carrier_Descr(const char* gname) : Worker_Descr(gname), Menu_Worker_Descr(gname), Sit_Dig_Base_Descr(gname), 
   Carrier_Descr(gname), Has_Walk1_Worker_Descr(gname), Has_Working_Worker_Descr(gname), Has_Working1_Worker_Descr(gname) {
   }
Add_Carrier_Descr::~Add_Carrier_Descr(void) {
}
int Add_Carrier_Descr::construct(Profile* p, Section *s) {

   int retval;

   //   cerr << "Parsing <" << get_name() << ">  Add_Carrier_Descr" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Sit_Dig_Base_Descr::construct(p,s);
   if(retval) return retval;
   retval=Carrier_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Walk1_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working1_Worker_Descr::construct(p,s);
   if(retval) return retval;
   return OK;
}
int Add_Carrier_Descr::write(Binary_file* f) {
   
   uchar id=SPEC_ADD_CARRIER;
   f->write(&id, sizeof(uchar));

   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   Sit_Dig_Base_Descr::write(f);
   Carrier_Descr::write(f);
   Has_Walk1_Worker_Descr::write(f);
   Has_Working_Worker_Descr::write(f);
   Has_Working1_Worker_Descr::write(f);

   return OK;
}

//
// Builder_Descr
// 
Builder_Descr::Builder_Descr(const char* gname) : Worker_Descr(gname), Menu_Worker_Descr(gname), Sit_Dig_Base_Descr(gname),
   Has_Working_Worker_Descr(gname), Has_Working1_Worker_Descr(gname) {
   }
Builder_Descr::~Builder_Descr(void) {
}
int Builder_Descr::construct(Profile* p, Section *s) {

   int retval;

   //   cerr << "Parsing <" << get_name() << ">  Builder_Descr" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Sit_Dig_Base_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working1_Worker_Descr::construct(p,s);
   if(retval) return retval;


   return OK;
}
int Builder_Descr::write(Binary_file* f) {
   
   uchar id=SPEC_BUILDER;
   f->write(&id, sizeof(uchar));
   
   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   Sit_Dig_Base_Descr::write(f);
   Has_Working_Worker_Descr::write(f);
   Has_Working1_Worker_Descr::write(f);
   
   //cerr << "Builder_Descr::write()" << endl;
   return OK;
}

//
// Planer_Descr
// 
Planer_Descr::Planer_Descr(const char* gname) : Worker_Descr(gname), Menu_Worker_Descr(gname), Sit_Dig_Base_Descr(gname),
   Has_Working_Worker_Descr(gname) {
   }
Planer_Descr::~Planer_Descr(void) {
}

int Planer_Descr::construct(Profile* p, Section *s) {
   int retval;

   //   cerr << "Parsing <" << get_name() << ">  Planer_DescrO" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Sit_Dig_Base_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working_Worker_Descr::construct(p,s);
   if(retval) return retval;
   return OK;
}
int Planer_Descr::write(Binary_file* f) {

   uchar id=SPEC_PLANER;
   f->write(&id, sizeof(uchar));
   
   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   Sit_Dig_Base_Descr::write(f);
   Has_Working_Worker_Descr::write(f);
      
  // cerr << "Planer_Descr::write()" << endl;
   return OK;
}

//
// Explorer_Descr
// 
Explorer_Descr::Explorer_Descr(const char* gname) : Worker_Descr(gname), Menu_Worker_Descr(gname), Sit_Dig_Base_Descr(gname) {
}
Explorer_Descr::~Explorer_Descr(void) {
}
int Explorer_Descr::construct(Profile* p, Section *s) {

   int retval;

   //   cerr << "Parsing <" << get_name() << ">  Explorer_Descr" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Sit_Dig_Base_Descr::construct(p,s);
   if(retval) return retval;

   return OK;
}
int Explorer_Descr::write(Binary_file* f) {

   uchar id=SPEC_EXPLORER;
   f->write(&id, sizeof(uchar));

   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   Sit_Dig_Base_Descr::write(f);

   //cerr << "Explorer_Descr::write()!" << endl;
   return OK;
}

//
// Geologist_Descr
// 
Geologist_Descr::Geologist_Descr(const char* gname) : Worker_Descr(gname), Menu_Worker_Descr(gname), Sit_Dig_Base_Descr(gname),
   Has_Working_Worker_Descr(gname), Has_Working1_Worker_Descr(gname) {
   }
Geologist_Descr::~Geologist_Descr(void) {
}
int Geologist_Descr::construct(Profile* p, Section *s) {

   int retval;

   //   cerr << "Parsing <" << get_name() << ">  Geologist_Descr" << endl;

   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Sit_Dig_Base_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working_Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Working1_Worker_Descr::construct(p,s);
   if(retval) return retval;

   return OK;
}
int Geologist_Descr::write(Binary_file* f) {
   uchar id=SPEC_GEOLOGIST;
   f->write(&id, sizeof(uchar));

   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   Sit_Dig_Base_Descr::write(f);
   Has_Working_Worker_Descr::write(f);
   Has_Working1_Worker_Descr::write(f);
   
   // cerr << "Geologist_Descr::write()!" << endl;
   
   return OK;
}

