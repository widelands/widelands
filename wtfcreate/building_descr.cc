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
#include "building_descr.h"
#include "need_list.h"

// TEMP
#include <iostream>
// TEMP

// 
// class Building_Descr
// 
Building_Descr::Building_Descr(void) {
   zmem(name, sizeof(name));
   see_area=0;
   w=0; 
   h=0;
   hsx=0;
   hsy=0;
   is_enabled=true;
}

Building_Descr::~Building_Descr(void) {
}

int Building_Descr::create_bob(Profile* p, Section* s, const char* def_suffix, const char* key_name, Bob_Descr* bob, ushort* ew, ushort* eh) {
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

   char *subdir=new char[strlen(get_name())+strlen(BOBS_DIR)+strlen(BUILDINGS_DIR)+2];
   strcpy(subdir, BUILDINGS_DIR);
   strcat(subdir, BOBS_DIR);
   strcat(subdir, get_name());
   strcat(subdir, SSEPERATOR);

   //   cerr << g_dirname << subdir << buf << endl;

   uint retval;
   if(ew && eh) {
      retval=bob->construct(buf, g_dirname, subdir, clrkey, shadowclr, ew, eh, 1); 
   } else {
      retval=bob->construct(buf, g_dirname, subdir, clrkey, shadowclr, &w, &h, 1); 
   }
   
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
int Building_Descr::construct(Profile* p, Section *s) {
   Section* def=p->get_section("defaults");
   if(!def) def=s;

   // copy name
   memcpy(name, s->get_name(), strlen(s->get_name()) < sizeof(name) ? strlen(s->get_name()) : sizeof(name)-1);
   to_lower(name);

   // parse is enabled
   is_enabled=s->get_boolean("is_enabled", true);

   // parse see_area
   see_area=s->get_int("see_area", def->get_int("see_area", 4));

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

   // parse idle bob
   uint retval;
   retval=create_bob(p, s, "_i_??.bmp", "idle_anim", &bob_idle);
   if(retval) {
      return retval;
   }

   // cerr << name << ":" << see_area << ":" << w << ":" << h << ":" << hsx << ":" << hsy << ":" << bob_idle.get_npics() << endl; 
   return OK;
}

int Building_Descr::write(Binary_file* f) {
 
   f->write(name, sizeof(name));
   uchar temp=is_enabled;
   f->write(&temp, sizeof(uchar));
   f->write(&see_area, sizeof(ushort));
   f->write(&w, sizeof(ushort));
   f->write(&h, sizeof(ushort));
   f->write(&hsx, sizeof(ushort));
   f->write(&hsy, sizeof(ushort));
   bob_idle.write(f);

//   cerr << "Building_Descr::write()" << endl;
   return OK;
}


//
// class Boring_Building_Descr 
// 
Boring_Building_Descr::Boring_Building_Descr(void) {
}

Boring_Building_Descr::~Boring_Building_Descr(void) {
}

int Boring_Building_Descr::construct(Profile* p, Section *s) {

   // NOTHING TO DO here
   return OK;
}
int Boring_Building_Descr::write(Binary_file* f) {
   // Nothing to do
   
   // cerr << "Boring_Building_Descr::write() " << endl;
   return OK;
}

//
// class Working_Building_Descr 
// 
Working_Building_Descr::Working_Building_Descr(void) {
}

Working_Building_Descr::~Working_Building_Descr(void) {
}

int Working_Building_Descr::construct(Profile* p, Section *s) {
   // Parse working bob
   uint retval;
   retval=create_bob(p, s, "_w_??.bmp", "working_anim", &bob_working);
   if(retval) {
      return retval;
   } 

   return OK;
}
int Working_Building_Descr::write(Binary_file* f) {
   
   bob_working.write(f);
   
   // cerr << "Working_Building_Descr::write()" << endl;
   return OK;
}

//
// class Has_Is_A_Building_Descr
// 
Has_Is_A_Building_Descr::Has_Is_A_Building_Descr(void) {
   is_a=0;
}
Has_Is_A_Building_Descr::~Has_Is_A_Building_Descr(void) {
}

int Has_Is_A_Building_Descr::construct(Profile* p, Section* s) {
   // parse is_a
   const char* str=s->get_string("is_a", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"is_a");
      return KEY_MISSING;
   }
   if(!strcasecmp(str, "small")) is_a=IS_A_SMALL;
   else if(!strcasecmp(str, "medium")) is_a=IS_A_MEDIUM;
   else if(!strcasecmp(str, "big")) is_a=IS_A_BIG;
   else if(!strcasecmp(str, "mining")) is_a=IS_A_MINE;
   else {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"is_a");
      strcpy(err_msg,"value is not valid (either small, medium, big or mining)");
      return SYNTAX_ERROR;
   }

   return OK;
}
int Has_Is_A_Building_Descr::write(Binary_file* f) {

   f->write(&is_a, sizeof(ushort));
   // cerr << "Has_Is_A_Building_Descr::write()" << endl;
   return OK;
}

// 
// class Buildable_Building_Descr
//
Buildable_Building_Descr::Buildable_Building_Descr(void) {
   zmem(category, sizeof(category));
   build_time=0;
   ncost=0;
   cost=0;
}

Buildable_Building_Descr::~Buildable_Building_Descr(void) {
   if(ncost) {
      free(cost);
   }
}

int Buildable_Building_Descr::construct(Profile* p, Section *s) {
   Section* def=p->get_section("defaults");
   if(!def) def=s;

   // Parse category
   const char *str=s->get_string("category", def->get_string("category", "Misc"));
   memcpy(category, str, strlen(str) < (sizeof(category)) ? strlen(str) : (sizeof(category)-1));


   // parse build_time
   build_time=s->get_int("build_time", 0);
   if(!build_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"build_time");
      return KEY_MISSING;
   }

   // parse building bob
   uint retval;
   retval=create_bob(p, s, "_b_??.bmp", "build_anim", &bob_build);
   if(retval) {
      return retval;
   } 

   // parse cost list
   str=s->get_string("build_cost", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"build_cost");
      return KEY_MISSING;
   }
   uint i=0;
   ncost=1;
   for(i=0; i<strlen(str); i++) 
      if(str[i]==',') ncost++;

   cost=(Ware_List*) malloc(sizeof(Ware_List)*ncost);
   ushort n=0, z=0;
   char temp[50];
   for(i=0; i<ncost; i++) {
      while(str[n]!='*') {
         temp[z]=str[n];
         n++; z++;
      }
      temp[z]='\0'; 
      z=0;
      cost[i].num=atoi(temp);

      n++;
      while(str[n]==' ' || str[n]=='\t') n++;
      while(str[n]!=',' && str[n]!='\n' && str[n]!=' ' && str[n]!='\t' & str[n]!='\0') {
         temp[z]=str[n];
         n++; z++;
      }
      temp[z]='\0';
      z=0;
      if((int)i!=ncost-1) while(str[n]!=',') n++;
      n++;

      // name of ware is now in temp, get index
      cost[i].ware=waref.get(temp);
      z=0;
   }
   return OK;
}
int Buildable_Building_Descr::write(Binary_file* f) {
   // cerr << "Buildable_Building_Descr::write()" << endl;
   
   f->write(category, sizeof(category));
   f->write(&build_time, sizeof(ushort));
  
   // write cost
   f->write(&ncost, sizeof(ushort));
   int i;
   ushort temp;
   for(i=0; i<ncost; i++) {
      temp=cost[i].num;
      f->write(&temp, sizeof(ushort));
      temp=waref.get_index(cost[i].ware->get_name());
      f->write(&temp, sizeof(ushort));
   }
  
   bob_build.write(f);
   
   return OK;
}



//
// class Has_Needs_Building_Descr
//
Has_Needs_Building_Descr::Has_Needs_Building_Descr(void) {
   nneeds=0;
   needs_or=0; 
   needs=0;
}
Has_Needs_Building_Descr::~Has_Needs_Building_Descr(void) {
   if(nneeds) free(needs);
}

int Has_Needs_Building_Descr::construct(Profile* p, Section* s) {
   const char* str;
   uint i, n, z;
   char temp[50];
   ushort def_stock;

   // get default stock
   Section* def=p->get_section("defaults");
   if(!def) {
      def_stock=8;
   } else {
      def_stock=def->get_int("stock",8);
   }

   // parse work
   str=s->get_string("work", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"work");
      return KEY_MISSING;
   }
   i=0;
   if(str[0]!='(') {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"work");
      strcpy(err_msg,"Syntax error!");
      return SYNTAX_ERROR;
   }
   i=1;
   nneeds=1;
   uint ends;
   while(str[i]=='\t' || str[i]==' ' || str[i]=='\n' || str[i]=='\0') i++;
   if(str[i]==')') nneeds=0;
   while(str[i]!=')') { 
      if(str[i]==',' || str[i]=='|') { 
         if(str[i]==',') { 
            needs_or=0;
         } else {
            needs_or=1;
         }
         nneeds++; 
      }
      i++;
   }
   ends=i;
   z=0;
   n=1;
   if(nneeds) needs=(Need_List*) malloc(sizeof(Need_List)*nneeds);
   else needs=0;

   for(i=0; i<nneeds; i++) {
      while(str[n]!='*') {
         temp[z]=str[n];
         n++; z++;
      }
      temp[z]='\0'; 
      z=0;
      needs[i].num=atoi(temp);
      needs[i].stock=def_stock; 

      n++;
      while(str[n]==' ' || str[n]=='\t') n++;
      while(str[n]!=',' && str[n]!='|' && str[n]!='\n' && str[n]!=' ' && str[n]!='\t' && n<ends) {
         temp[z]=str[n];
         n++; z++;
      }
      temp[z]='\0';
      z=0;
      if((int)i!=nneeds-1) while(str[n]!=',' && str[n]!='|') n++;
      n++;

      // name of ware is now in temp, get index
      needs[i].ware=waref.get(temp);
      // cerr << needs[i].num <<":" <<  needs[i].ware->get_name() << ":" << needs_or << endl;
      z=0;
   }

   // parse stock
   str=s->get_string("stock", 0);
   if(str) {
      n=0;
      z=0;
      for(i=0; i<nneeds; i++) {
         if(n>=strlen(str)) break;
         while(str[n]!=',' && str[n]!='\0') {
            temp[z]=str[n];
            n++;
            z++;
         }
         temp[z]='\0';
         needs[i].stock=atoi(temp);
         z=0;
         n++;
      }
   }

   //   for(i=0; i<nneeds; i++) 
   //    cerr << needs[i].ware->get_name() << ":" << needs[i].num << ":" << needs[i].stock << endl;

   return OK;
}
int Has_Needs_Building_Descr::write(Binary_file* f) {
      
   uchar temp=needs_or;
   f->write(&temp, sizeof(uchar));

   // write needs
   f->write(&nneeds, sizeof(ushort));
   int i;
   ushort temp1;
   for(i=0; i<nneeds; i++) {
      temp1=needs[i].num;
      f->write(&temp1, sizeof(ushort));
      temp1=waref.get_index(needs[i].ware->get_name());
      f->write(&temp1, sizeof(ushort));
      temp1=needs[i].stock;
      f->write(&temp1, sizeof(ushort));
   }
  
  // cerr << "Has_Needs_Building_Descr::write()" << endl;
   return OK;
}


//
// class Has_Products_Building_Descr
// 
Has_Products_Building_Descr::Has_Products_Building_Descr(void) {
   products_or=0;
   nproducts=0;
   products=0;
}
Has_Products_Building_Descr::~Has_Products_Building_Descr(void) {
   if(nproducts) free(products);
}
int Has_Products_Building_Descr::construct(Profile* p, Section* s) {
   const char* str;
   uint i, z, n;
   char temp[50];

   str=s->get_string("work", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"work");
      return KEY_MISSING;
   }
   i=0;
   if(str[0]!='(') {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"work");
      strcpy(err_msg,"Syntax error!");
      return SYNTAX_ERROR;
   }
   i=1;
   nproducts=1;
   uint begins;
   while(str[i]!='(') i++;
   i++;
   while(str[i]=='\t' || str[i]==' ' || str[i]=='\n' || str[i]=='\0') i++;
   begins=i;
   if(str[i]==')') nproducts=0;
   while(str[i]!=')') { 
      if(str[i]==',' || str[i]=='|') { 
         if(str[i]==',') { 
            products_or=0;
         } else {
            products_or=1;
         }
         nproducts++;
      }
      i++;
   }

   z=0;
   n=begins;
   if(nproducts) products=(Product_List*) malloc(sizeof(Product_List)*nproducts);
   else products=0;
   for(i=0; i<nproducts; i++) {
      while(str[n]!='*') {
         temp[z]=str[n];
         n++; z++;
      }
      temp[z]='\0'; 
      z=0;
      products[i].num=atoi(temp);

      n++;
      while(str[n]==' ' || str[n]=='\t') n++;
      while(str[n]!=',' && str[n]!='|' && str[n]!='\n' && str[n]!=' ' && str[n]!='\t' && str[n]!=')') {
         temp[z]=str[n];
         n++; z++;
      }
      temp[z]='\0';
      z=0;
      if((int)i!=nproducts-1) while(str[n]!=',' && str[n]!='|') n++;
      n++;

      // name of ware is now in temp, get index
      products[i].ware=waref.get(temp);
      // cerr << products[i].num <<":" <<  products[i].ware->get_name() << ":" << products_or << endl;
      z=0;
   }

   //   for(i=0; i<nproducts; i++)
   //    cerr << products[i].ware->get_name() << ":" << products[i].num  << ":" << products_or << endl;

   return OK;
}
int Has_Products_Building_Descr::write(Binary_file* f) {
  // cerr << "Has_Products_Building_Descr::write()!" << endl;
 
   uchar temp=products_or;
   f->write(&temp, sizeof(uchar));

   // write needs
   f->write(&nproducts, sizeof(ushort));
   int i;
   ushort temp1;
   for(i=0; i<nproducts; i++) {
      temp1=products[i].num;
      f->write(&temp1, sizeof(ushort));
      temp1=waref.get_index(products[i].ware->get_name());
      f->write(&temp1, sizeof(ushort));
   }
   
   return OK;
}

//
// class Search_Building_Descr
//
Search_Building_Descr::Search_Building_Descr(void) {
   working_time=0;
   idle_time=0;
   working_area=0;
   worker=0;
   nbobs=0;
   bobs=0;
}

Search_Building_Descr::~Search_Building_Descr(void) {
   if(nbobs) delete[] bobs;
}

int Search_Building_Descr::construct(Profile* p, Section *s) {
   int retval;

   //   cerr << "Parsing Search_Building!" << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Boring_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Needs_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Products_Building_Descr::construct(p,s);
   if(retval) return retval;

   working_area=s->get_int("working_area", 0);
   if(!working_area) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"working_area");
      return KEY_MISSING;
   }

   working_time=s->get_int("working_time", 0);
   if(!working_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"working_time");
      return KEY_MISSING;
   }

   idle_time=s->get_int("idle_time", 0);
   if(!idle_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"idle_time");
      return KEY_MISSING;
   }

   // Parse worker
   const char* str;
   str=s->get_string("worker", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      return KEY_MISSING;
   }
   worker=workerf.get_searcher(str);
   if(!worker) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      strcpy(err_msg,"This worker is already defined as something else than a searcher!");
      return ERROR;
   }

   //  parse for list
   str=s->get_string("for", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"for");
      return KEY_MISSING;
   }
   uint i=0;
   nbobs=1;
   for(i=0; i<strlen(str); i++) 
      if(str[i]==',') nbobs++;

   bobs=new char[30*nbobs];
   zmem(bobs, 30*nbobs);

   ushort n=0, z=0;
   char temp[50];
   for(i=0; i<nbobs; i++) {
      while(str[n]==' ' || str[n]=='\t') n++;
      while(str[n]!=',' && str[n]!='\n' && str[n]!=' ' && str[n]!='\t' && str[n]!='\0') {
         temp[z]=str[n];
         n++; z++;
      }
      temp[z]='\0';
      z=0;
      if((int)i!=nbobs-1) while(str[n]!=',') n++;
      n++;

      // name of bob is now in temp, write it
      to_lower(temp);
      memcpy(&bobs[i*30], temp, strlen(temp) < 30 ? strlen(temp) : 29);
      needl.add_need(&bobs[i*30], Tribe_File_Need_List::IS_SOME_BOB);
      z=0;
   }

   return OK;
}
int Search_Building_Descr::write(Binary_file* f) {

   uchar id=SEARCH;
   f->write(&id, sizeof(uchar));

   Building_Descr::write(f);
   Boring_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);
   Has_Is_A_Building_Descr::write(f);
   Has_Needs_Building_Descr::write(f);
   Has_Products_Building_Descr::write(f);

   // write our own stuff
   f->write(&working_time, sizeof(ushort));
   f->write(&idle_time, sizeof(ushort));
   f->write(&working_area, sizeof(ushort));
   ushort temp;
   temp=workerf.get_index(worker->get_name());
   f->write(&temp, sizeof(ushort));
   f->write(&nbobs, sizeof(nbobs));
   f->write(bobs, nbobs*30);
   
   // cerr << "Search_Building_Descr::write()" << endl;
   return OK;
}

//
// class Plant_Building_Descr
//
Plant_Building_Descr::Plant_Building_Descr(void) {
   working_time=0;
   idle_time=0;
   working_area=0;
   worker=0;
   nbobs=0;
   bobs=0;
}
Plant_Building_Descr::~Plant_Building_Descr(void) {
   if(nbobs) delete[] bobs;
}

int Plant_Building_Descr::construct(Profile* p, Section *s) {
   int retval;

   //   cerr << "Parsing Plant_Building! " << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Boring_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Needs_Building_Descr::construct(p,s);
   if(retval) return retval;

   working_area=s->get_int("working_area", 0);
   if(!working_area) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"working_area");
      return KEY_MISSING;
   }

   working_time=s->get_int("working_time", 0);
   if(!working_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"working_time");
      return KEY_MISSING;
   }

   idle_time=s->get_int("idle_time", 0);
   if(!idle_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"idle_time");
      return KEY_MISSING;
   }

   // Parse worker
   const char* str;
   str=s->get_string("worker", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      return KEY_MISSING;
   }
   worker=workerf.get_planter(str);
   if(!worker) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      strcpy(err_msg,"This worker is already defined as something else than a planter!");
      return ERROR;
   }

   //  parse for list (list of bobs)
   str=s->get_string("for", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"for");
      return KEY_MISSING;
   }
   uint i=0;
   nbobs=1;
   for(i=0; i<strlen(str); i++) 
      if(str[i]==',') nbobs++;

   bobs=new char[30*nbobs];
   zmem(bobs, 30*nbobs);

   ushort n=0, z=0;
   char temp[50];
   for(i=0; i<nbobs; i++) {
      while(str[n]==' ' || str[n]=='\t') n++;
      while(str[n]!=',' && str[n]!='\n' && str[n]!=' ' && str[n]!='\t' && str[n]!='\0') {
         temp[z]=str[n];
         n++; z++;
      }
      temp[z]='\0';
      z=0;
      if((int)i!=nbobs-1) while(str[n]!=',') n++;
      n++;

      // name of bob is now in temp, write it
      to_lower(temp);
      memcpy(&bobs[i*30], temp, strlen(temp) < 30 ? strlen(temp) : 29);
      z=0;
      needl.add_need(&bobs[i*30], Tribe_File_Need_List::IS_GROWING_BOB);
   }

   return OK;
}
int Plant_Building_Descr::write(Binary_file* f) {
   // cerr << "Plant_Building_Descr::write()" << endl;

   uchar id=PLANT;
   f->write(&id, sizeof(uchar));

   Building_Descr::write(f);
   Boring_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);
   Has_Is_A_Building_Descr::write(f);
   Has_Needs_Building_Descr::write(f);

   // write our own stuff
   f->write(&working_time, sizeof(ushort));
   f->write(&idle_time, sizeof(ushort));
   f->write(&working_area, sizeof(ushort));
   ushort temp;
   temp=workerf.get_index(worker->get_name());
   f->write(&temp, sizeof(ushort));
   f->write(&nbobs, sizeof(nbobs));
   f->write(bobs, nbobs*30);

   return OK;
}

//
// Grow Building
// 
Grow_Building_Descr::Grow_Building_Descr(void) {
   worker=0;
   working_area=0;
   working_time=0;
   idle_time=0;
   zmem(plant_bob, sizeof(plant_bob));
   zmem(search_bob, sizeof(search_bob));
}
Grow_Building_Descr::~Grow_Building_Descr(void) {
}
int Grow_Building_Descr::construct(Profile* p, Section *s) {
   int retval;

   //   cerr << "Parsing Grow_Building! " << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Boring_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Needs_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Products_Building_Descr::construct(p,s);
   if(retval) return retval;

   working_area=s->get_int("working_area", 0);
   if(!working_area) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"working_area");
      return KEY_MISSING;
   }

   working_time=s->get_int("working_time", 0);
   if(!working_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"working_time");
      return KEY_MISSING;
   }

   idle_time=s->get_int("idle_time", 0);
   if(!idle_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"idle_time");
      return KEY_MISSING;
   }

   // Parse worker
   const char* str;
   str=s->get_string("worker", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      return KEY_MISSING;
   }
   worker=workerf.get_grower(str);
   if(!worker) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      strcpy(err_msg,"This worker is already defined as something else than a grower!");
      return ERROR;
   }

   // parse for
   str=s->get_string("for", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"for");
      return KEY_MISSING;
   }
   uint i;

   for(i=0; i<strlen(str); i++) if(str[i]==',') break;
   if(i==strlen(str)) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"for");
      strcpy(err_msg,"For growings, this must contain a ',' to seperate 'planting' and 'searching for' bob");
      return ERROR;
   }
   uint n=0;
   while(str[n]!=' ' && str[n]!='\t' && n<i) n++;
   memcpy(plant_bob, str, n < 30 ? n : 29 );
   to_lower(plant_bob);
   needl.add_need(plant_bob, Tribe_File_Need_List::IS_GROWING_BOB);

   n=i+1;
   while((str[n]==' ' || str[n]=='\t') && n<strlen(str)) n++;
   memcpy(search_bob, str+n, strlen(str)-n < 30 ? strlen(str)-n : 29); 
   to_lower(search_bob);
   needl.add_need(search_bob, Tribe_File_Need_List::IS_DIMINISHING_BOB);


   return OK;
}
int Grow_Building_Descr::write(Binary_file* f) {
      uchar id=GROW;
      f->write(&id, sizeof(uchar));

      Building_Descr::write(f);
      Boring_Building_Descr::write(f);
      Buildable_Building_Descr::write(f);
      Has_Is_A_Building_Descr::write(f);
      Has_Needs_Building_Descr::write(f);
      Has_Products_Building_Descr::write(f);

      // own stuff
      f->write(&working_time, sizeof(ushort));
      f->write(&idle_time, sizeof(ushort));
      f->write(&working_area, sizeof(ushort));
      ushort temp;
      temp=workerf.get_index(worker->get_name());
      f->write(&temp, sizeof(ushort));
      f->write(plant_bob, sizeof(plant_bob));
      f->write(search_bob, sizeof(search_bob));

      // cerr << "Grow_Building_Descr::write()" << endl;
   return OK;
}

//
// Science Building
// 
Science_Building_Descr::Science_Building_Descr(void) {
}
Science_Building_Descr::~Science_Building_Descr(void) {
}
int Science_Building_Descr::construct(Profile* p, Section *s) {
   int retval;

   cerr << "Parsing Science_Building! TODO" << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Working_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;


   return OK;
}
int Science_Building_Descr::write(Binary_file* f) {
   uchar id=SCIENCE;
   f->write(&id, sizeof(uchar));

   Building_Descr::write(f);
   Working_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);
   Has_Is_A_Building_Descr::write(f);
   
   cerr << "Science_Building_Descr::write() TODO!" << endl;
   return OK;
}

//
// Dig Building
// 
Dig_Building_Descr::Dig_Building_Descr(void) {
   working_time=0;
   idle_time=0;
   worker=0;
   zmem(resource, sizeof(resource));
}
Dig_Building_Descr::~Dig_Building_Descr(void) {
}
int Dig_Building_Descr::construct(Profile* p, Section *s) {
   int retval;

   //   cerr << "Parsing Dig_Building! " << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Working_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Needs_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Products_Building_Descr::construct(p,s);
   if(retval) return retval;


   working_time=s->get_int("working_time", 0);
   if(!working_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"working_time");
      return KEY_MISSING;
   }

   idle_time=s->get_int("idle_time", 0);
   if(!idle_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"idle_time");
      return KEY_MISSING;
   }

   // Parse worker
   const char* str;
   str=s->get_string("worker", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      return KEY_MISSING;
   }
   worker=workerf.get_sit_dig(str);
   if(!worker) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      strcpy(err_msg,"This worker is already defined as something else than a digger!");
      return ERROR;
   }

   // parse for: ONE RESOURCE
   str=s->get_string("for", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"for");
      return KEY_MISSING;
   }
   memcpy(resource, str, strlen(str) < sizeof(resource) ? strlen(str) : sizeof(resource)-1);
   to_lower(resource);
   needl.add_need(resource, Tribe_File_Need_List::IS_RESOURCE);

   return OK;
}
int Dig_Building_Descr::write(Binary_file* f) {
  
   uchar id=DIG;
   f->write(&id, sizeof(uchar));

   Building_Descr::write(f);
   Working_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);
   Has_Is_A_Building_Descr::write(f);
   Has_Needs_Building_Descr::write(f);
   Has_Products_Building_Descr::write(f);
   
   // own
   f->write(&working_time, sizeof(ushort));
   f->write(&idle_time, sizeof(ushort));
   ushort temp;
   temp=workerf.get_index(worker->get_name());
   f->write(&temp, sizeof(ushort));
   f->write(resource, sizeof(resource));
   
   // cerr << "Dig_Building_Descr::write()" << endl;
   
   return OK;
}

//
// Sit Building
// 
Sit_Building_Descr::Sit_Building_Descr(void) {
   worker=0;
   idle_time=0;
   working_time=0;
   order_worker=0;
}
Sit_Building_Descr::~Sit_Building_Descr(void) {
}
int Sit_Building_Descr::construct(Profile* p, Section *s) {
   int retval;

   //   cerr << "Parsing Sit_Building! " << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Working_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Needs_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Products_Building_Descr::construct(p,s);
   if(retval) return retval;

   working_time=s->get_int("working_time", 0);
   if(!working_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"working_time");
      return KEY_MISSING;
   }

   idle_time=s->get_int("idle_time", 0);
   if(!idle_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"idle_time");
      return KEY_MISSING;
   }

   // is order worker?
   order_worker=s->get_boolean("order_worker", 0);
   if(order_worker && !products_ored()) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"order_worker");
      strcpy(err_msg,"not valid with 'and'ed products!");
      return ERROR;
   }

   // Parse worker
   const char* str;
   str=s->get_string("worker", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      return KEY_MISSING;
   }
   worker=workerf.get_sit_dig(str);
   if(!worker) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      strcpy(err_msg,"This worker is already defined as something else than a sitter!");
      return ERROR;
   }


   return OK;
}
int Sit_Building_Descr::write(Binary_file* f) {

   uchar id=SIT;
   f->write(&id, sizeof(uchar));
  

   Building_Descr::write(f);
   Working_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);
   Has_Is_A_Building_Descr::write(f);
   Has_Needs_Building_Descr::write(f);
   Has_Products_Building_Descr::write(f);
                      
   // our stuff
   f->write(&working_time, sizeof(ushort));
   f->write(&idle_time, sizeof(ushort));
   ushort temp;
   temp=workerf.get_index(worker->get_name());
   f->write(&temp, sizeof(ushort));
   id=order_worker;
   f->write(&id, sizeof(uchar));

   // cerr << "Sit_Building_Descr::write()!" << endl;
   return OK;
}

// 
// Sit building which makes workers
// 
Sit_Building_Produ_Worker_Descr::Sit_Building_Produ_Worker_Descr(void) {
   idle_time=0;
   working_time=0;
   worker=0;
   prod_worker=0;
}
Sit_Building_Produ_Worker_Descr::~Sit_Building_Produ_Worker_Descr(void) {
}
int Sit_Building_Produ_Worker_Descr::construct(Profile* p, Section *s) {
   int retval;

   //   cerr << "Parsing Sit_Building_Produ_Worker_Descr!" << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Working_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Needs_Building_Descr::construct(p,s);
   if(retval) return retval;

   working_time=s->get_int("working_time", 0);
   if(!working_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"working_time");
      return KEY_MISSING;
   }

   idle_time=s->get_int("idle_time", 0);
   if(!idle_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"idle_time");
      return KEY_MISSING;
   }

   // Parse worker
   const char* str;
   str=s->get_string("worker", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      return KEY_MISSING;
   }
   worker=workerf.get_sit_dig(str);
   if(!worker) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      strcpy(err_msg,"This worker is already defined as something else than a sitter!");
      return ERROR;
   }

   // Parse production
   str=s->get_string("work",0); // work exists for sure
   char temp[50];
   uint i=0;
   while(str[i]!=')') i++;
   while(str[i]!='(') i++;
   i++;
   while(str[i]=='\t' || str[i]=='\n' || str[i]==' ') i++;
   strncpy(temp, str+i, 49);
   i=0;
   while(temp[i]!=')' && temp[i]!='\t' && temp[i]!='\n' && temp[i]!=' ') i++;
   temp[i]='\0';
   prod_worker=workerf.get_sit_dig(temp);
   if(!prod_worker) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      strcpy(err_msg,"The produced worker is already defined as something else then a sitter!");
      return ERROR;
   }

   return OK;
}
int Sit_Building_Produ_Worker_Descr::write(Binary_file* f) {
   uchar id=SIT_PRODU_WORKER;
   f->write(&id, sizeof(uchar));

   Building_Descr::write(f);
   Working_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);
   Has_Is_A_Building_Descr::write(f);
   Has_Needs_Building_Descr::write(f);

   // own stuff
   f->write(&working_time, sizeof(ushort));
   f->write(&idle_time, sizeof(ushort));
   ushort temp;
   temp=workerf.get_index(worker->get_name());
   f->write(&temp, sizeof(ushort));
   temp=workerf.get_index(prod_worker->get_name());
   f->write(&temp, sizeof(ushort));
   
   // cerr << "Sit_Building_Produ_Descr::write()!" << endl;
   return OK;
}

// 
// Military Building
//
Military_Building_Descr::Military_Building_Descr(void) {
   beds=0;
   conquers=0;
   idle_time=0;
   nupgr=0;
}
Military_Building_Descr::~Military_Building_Descr(void) {
}

int Military_Building_Descr::construct(Profile* p, Section* s) {
   int retval;

   // cerr << "Parsing military building!" << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Boring_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Needs_Building_Descr::construct(p,s);
   if(retval) return retval;

   idle_time=s->get_int("idle_time", 0);
   if(!idle_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"idle_time");
      return KEY_MISSING;
   }

   beds=s->get_int("beds", 0);
   if(!beds) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"beds");
      return KEY_MISSING;
   }

   conquers=s->get_int("conquers", 0);
   if(!conquers) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"conquers");
      return KEY_MISSING;
   }

   // number of upgraded soldiers per update?
   const char* str=s->get_string("work",0); // work exists for sure
   char temp[50];
   uint i=0;
   while(str[i]!=')') i++;
   while(str[i]!='(') i++;
   i++;
   strcpy(temp, str+i);
   i=0;
   while(temp[i]!=')') i++;
   temp[i]='\0';
   nupgr=atoi(temp);

   return OK;
}
int Military_Building_Descr::write(Binary_file* f) {
//   cerr << "Military_Building_Descr::write()" << endl;
   
   uchar id=MILITARY;
   f->write(&id, sizeof(uchar));
   
   Building_Descr::write(f);
   Boring_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);
   Has_Is_A_Building_Descr::write(f);
   Has_Needs_Building_Descr::write(f);

   // own stuff
   f->write(&beds, sizeof(ushort));
   f->write(&conquers, sizeof(ushort));
   f->write(&idle_time, sizeof(ushort));
   f->write(&nupgr, sizeof(ushort));
   
   return OK;
}

//
// Cannons
// 
Cannon_Descr::Cannon_Descr(void) {
   worker=0;
   projectile_speed=0;
   fires_balistic=false;
   idle_time=0;
   wproj=0;
   hproj=0;
}
Cannon_Descr::~Cannon_Descr(void) {
}

int Cannon_Descr::construct(Profile* p, Section* s) {
   int retval;

   // cerr << "Parsing cannon!" << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Boring_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Needs_Building_Descr::construct(p,s);
   if(retval) return retval;

   const char* str;
   str=s->get_string("worker", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      return KEY_MISSING;
   }
   worker=workerf.get_sit_dig(str);
   if(!worker) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      strcpy(err_msg,"This worker is already defined as something else than a sitter!");
      return ERROR;
   }

   idle_time=s->get_int("idle_time", 0);
   if(!idle_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"idle_time");
      return KEY_MISSING;
   }

   str=s->get_string("firing_type", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"firing_type");
      return KEY_MISSING;
   }
   if(!strcasecmp(str, "balistic")) {
      fires_balistic=true;
   } else if(!strcasecmp(str, "direct")) {
      fires_balistic=false;
   } else {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"firing_type");
      strcpy(err_msg,"Value is not valid (either balistic or direct)");
      return ERROR;
   } 

   projectile_speed=s->get_int("projectile_speed", 0);
   if(!projectile_speed) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"projectile_speed");
      return KEY_MISSING;
   }

   // Load the bobs
   retval=create_bob(p, s, "_a_??.bmp", "projectile_bob", &bob_projectile, &wproj, &hproj);
   if(retval) return retval;
   retval=create_bob(p, s, "_fire_ne_??.bmp", "fire_ne_anim", &bob_fire_ne);
   if(retval) return retval;
   retval=create_bob(p, s, "_fire_e_??.bmp", "fire_e_anim", &bob_fire_e);
   if(retval) return retval;
   retval=create_bob(p, s, "_fire_se_??.bmp", "fire_se_anim", &bob_fire_se);
   if(retval) return retval;
   retval=create_bob(p, s, "_fire_sw_??.bmp", "fire_sw_anim", &bob_fire_sw);
   if(retval) return retval;
   retval=create_bob(p, s, "_fire_w_??.bmp", "fire_w_anim", &bob_fire_w);
   if(retval) return retval;
   retval=create_bob(p, s, "_fire_nw_??.bmp", "fire_nw_anim", &bob_fire_nw);
   if(retval) return retval;

   return OK;
}
int Cannon_Descr::write(Binary_file* f) {
   uchar id=CANNON;
   f->write(&id, sizeof(uchar));

   Building_Descr::write(f);
   Boring_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);
   Has_Is_A_Building_Descr::write(f);
   Has_Needs_Building_Descr::write(f);

   // own stuff
   f->write(&idle_time, sizeof(ushort));
   f->write(&projectile_speed, sizeof(ushort));
   id=fires_balistic;
   f->write(&id, sizeof(uchar));
   ushort temp;
   temp=workerf.get_index(worker->get_name());
   f->write(&temp, sizeof(ushort));
   // width and height ob projectile bob
   temp=wproj;
   f->write(&temp, sizeof(ushort));
   temp=hproj;
   f->write(&temp, sizeof(ushort));
   bob_projectile.write(f);
   bob_fire_ne.write(f);
   bob_fire_e.write(f);
   bob_fire_se.write(f);
   bob_fire_sw.write(f);
   bob_fire_w.write(f);
   bob_fire_nw.write(f);

  // cerr << "Cannon_Descr::write()" << endl;
   return OK;
}


// 
// SPECIAL buildings
//


//
//HQ description
//
HQ_Descr::HQ_Descr(void) {
   conquers=0;
}
HQ_Descr::~HQ_Descr(void) {
}
int HQ_Descr::construct(Profile* p, Section* s) {
   int retval;

   //   cerr << "Parsing HQ!" << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Boring_Building_Descr::construct(p,s); 
   if(retval) return retval;

   conquers=s->get_int("conquers", 0);
   if(!conquers) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"conquers");
      return KEY_MISSING;
   }



   return OK;
}
int HQ_Descr::write(Binary_file* f) {
   uchar id=SPEC_HQ;
   f->write(&id, sizeof(uchar));

   Building_Descr::write(f);
   Boring_Building_Descr::write(f);

   // own
   f->write(&conquers, sizeof(ushort));

   // cerr << "HQ_Descr::write()" << endl;
   return OK;
}

//
//Store description
//
Store_Descr::Store_Descr(void) {
}
Store_Descr::~Store_Descr(void) {
}
int Store_Descr::construct(Profile* p, Section* s) {
   int retval;

   //   cerr << "Parsing Store!" << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Boring_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;

   return OK;
}
int Store_Descr::write(Binary_file* f) {
   uchar id=SPEC_STORE;
   f->write(&id, sizeof(uchar));

   Building_Descr::write(f);
   Boring_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);
   Has_Is_A_Building_Descr::write(f);

   // cerr << "Store_Descr::write()!" << endl;
   return OK;
}

//
//Dockyard description, is a IS_A_PORT
//
Dockyard_Descr::Dockyard_Descr(void) {
   working_time=0;
   idle_time=0;
   worker=0;
}
Dockyard_Descr::~Dockyard_Descr(void) {
}
int Dockyard_Descr::construct(Profile* p, Section* s) {
   int retval;

   // cerr << "Parsing Dockyard!" << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Boring_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Has_Is_A_Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Has_Needs_Building_Descr::construct(p,s); 
   if(retval) return retval;

   working_time=s->get_int("working_time", 0);
   if(!working_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"working_time");
      return KEY_MISSING;
   }

   idle_time=s->get_int("idle_time", 0);
   if(!idle_time) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"idle_time");
      return KEY_MISSING;
   }

   // Parse worker
   const char* str;
   str=s->get_string("worker", 0);
   if(!str) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      return KEY_MISSING;
   }
   worker=workerf.get_planter(str);
   if(!worker) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"worker");
      strcpy(err_msg,"This worker is already defined as something else than a planter!");
      return ERROR;
   }


   return OK;
}
int Dockyard_Descr::write(Binary_file* f) {
   uchar id=SPEC_DOCKYARD;
   f->write(&id, sizeof(uchar));
   
   Building_Descr::write(f);
   Boring_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);
   Has_Is_A_Building_Descr::write(f);
   Has_Needs_Building_Descr::write(f);

   // own
   f->write(&working_time, sizeof(ushort));
   f->write(&idle_time, sizeof(ushort));
   ushort temp;
   temp=workerf.get_index(worker->get_name());
   f->write(&temp, sizeof(ushort));

      // cerr << "Dockyard_Descr::write()!" << endl;
   return OK;
}

//
// Port description
//
// This one is a little hacking around. this is a must change for further
// versions. 
//
Port_Descr::Port_Descr(void) {
}
Port_Descr::~Port_Descr(void) {
}
int Port_Descr::construct(Profile* p, Section* s) {
   int retval;

   //   cerr << "Parsing Port!" << endl;

   retval=Building_Descr::construct(p,s);
   if(retval) return retval;
   retval=Boring_Building_Descr::construct(p,s); 
   if(retval) return retval;
   retval=Buildable_Building_Descr::construct(p,s); 
   if(retval) return retval;

   return OK;
}
int Port_Descr::write(Binary_file* f) {
   uchar id=SPEC_PORT;
   f->write(&id, sizeof(uchar));

   Building_Descr::write(f);
   Boring_Building_Descr::write(f);
   Buildable_Building_Descr::write(f);

   // cerr << "Port_Descr::write()" << endl;
   return OK;
}

