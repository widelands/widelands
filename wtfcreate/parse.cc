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
#include "soldier_descr.h"
#include "worker_descr.h"
#include "logic_bob_descr.h"
#include "parse.h"
#include "parse_bobs.h"

#define OK        0
#define ERROR     1

void key_missing(const char* file, const char* section, const char* key) {
   cerr << "In file <" << file << ">:" << endl
      << "\t" <<  section << "(" << key << "): mandatory key is missing!" << endl;
};

void sec_missing(const char* file, const char* sec) {
   cerr << "In file <" << file << ">:" << endl
      << "\t" << sec << ": mandatory section is missing!" << endl;
}

void error(const char* file, const char* msg) {
   cerr << "In file <" << file << ">:" << endl
      << "\t" << msg << endl;
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

int parse_buildings(Buildings_Descr* pbuildings) {
   char *file= new char[strlen(g_dirname)+strlen(BUILDINGS_DIR)+strlen(CONF_NAME)+2];

   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, BUILDINGS_DIR);
   strcat(file, CONF_NAME);

   Profile *p = new Profile(cout, file);
   Section* s;
   int retval;

   // directly address [defaults], so our loop won't try to use it
   s=p->get_section("defaults");

   // parse defaults
   pbuildings->nbuilds=0;
   pbuildings->builds=(Building_Descr**) malloc(sizeof(Building_Descr*)*99);

   // parse HQ
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No HQ description found!");
      delete[] file;
      return ERROR;
   }
   pbuildings->builds[0]=new HQ_Descr;
   retval=pbuildings->builds[0]->construct(p,s);
   if(retval) {
      delete p;
      inform_over_construct_error(file, pbuildings->builds[0], retval);
      delete[] file;
      return ERROR;
   } 
   pbuildings->nbuilds++;

   // parse store
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No Store description found!");
      delete[] file;
      return ERROR;
   }
   pbuildings->builds[1]=new Store_Descr;
   retval=pbuildings->builds[1]->construct(p,s);
   if(retval) {
      delete p;
      inform_over_construct_error(file, pbuildings->builds[1], retval);
      delete[] file;
      return ERROR;
   } 
   pbuildings->nbuilds++;


   // parse dockyard
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No dockyard description found!");
      delete[] file;
      return ERROR;
   }
   pbuildings->builds[2]=new Dockyard_Descr;
   retval=pbuildings->builds[2]->construct(p,s);
   if(retval) {
      delete p;
      inform_over_construct_error(file, pbuildings->builds[2], retval);
      delete[] file;
      return ERROR;
   } 
   pbuildings->nbuilds++;


   // parse port
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No Port description found!");
      delete[] file;
      return ERROR;
   }
   pbuildings->builds[3]=new Port_Descr;
   retval=pbuildings->builds[3]->construct(p,s);
   if(retval) {
      delete p;
      inform_over_construct_error(file, pbuildings->builds[3], retval);
      delete[] file;
      return ERROR;
   } 
   pbuildings->nbuilds++;

   // Parse ordinary buildings
   const char* str;
   uint i=4;
   s=p->get_next_section(0);
   while(s) {
      str=s->get_string("type");
      if(!str) {
         delete p;
         key_missing(file, s->get_name(), "type");
         delete[] file;
         return ERROR; 
      }
      if(!strcasecmp(str, "search")) {
         pbuildings->builds[i]=new Search_Building_Descr();
      } else if(!strcasecmp(str, "plant")) {
         pbuildings->builds[i]=new Plant_Building_Descr();
      } else if(!strcasecmp(str, "grow")) {
         pbuildings->builds[i]=new Grow_Building_Descr();
      } else if(!strcasecmp(str, "dig")) {
         pbuildings->builds[i]=new Dig_Building_Descr();
      } else if(!strcasecmp(str, "sit")) {
         if(s->get_boolean("produces_worker", 0)) {
            pbuildings->builds[i]=new Sit_Building_Produ_Worker_Descr();
         } else {
            pbuildings->builds[i]=new Sit_Building_Descr();
         }
      } else if(!strcasecmp(str, "science")) {
         pbuildings->builds[i]=new Science_Building_Descr();
      } else if(!strcasecmp(str, "military")) {
         pbuildings->builds[i]=new Military_Building_Descr();
      } else if(!strcasecmp(str, "cannon")) {
         pbuildings->builds[i]=new Cannon_Descr();
      } else {
         delete p;
         cerr << "In file <" << file << ">:" << endl;
         cerr << "\t" << s->get_name() << "(type): "  << "\"" 
            << str << "\" is not valid! (search, plant, grow, dig, sit, science, military, cannon)" << endl;
         delete[] file;
         return ERROR; 
      }
      
      if((retval=pbuildings->builds[i]->construct(p, s))) {
         delete p;
         inform_over_construct_error(file, pbuildings->builds[i], retval);
         delete[] file;
         return ERROR;
      }      
      i++;
      pbuildings->nbuilds++;
      s=p->get_next_section(0);
   }

   pbuildings->builds=(Building_Descr**) realloc(pbuildings->builds, sizeof(Building_Descr*)*pbuildings->nbuilds);

   delete[] file;
   delete p;

   return OK;
}

int parse_wares(void) {
   char *file= new char[strlen(g_dirname)+strlen(WARES_DIR)+strlen(CONF_NAME)+2];

   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, WARES_DIR);
   strcat(file, CONF_NAME);
   Profile *p = new Profile(cout, file);
   Section* s;

   int retval;

   Ware_Descr* w=waref.start_enum();
   while(w) {
      s=p->get_section(w->get_name());
      if((retval=w->construct(p, s))) {
         delete p;
         inform_over_construct_error(file, w, retval);
         delete[] file;
         return ERROR;
      }
      w=waref.get_nitem();
   }

   delete p;
   delete[] file;

   return OK; 
}

int parse_workers(void) {
   char *file= new char[strlen(g_dirname)+strlen(WORKERS_DIR)+strlen(CONF_NAME)+2];

   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, WORKERS_DIR);
   strcat(file, CONF_NAME);
   Profile *p = new Profile(cout, file);
   Section* s;

   int retval;

   Worker_Descr* w=workerf.start_enum();
   while(w) {
      s=p->get_section(w->get_name());
      if(!s) {
         delete p;
         sec_missing(file, w->get_name());
         delete[] file;
         return ERROR;
      }
      if((retval=w->construct(p,s))) {
         delete p;
         inform_over_construct_error(file, w, retval);
         delete[] file;
         return ERROR;
      }
      w=workerf.get_nitem();
   }

   delete p;
   delete[] file;

   return OK;
}


int create_def_worker(void) {
   char *file= new char[strlen(g_dirname)+strlen(WORKERS_DIR)+strlen(CONF_NAME)+2];

   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, WORKERS_DIR);
   strcat(file, CONF_NAME);
   Profile *p = new Profile(cout, file, 0, 1);
   Section* s;

   // touch default
   p->get_section("defaults");

   Sit_Dig_Base_Descr* w;

   // get first worker: def_carrier
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No default-carrier description found!");
      delete[] file;
      return ERROR;
   }
   w=new Def_Carrier_Descr(s->get_name());
   workerf.add_sit_dig(w);

   // get next worker: builder
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No Builder description found!");
      delete[] file;
      return ERROR;
   }
   w=new Builder_Descr(s->get_name());
   workerf.add_sit_dig(w);

   // Planer
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No Planer description found!");
      delete[] file;
      return ERROR;
   }
   w=new Planer_Descr(s->get_name());
   workerf.add_sit_dig(w);

   // Explorer
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No Explorer description found!");
      delete[] file;
      return ERROR;
   }
   w=new Explorer_Descr(s->get_name());
   workerf.add_sit_dig(w);

   // Geologist
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No Geologist description found!");
      delete[] file;
      return ERROR;
   }
   w=new Geologist_Descr(s->get_name());
   workerf.add_sit_dig(w);

   // Add_Carrier
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No second carrier description found!");
      delete[] file;
      return ERROR;
   }
   w=new Add_Carrier_Descr(s->get_name());
   workerf.add_sit_dig(w);

   // 2. add_carrier
   s=p->get_next_section(0);
   if(!s) {
      delete p;
      error(file, "No third carrier description found!");
      delete[] file;
      return ERROR;
   }
   w=new Add_Carrier_Descr(s->get_name());
   workerf.add_sit_dig(w);

   delete p;

   delete[] file;

   return OK;
}

int parse_soldiers(void) {
   char *file= new char[strlen(g_dirname)+strlen(SOLDIERS_DIR)+strlen(CONF_NAME)+2];

   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, SOLDIERS_DIR);
   strcat(file, CONF_NAME);

   Profile *p = new Profile(cout, file);
   Section* s;
   int retval;

   // directly address [defaults], so our loop won't try to use it
   s=p->get_section("defaults");
   
   s=p->get_next_section(0);
   Soldier_Descr* sol;
   while(s) {
      sol=soldierf.get(s->get_name());
      if((retval=sol->construct(p,s))) {
         delete p;
         inform_over_construct_error(file, sol, retval);
         delete[] file;
         return ERROR;
      }
      s=p->get_next_section(0);
   }

   delete[] file;
   delete p;

   return OK;
}

int parse_root_conf(Tribe_Header* ph, Regent_Descr* pregent) {
   char *file= new char[strlen(g_dirname)+strlen(CONF_NAME)+2];

   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, CONF_NAME);

   Profile *p = new Profile(cout, file);
   Section* s;
   
   int retval;

   s=p->get_section("tribe");
   if(!s) {
      delete p;
      sec_missing(file, "tribe");
      delete[] file;
      return ERROR;
   }
   if((retval=ph->construct(p,s))) {
      delete p;
      inform_over_construct_error(file, ph, retval);
      delete[] file;
      return ERROR;
   }

   s=p->get_section("regent");
   if(!s) {
      delete p;
      sec_missing(file, "regent");
      delete[] file;
      return ERROR;
   }
   if((retval=pregent->construct(p,s))) {
      delete p;
      inform_over_construct_error(file, pregent, retval);
      delete[] file;
      return ERROR;
   }

   delete p;
   delete[] file;

   return OK;
}

int parse(Buildings_Descr* buildings, Tribe_Header* header, Regent_Descr* regent) {
   int retval=OK;
   
   if(create_def_worker()) {
      retval=-1;
      return retval; 
   } 

   // parse bobs
   if(parse_bobs()) {
      retval=-1;
      return retval;
   }

   if(parse_buildings(buildings)) {
      // uho, some error. error msg is already printed. cleanup and die
      retval=-1;
      goto cleanup;
   }

   if(parse_wares()) {
      retval=-1;
      goto cleanup;
   }

   // parse workers 
   if(parse_workers()) {
      retval=-1;
      goto cleanup;
   }

   // parse_soldiers
   if(parse_soldiers()) {
      retval=-1;
      goto cleanup;
   }

   // parse root conf
   if(parse_root_conf(header, regent)) {
      retval=-1;
      goto cleanup;
   }

cleanup:
   if(retval) {
      // deleting buildings
      if(buildings->nbuilds) {   
         uint i;
         for(i=0; i<buildings->nbuilds; i++) {
            delete buildings->builds[i];
         }
      } 
      free(buildings->builds);
   }

   return retval ;


   return OK;
}



