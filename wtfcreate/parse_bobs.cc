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

#define OK        0
#define ERROR     1

void key_missing(const char* file, const char* section, const char* key);
void sec_missing(const char* file, const char* sec); 
void error(const char* file, const char* msg);
void inform_over_construct_error(const char* file, File_Descr* b, int error);

int parse_bobs(void) {
   // Enumerate the subdirs
   char* dir= new char[strlen(g_dirname)+strlen(BOBS_DIR)+2];

   strcpy(dir, g_dirname);
   strcat(dir, SSEPERATOR);
   strcat(dir, BOBS_DIR);

   DIR* d=opendir(dir);
   if(!d) {
      cerr << dir << ": dir not found or read error!" << endl;
   }
   struct dirent *file;
   Profile* p;
   Section* s;
   const char* type; 
   Logic_Bob_Descr* bob;
   int retval;
   
   // first turn: care for diminishings!
   while((file=readdir(d))) {
      if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")) continue;
      char* filename= new char[strlen(file->d_name)+strlen(dir)+strlen(CONF_NAME)+2];

      strcpy(filename, dir);
      strcat(filename, file->d_name);
      filename[strlen(dir)+strlen(file->d_name)]=SEPERATOR;
      filename[strlen(dir)+strlen(file->d_name)+1]='\0';
      strcat(filename, CONF_NAME);

      // Parse file, check if it is a diminishing bob
      p=new Profile(cout , filename, true);

      s=p->get_next_section(NULL);
      type=s->get_string("type", 0);
      if(!type) {
         delete p;
         sec_missing(filename, "type");
         delete[] filename;
         delete[] dir;
         closedir(d);
         return ERROR;
      }

      if(!strcasecmp(type, "diminishing")) {
         bob=new Diminishing_Bob_Descr(file->d_name);
      } else if(!strcasecmp(type, "growing")) {
         bob=new Growing_Bob_Descr(file->d_name);
      } else if(!strcasecmp(type, "boring")) {
         bob=new Boring_Bob_Descr(file->d_name);
      } else {
         delete p;
         sec_missing(filename, "type");
         cerr << "In file <" << filename << ">:" << endl
            << "\ttype is not valid! (either diminishing, growing, boring, critter)" << endl;
         delete[] filename;
         delete[] dir;
         closedir(d);
         return ERROR;
      }
      
      if((retval=bob->construct(p,s))) {
         delete p;
         inform_over_construct_error(filename, bob, retval);
         delete[] filename;
         delete[] dir;
         closedir(d);
         return ERROR;
      }
 

      bobf.add(bob);

      delete p;
      delete[] filename;
   }
    
   closedir(d);
   
   delete[] dir;

   return 0;
}



