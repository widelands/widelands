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
#include "../src/mydirent.h"
#include "../wtfcreate/logic_bob_descr.h"
#include "parse.h"
#include "../wtfcreate/parse_bobs.h"
#include "wwfcreate.h"
#include "resource_descr.h"
#include "terrain_descr.h"

void key_missing(const char* file, const char* section, const char* key) {
   cerr << "In file <" << file << ">:" << endl
      << "\t" <<  section << "(" << key << "): mandatory key is missing!" << endl;
};

void sec_missing(const char* file, const char* sec) {
   cerr << "In file <" << file << ">:" << endl
      << "\t" << sec << ": mandatory section is missing!" << endl;
}

void error(const char* file, File_Descr* f) {
   cerr << "In file <" << file << ">:" << endl
      << "\t" << f->get_last_err_section() << " (" << f->get_last_err_key() << "): " 
      << f->get_last_err_msg() << endl;
}

void inform_over_construct_error(const char* file, File_Descr* b, int error) {
   if(error==File_Descr::KEY_MISSING) {
      key_missing(file, b->get_last_err_section(),
            b->get_last_err_key());
   } else if(error==File_Descr::SECTION_MISSING) {
      sec_missing(file,  b->get_last_err_section());
   } else {
      cerr << "In file <" << file << ">:" << endl;
      cerr << "\t" <<
         b->get_last_err_section() << "(" << b->get_last_err_key() << ") :"
         << b->get_last_err_msg() << endl;
   }
}

int parse_root_conf(char* name, char* author, char* descr) {
   char *file= new char[strlen(g_dirname)+strlen(CONF_NAME)+2];

   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, CONF_NAME);

   Profile *p = new Profile(cout, file);
   Section* s;
   
   s=p->get_section("world");
   if(!s) {
      delete p;
      sec_missing(file, "world");
      delete[] file;
      return ERR_FAILED;
   }
   
   const char* str=s->get_string("name", 0);
   if(!str) {
      delete p;
      key_missing(file, "world", "name");
      delete[] file;
      return ERR_FAILED;
   }
   strncpy(name, str, 30);
   name[29]='\0';

   str=s->get_string("author", 0);
   if(!str) {
      delete p;
      key_missing(file, "world", "author");
      delete[] file;
      return ERR_FAILED;
   }
   strncpy(author, str, 30);
   author[29]='\0';

   str=s->get_string("descr", 0);
   if(!str) {
      delete p;
      key_missing(file, "world", "descr");
      delete[] file;
      return ERR_FAILED;
   }
   strncpy(descr, str, 1024);
   descr[1023]='\0';


   delete p;
   delete[] file;

   return RET_OK;
}

int parse_resource_conf(void) {
   char* file= new char[strlen(g_dirname)+strlen(RESCONF)+2];

   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, RESCONF);

   Profile* p=new Profile(cout, file, 0, 0);
   Section* s=0;
   
   s=p->get_next_section(0);
   Resource_Descr* res;
   while(s) {
      res=resf.get(s->get_name());
      res->construct(p,s);
      s=p->get_next_section(0);
   }
   
   delete p;
   delete[] file;

   return RET_OK;
}

int parse_terrains_conf(void) {
   char* file= new char[strlen(g_dirname)+strlen(TERRAINCONF)+2];

   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, TERRAINCONF);

   Profile* p=new Profile(cout, file, 0, 0);
   Section* s=0;

   s=p->get_next_section(0);
   Terrain_Descr* ter;
   int retval;
   while(s) {
      ter=terf.get(s->get_name());
      if((retval=ter->construct(p,s))) {
         switch(retval) {
            case File_Descr::ERROR:
               delete p;
               error(file, ter);
               delete[] file;
               return ERR_FAILED;

            case File_Descr::KEY_MISSING:
               delete p;
               key_missing(file, ter->get_last_err_section(), ter->get_last_err_key());
               delete[] file;
               return ERR_FAILED;

            case File_Descr::SECTION_MISSING:
               delete p;
               sec_missing(file, ter->get_last_err_section());
               delete[] file;
               return ERR_FAILED;
         }
      }
      s=p->get_next_section(0);
   }

   delete p;
   delete[] file;

   return RET_OK;
}

int parse(char* name, char* author, char* descr) {
   // parse root conf
   if(parse_root_conf(name, author, descr)) {
      return -1;
   } 
   
   // parse resources
   if(parse_resource_conf()) {
      return -1;
   }
   
   // parse terrains
   if(parse_terrains_conf()) {
      return -1;
   } 
   
   // parse bobs
   if(parse_bobs()) {
      return -1;
   }

 
   return RET_OK;
}



