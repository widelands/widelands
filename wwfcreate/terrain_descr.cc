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
#include "../src/helper.h"
#include "terrain_descr.h"
#include "resource_descr.h"
#include "wwfcreate.h"

Fabric<Terrain_Descr> terf;


//
// class Terrain_Descr
//
Terrain_Descr::Terrain_Descr(const char* gname) {
   strncpy(name, gname, 30);
   name[29]='\0';
   def_res=-1;
   def_stock=-1;
   nres=0;
   res=0;
   ntextures=0;
   tex=0;
   minh=maxh=0;
   is=0;
}

Terrain_Descr::~Terrain_Descr(void) {
   if(nres) delete[] res;
   if(ntextures) {
      uint i;
      for(i=0; i<ntextures; i++)
         delete tex[i];
      delete[] tex;
   }
}

int Terrain_Descr::construct(Profile* p, Section *s) {
   
   minh=s->get_int("min_height", 0);
   maxh=s->get_int("max_height", 255);
  
   // get def_res
   const char* str=s->get_string("def_res", 0);
   if(str) {
      Resource_Descr* temp;
      temp=resf.exists(str);
      if(!temp) {
         strcpy(err_sec,s->get_name());
         strcpy(err_key,"def_res");
         strcpy(err_msg,": Resource does not exist!");
         return ERROR;  
      } 
      def_res=resf.get_index(temp->get_name());
      
      def_stock=s->get_int("def_stock", 0);
   }
  
   // switch is
   str=s->get_string("is", 0);
   if(!str) {
      strcpy(err_sec, s->get_name());
      strcpy(err_key, "is");
      return KEY_MISSING;
   }

   if(!strcmpi(str, "dry")) {
      is=TERRAIN_DRY;
   } else if(!strcmpi(str, "green")) {
      is=TERRAIN_GREEN;
   } else if(!strcmpi(str, "water")) {
      is=TERRAIN_WATER;
   } else if(!strcmpi(str, "acid")) {
      is=TERRAIN_ACID;
   } else if(!strcmpi(str, "mountain")) {
      is=TERRAIN_MOUNTAIN;
   } else {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"is");
      strcpy(err_msg, str);
      strcat(err_msg,": Not a valid type (dry, green, water, acid, mountain)");
      return ERROR;
   }                              

   // parse resources
   str=s->get_string("resources", 0);
   if(str && strcmpi("", str)) {
      nres=1;
      uint i=0;
      while(i < strlen(str)) { if(str[i]==',') { nres++; }  i++; }

      res=new uchar[nres];
      char temp[200];
      uint n=0;
      uint cur_res=0;
      i=0;
      Resource_Descr* rtemp;
      while(i<strlen(str)) {
         temp[n]=str[i];
         i++;
         n++;
         if(str[i]==',') {
            temp[n]='\0';
            n--;
            while(temp[n] == ',' || temp[n]==' ' || temp[n]=='\t') temp[n--]='\0';
            uint z=0;
            while(temp[z] == ' ' || temp[z] == '\t') z++;
            n=0;
            i++;
            rtemp=resf.exists(temp+z);
            if(!rtemp) {
               strcpy(err_sec,s->get_name());
               strcpy(err_key,"resource");
               strcpy(err_msg, temp+z);
               strcat(err_msg,": Resource does not exist!");
               return ERROR;  
            } 
            res[cur_res++]=resf.get_index(rtemp->get_name());
         }
      }
      temp[n]='\0';
      n--;
      while(temp[n] == ',' || temp[n]==' ' || temp[n]=='\t') temp[n--]='\0';
      uint z=0;
      while(temp[z] == ' ' || temp[z] == '\t') z++;
      rtemp=resf.exists(temp+z);
      if(!rtemp) {
         strcpy(err_sec,s->get_name());
         strcpy(err_key,"resource");
         strcpy(err_msg, temp+z);
         strcat(err_msg,": Resource does not exist!");
         return ERROR;  
      } 
      res[cur_res++]=resf.get_index(rtemp->get_name());
   }

   // load textures
   str=s->get_string("texture", 0);
   char file[255];
   strcpy(file, g_dirname);
   strcat(file, SSEPERATOR);
   strcat(file, PICS_DIR);
   if(str) {
   strcat(file, str);
   } else {
      strcat(file, s->get_name());
      strcat(file, "_??.bmp");
   }
   
   uint nidx=strlen(file);
   while(!(file[nidx]=='?' && file[nidx+1]=='?') && nidx) nidx--;
   if(file[nidx]!='?' && file[nidx+1]!='?') {
      strcpy(err_sec, s->get_name());
      strcpy(err_key, "texture");
      strcpy(err_msg, "file names do not contain a ?? !");
      return ERROR; 
   } 

   char ten, one;
   tex=new Pic_Descr*[99];
   Pic_Descr* pic;
   for(ten='0'; ten<='9'; ten++) {
      file[nidx]=ten;
      for(one='0'; one<='9'; one++) {
         file[nidx+1]=one;
         pic=new Pic_Descr();
         if(pic->load(file)) {
            delete pic; 
            break;
         } else {
            if(pic->get_w() != TEXTURE_W && pic->get_h() != TEXTURE_H) {
               strcpy(err_sec, s->get_name());
               strcpy(err_key, "texture");
               sprintf(err_msg, "%s: texture has wrong size, must have %ix%i", file, TEXTURE_W, TEXTURE_H);
               delete pic;
               return ERROR;
            }

            ntextures++;
            tex[ntextures-1]=pic;
         }
      }
      if(one != ('9'+1)) break;
   }

   return OK;
}
 
int Terrain_Descr::write(Binary_file* f) {
   f->write(name, 30);
   f->write(&is, sizeof(uchar));
   f->write(&def_res, sizeof(uchar));
   f->write(&def_stock, sizeof(ushort));
   f->write(&minh, sizeof(uchar));
   f->write(&maxh, sizeof(uchar));
   f->write(&nres, sizeof(uchar));
   f->write(res, nres*sizeof(uchar));
   f->write(&ntextures, sizeof(ushort));
   uint i;
   for(i=0; i<ntextures; i++) {
      tex[i]->write(f);
   }
   
   return OK;
}
