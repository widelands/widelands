/*
 * Copyright (C) 2002 by Florian Bluemel, Holger Rapp
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

#if defined(WIN32) && defined(_MSC_VER)
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif


#include <stdio.h>
#include <string.h>
#include "tribedata.h"
#include "../src/myfile.h"
#include "profile.h"
#include "../src/graphic.h"
#include "../src/mydirent.h"
#include "../src/os.h"
#include "bob_descr.h"
#include "pic_descr.h"

struct TribeFileHeader {
   char magic[6];
   ushort version;
   char author[30];
   char name[30];
   char descr[1024];
   uchar checksum[16];
};

struct TribeFileNeedList {
   char magic1[6];
   ushort nbobs_needed;
   char* bobs_name;
   char magic2[6];
   ushort nres_needed;
   char* res_name;
};

struct TribeFileRegentDescr {
   char magic[6];
   char name[30];
   char big_picture[14000];
   char small_picture[6300];
};

  
struct TribeFileBobsDescr {
   char magic1[6];
   ushort ndbobs;
   DBobsDescr* dbobs;
   char magic2[6];
   ushort ngbobs;
   GBobsDescr* gbobs;
};

struct WareDescr {
   char name[30];
   ushort clrkey;
   ushort width;
   ushort height;
   char menu_pic_data[1152];
   char* pic_data;
};

struct TribeFileWaresDescr {
   char magic[6];
   ushort nwares;
   WareDescr* wares;
};

struct CostList {
   ushort nitems;
   ushort *list; // always : 2bytes index, 2bytes count-- next item
};

struct SoldierDescr {
   char name[30];
   ushort energy;
   ushort walking_speed;
   ushort width; 
   ushort height;
   ushort hsx;
   ushort hsy;
   ushort clrkey;
   char menu_pic_data[1152];
   BobDescr* bob_walk_n;
   BobDescr* bob_walk_ne;
   BobDescr* bob_walk_e;
   BobDescr* bob_walk_se;
   BobDescr* bob_walk_s;
   BobDescr* bob_walk_sw;
   BobDescr* bob_walk_w;
   BobDescr* bob_walk_nw;
   BobDescr* bob_attackl;
   BobDescr* bob_attackr;
   BobDescr* bob_evadel;
   BobDescr* bob_evader;
};

struct TribeFileSoldiersDescr {
   char magic[10];
   ushort nsoldiers;
   CostList needed_items;
   SoldierDescr* soldiers;
};

struct WorkerDescr {
   CostList needed_items;
   char name[30];
   ushort walking_speed;
   ushort width;
   ushort height;
   ushort hsx;
   ushort hsy;
   ushort clrkey;
   char menu_pic_data[1152];
   BobDescr* bob_walk_n;
   BobDescr* bob_walk_ne;
   BobDescr* bob_walk_e;
   BobDescr* bob_walk_se;
   BobDescr* bob_walk_s;
   BobDescr* bob_walk_sw;
   BobDescr* bob_walk_w;
   BobDescr* bob_walk_nw;
   
   BobDescr* bob_walk1_n;
   BobDescr* bob_walk1_ne;
   BobDescr* bob_walk1_e;
   BobDescr* bob_walk1_se;
   BobDescr* bob_walk1_s;
   BobDescr* bob_walk1_sw;
   BobDescr* bob_walk1_w;
   BobDescr* bob_walk1_nw;
   BobDescr* bob_work;
   BobDescr* bob_work1;
   BobDescr* bob_special;
   BobDescr* bob_special1;
 };
   
struct TribeFileWorkersDescr {
   char magic[8];
   ushort nworkers;
   WorkerDescr* workers;
};

struct BuildingDescr {
   char name[30];
   char category[30];
   ushort is_a;
   ushort type;
   ushort nfors;
   char* forlist;
   ushort build_time;
   CostList build_cost;
   ushort worker;
   ushort work_need_type;
   ushort nwork_needs;
   ushort* nwork_needs_list; // index, num, stock
   ushort nproducts;
   ushort* nproduct_list; // is_bob, index, num, probability
   ushort working_time;
   ushort idle_time;
   ushort order_worker;
   ushort working_area;
   ushort is_enabled;
   ushort see_area;
   BobDescr* bob_build;
   BobDescr* bob_idle;
   BobDescr* bob_working;
};

struct MilBuildingDescr {
   char name[30];
   char category[30];
   ushort is_a;
   ushort build_time;
   CostList build_cost;
   ushort nupgr_needs;
   ushort* upr_needs; // index, num, stock
   ushort nupgrade;
   ushort idle_time;
   ushort beds;
   ushort conquers;
   ushort is_enabled;
   ushort see_area;
   BobDescr* bob_build;
   BobDescr* bob_idle;
};

struct CannonDescr {
   char name[30];
   char category[30];
   ushort is_a;
   ushort build_time;
   CostList build_cost;
   ushort worker;
   ushort projectile_speed;
   ushort firing_type;
   ushort nfire_needs;
   short* fire_needs; // index, num, stock
   ushort idle_time;
   ushort see_area;
   ushort is_enabled;
   BobDescr* bob_build;
   BobDescr* bob_idle;
   BobDescr* bob_fire_n;
   BobDescr* bob_fire_ne;
   BobDescr* bob_fire_e;
   BobDescr* bob_fire_se;
   BobDescr* bob_fire_s;
   BobDescr* bob_fire_sw;
   BobDescr* bob_fire_w;
   BobDescr* bob_fire_nw;
};

struct TribeFileBuildingsDescr {
   char magic[10];
   ushort nbuildings;
   BuildingDescr* buildings;
   char magic1[8];
   ushort nmil_buildings;
   MilBuildingDescr* mil_buildings;
   char magic2[8];
   ushort ncannons;
   CannonDescr* cannons;
};

#define CONF_NAME		"conf"
#define WTF_EXT			".wtf"
#ifdef WIN32
#define SEPERATOR		'\\'
#define SSEPERATOR	"\\"
#else
#define SEPERATOR		'/'
#define SSEPERATOR	"/"
#endif
#define PICS_DIR     "pics"  SSEPERATOR
#define BOBS_DIR     "bobs"  SSEPERATOR
#define WARES_DIR    "wares" SSEPERATOR
#define SOLDIERS_DIR "soldiers" SSEPERATOR
#define WORKERS_DIR  "workers" SSEPERATOR
#define BUILDINGS_DIR "buildings" SSEPERATOR


#include <iostream.h>

void usage()
{
   cout << "Creates Widelands Tribe Files." << endl
      << "usage: wtfcreate <directory name>" << endl
      << endl 
      << "See Tribe FAQ for details of tribe creation" << endl;
}

void sec_missing(const char *filename, const char* sec) {
   cout << filename << ": mandatory section [" << sec << "] is missing!" << endl;
   exit(-1);
}

void key_missing(const char* filename, const char* sec, const char* key) {
   cout << filename << ": mandatory key \"" << key << "\" in section [" << sec <<  "] is missing!" << endl;
   exit(-1);
}

void zmem(void* mem, ulong size) {
   ulong i;
   for(i=0; i<size; i++) 
      ((char*) mem)[i]='\0';
}

void add_to_need_list(TribeFileNeedList* nlist, const char* name, bool is_bob) {
   
   if(is_bob) {
      nlist->nbobs_needed++;
      if(nlist->nbobs_needed==1) {
         nlist->bobs_name=(char*) malloc(30);
      } else {
         nlist->bobs_name=(char*) realloc(nlist->bobs_name, 30*nlist->nbobs_needed);
      }
      uint i=0;
      for(i=0; i<30; i++) 
         nlist->bobs_name[i+(nlist->nbobs_needed-1)*30]='\0';
      memcpy((nlist->bobs_name+(nlist->nbobs_needed-1)*30), name, 30 < strlen(name) ? 30 : strlen(name));
   } else {
      nlist->nres_needed++;
      if(nlist->nres_needed==1) {
         nlist->res_name=(char*) malloc(30);
      } else {
         nlist->res_name=(char*) realloc(nlist->res_name, 30*nlist->nres_needed);
      }
      uint i=0;
      for(i=0; i<30; i++)
         nlist->res_name[i+(nlist->nres_needed-1)*30]='\0';
      memcpy((nlist->res_name+(nlist->nres_needed-1)*30), name, 30 < strlen(name) ? 30 : strlen(name));
   }
}

uint load_bitmap(void* gdata, uint size, const char* filename, const char* dir, const char* subdir=0, ushort dw=0, ushort dh=0, ushort *w=0, ushort *h=0) {
   char *file;
   uint len;

   len=strlen(filename)+1;
   if(dir) len+=strlen(dir);
   if(subdir) len+=strlen(dir);
   
   file = new char[len];
   file[0]='\0';
   
   if(dir) strcat(file, dir);
   if(subdir) strcat(file, subdir);
   strcat(file, filename);
   
   // check for files existence
   Bitmap_Descr p;
   if(p.load(file)) {
      cout << file << ": File not found or read error!" << endl;
      exit(-1);
   }
   // check for dw && dh
   if(dw && p.get_w() != dw) {
      cout << file << ": The picture must have a width of " << dw << " pixels, but got " << p.get_w() << endl;
      exit(-1);
   }
   if(dh && p.get_h() != dh) {
      cout << file << ": The picture must have a height of " << dw << " pixels, but got " << p.get_h() << endl;
      exit(-1);
   }
   
   // check for data size
   if(!p.construct_description(gdata, size)) {
      cout << file << ": construct_description failed! probably internal error or bug!" << endl;
      exit(-1);
   }

   if(w && h) {
      *w=p.get_w();
      *h=p.get_h();
   }

   delete[] file;


   return 0;
}

uint load_bob_data(BobDescr** gbob, const char* pfile_names, const char* dirname, const char* subdir, ushort clrkey, ushort shadowclr, ushort* w, ushort *h, bool is_mandatory=true) {
   *gbob=(BobDescr*) malloc(sizeof(BobDescr));

   BobDescr* bob=*gbob;
   bob->npics=0;
  
   uint len=strlen(dirname)+strlen(pfile_names)+1;
   if(subdir) len+=strlen(subdir)+1;
   char* buf = new char[len];
   strcpy(buf, dirname);
   if(subdir) { 
      strcat(buf, subdir); 
      if(buf[strlen(buf)-1]!= SEPERATOR) {
         buf[strlen(buf)+1]='\0'; 
         buf[strlen(buf)]=SEPERATOR; 
      }
   }
   strcat(buf, pfile_names);

   uint nidx=strlen(buf);
   while(!(buf[nidx]=='?' && buf[nidx+1]=='?') && nidx) nidx--;
   if(buf[nidx]!='?' && buf[nidx+1]!='?') {
      cout << pfile_names << ": Err in file names: don't contain \'\?\?\'!" << endl;
      exit(-1);
   }

   char one, ten;
   Bob_Data_Pic_Descr* p;
   for(ten='0'; ten<='9'; ten++) {
      buf[nidx]=ten;
      for(one='0'; one<='9'; one++) {
         buf[nidx+1]=one;
         
         // Handle this picture!
         p=new Bob_Data_Pic_Descr();
         if(p->load(buf)) {
            delete p;
            break; // file not found or some error
         }
         
         if(!(*w) && !(*h)) {
            *w=p->get_w();
            *h=p->get_h();
         } else {
            if(*w!=p->get_w() || *h!=p->get_h()) {
               cout << buf << ": This file doesn't match the size " << *w << "x" << *h << " of the former pics!" << endl;
               exit(-1);
            }
         }
         p->set_clrkey(clrkey);
         p->set_shadowclr(shadowclr);
         p->set_plclr(DARKEST_PL_CLR, DARK_PL_CLR, MIDDLE_PL_CLR, LIGHT_PL_CLR);
         p->construct_description(bob);

         delete p;
      }
      if((one-1)!='9') break;
   }

   if(!bob->npics && is_mandatory) {
      cout << pfile_names << ": No pictures for this bob, check file names and reading rights!" << endl;
      exit(-1);
   }
   
   delete[] buf;
   
   return 0;
}

int parse_root_conf(const char* dirname, ushort* pdef_clrkey, ushort* pdef_shadowclr, TribeFileHeader* pheader, TribeFileRegentDescr* pregent) {
   char* filename=new char[strlen(dirname)+strlen(CONF_NAME)];
   int retval=0;

   strcpy(filename, dirname);
   strcat(filename, CONF_NAME);

   Profile p(cout, filename);
  
   Section *s=p.get_section("defaults");
   if(s) {
      // care for defaults section
      *pdef_clrkey=pack_rgb(s->get_int("clrkey_r", 255), s->get_int("clrkey_G", 255), s->get_int("clrkey_b", 255));
      *pdef_shadowclr=pack_rgb(s->get_int("shadowclr_r", 255), s->get_int("shadowclr_G", 255), s->get_int("shadowclr_b", 255));
   }

   s=p.get_section("tribe");
   if(!s) {
      sec_missing(filename, "tribe");
      retval=-1;
   } else {
      // First, init the header
      memcpy(pheader->magic, "WLtf\0\0", sizeof(pheader->magic));
      pheader->version=WLTF_VERSION;
      zmem(pheader->author, sizeof(pheader->author));
      zmem(pheader->name, sizeof(pheader->name));
      zmem(pheader->descr, sizeof(pheader->descr));
      zmem(pheader->checksum, sizeof(pheader->checksum));
    
      const char* str;
      str=s->get_string("author","");
      if(str[0]=='\0') { key_missing(filename, "tribe", "author"); retval=-1; }
      else strncpy(pheader->author, str, sizeof(pheader->author)-1);
      str=s->get_string("name","");
      if(str[0]=='\0') { key_missing(filename, "tribe", "name"); retval=-1; }
      else strncpy(pheader->name, str, sizeof(pheader->name)-1);
      str=s->get_string("descr","");
      if(str[0]=='\0') { key_missing(filename, "tribe", "descr"); retval=-1; }
      else strncpy(pheader->descr, str, sizeof(pheader->descr)-1);
      
   }
  
   s=p.get_section("regent");
   if(!s) {
      sec_missing(filename, "regent");
      retval=-1;
   } else {
      // First init the regent description
      memcpy(pregent->magic, "Regent\0\0", sizeof(pregent->magic));
      zmem(pregent->name, sizeof(pregent->name));
      zmem(pregent->big_picture, sizeof(pregent->big_picture));
      zmem(pregent->small_picture, sizeof(pregent->small_picture));

      const char* str;
      str=s->get_string("name","");
      if(str[0]=='\0') { key_missing(filename, "regent", "name"); retval=-1; }
      else strncpy(pregent->name, str, sizeof(pregent->name)-1);

      // Load big pic
      str=s->get_string("pic_big","reg_big.bmp");
      load_bitmap(pregent->big_picture, sizeof(pregent->big_picture), str, dirname, PICS_DIR, 70,100);

      // Load small pic
      str=s->get_string("pic_small","reg_small.bmp");
      load_bitmap(pregent->small_picture, sizeof(pregent->small_picture), str, dirname, PICS_DIR, 45,70);
   }

   delete[] filename;
   return retval;
}
 
int parse_wares_conf(const char* dirname, ushort def_clrkey, TribeFileWaresDescr* wares) {
   char* file=new char[strlen(dirname)+strlen(WARES_DIR)+strlen(CONF_NAME)+1];
   char* dir=new char[strlen(dirname)+strlen(WARES_DIR)];
   
   strcpy(dir, dirname);
   strcat(dir, WARES_DIR);
   strcpy(file, dir);
   strcat(file, CONF_NAME);

   Profile p(cout, file);
   Section* s;
   
   char buf[] ="ware_00";

   wares->wares=(WareDescr*) malloc(sizeof(WareDescr)*99);
      
   uint one, ten;
   WareDescr* w;
   uchar r,g,b;
   const char* str;
   for(ten='0'; ten<='9'; ten++) {
         buf[5]=ten;
      for(one='0'; one<='9'; one++) {
         buf[6]=one;
         s=p.get_section(buf);
         if(!s) break;
         wares->nwares++;
         w=&wares->wares[wares->nwares-1];
         zmem(w->name, sizeof(w->name));
         w->clrkey=0;
         w->width=0;
         w->height=0;
         zmem(w->menu_pic_data, sizeof(w->menu_pic_data));
         w->pic_data=0;
         
         // name
         str=s->get_string("name","");
         if(str[0]=='\0') key_missing(file, buf, "name");
         memcpy(w->name, str, strlen(str) < sizeof(w->name) ? strlen(str) : sizeof(w->name));
         
         // clrkey
         unpack_rgb(def_clrkey, &r, &g, &b); 
         r=s->get_int("clrkey_r", r);
         g=s->get_int("clrkey_g", g);
         b=s->get_int("clrkey_b", b);
         w->clrkey=pack_rgb(r, g, b);
      
         // menupic
         char def_name[40];
         strcpy(def_name, w->name);
         strcat(def_name, "_m.bmp");
         str=s->get_string("menupic", def_name);
         load_bitmap(w->menu_pic_data, sizeof(w->menu_pic_data), str, dir, PICS_DIR, 24,24);

         // flag pic
         strcpy(def_name, w->name);
         strcat(def_name, ".bmp");
         str=s->get_string("pic", def_name);
         w->pic_data=(char*) malloc(100*100*2); // ought to be enough
         load_bitmap(w->pic_data, 100*100*2, str, dir, PICS_DIR, 0, 0, &w->width, &w->height);
         w->pic_data=(char*) realloc(w->pic_data, w->width*w->height*2);
      }
      if(one-1!='9') break;
   }
      
   wares->wares=(WareDescr*) realloc(wares->wares, sizeof(WareDescr)*wares->nwares);
   delete[] file;
   delete[] dir;
   return 0;
}

int parse_bobs_conf(const char* dirname, ushort def_clrkey, ushort def_shadowclr, TribeFileBobsDescr* pbobs, TribeFileNeedList* nlist) {
   char *dir= new char[strlen(dirname)+strlen(BOBS_DIR)+1];

   strcpy(dir, dirname);
   strcat(dir, BOBS_DIR);
   
   // Enumerate the subdirs
   DIR* d=opendir(dir);
   if(!d) {
      cout << dir << ": dir not found or read error!" << endl;
   }
   struct dirent *file;
   Profile* p;
   Section* s;
   const char* type; 
   
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
      type=s->get_string("type", "");
      
      if(!strcmp(type, "diminishing")) {
         // here we go, handle diminishing bob
         pbobs->ndbobs++;
         if(pbobs->ndbobs==1) {
            pbobs->dbobs=(DBobsDescr*) malloc(sizeof(DBobsDescr));
         } else {
            pbobs->dbobs=(DBobsDescr*) realloc(pbobs->dbobs, sizeof(DBobsDescr)*pbobs->ndbobs);
         }

         DBobsDescr* bob=&pbobs->dbobs[pbobs->ndbobs-1];
         zmem(bob->name, sizeof(bob->name));
         zmem(bob->ends_in, sizeof(bob->ends_in));
         bob->hsx=0;
         bob->hsy=0;
         bob->stock=0;
         bob->width=0;
         bob->height=0;
         bob->bob_idle=0;
  
         memcpy(bob->name, file->d_name, strlen(file->d_name) < sizeof(bob->name) ? strlen(file->d_name) : sizeof(bob->name));
         int temp=s->get_int("hot_spot_x", -1);
         if(temp==-1) {
             key_missing(filename, "[NO__SEC]", "hot_spot_x");
         } 
         bob->hsx=temp;
         temp=s->get_int("hot_spot_y", -1);
         if(temp==-1) {
             key_missing(filename, "[NO__SEC]", "hot_spot_y");
         } 
         bob->hsy=temp;
         temp=s->get_int("stock", -1);
         if(temp==-1) {
            key_missing(filename, "[NO__SEC]", "stock");
         } 
         bob->stock=temp;
         const char* ends_in=s->get_string("ends_in", "");
         memcpy(bob->ends_in, ends_in, strlen(ends_in) < sizeof(bob->ends_in) ? strlen(ends_in) : sizeof(bob->ends_in));
         
         // check clrkey
         uchar  clrk_r, clrk_g, clrk_b;
         unpack_rgb(def_clrkey, &clrk_r, &clrk_g, &clrk_b); 
         clrk_r=s->get_int("clrkey_r", clrk_r);
         clrk_g=s->get_int("clrkey_g", clrk_g);
         clrk_b=s->get_int("clrkey_b", clrk_b);
         ushort clrkey=pack_rgb(clrk_r, clrk_g, clrk_b);
         
         // check shadowclr
         uchar  shad_r, shad_g, shad_b;
         unpack_rgb(def_shadowclr, &shad_r, &shad_g, &shad_b);
         shad_r=s->get_int("shadowclr_r", shad_r);
         shad_g=s->get_int("shadowclr_g", shad_g);
         shad_b=s->get_int("shadowclr_b", shad_b);
         ushort shadowclr=pack_rgb(shad_r, shad_g, shad_b);

         // construct bob data
         char *def_name= new char[strlen(file->d_name)+strlen("_??.bmp")+1];
         strcpy(def_name, file->d_name);
         strcat(def_name, "_??.bmp");
         const char* bob_name=s->get_string("idle_anim", def_name);
         char* subdir= new char[strlen(BOBS_DIR)+strlen(bob->name)+1];
         strcpy(subdir, BOBS_DIR);
         strcat(subdir, bob->name);
         load_bob_data(&bob->bob_idle, bob_name, dirname, subdir, clrkey, shadowclr, &bob->width, &bob->height);
         delete[] def_name;
         delete[] subdir;
         
         // check if hsx && hsy are inside picture
         if(bob->hsx>=bob->width) {
            cout << filename << ": hot_spot_x is bigger than bob width!" << endl;
            exit(-1);
         }
         if(bob->hsy>=bob->height) {
            cout << filename << ": hot_spot_y is bigger than bob height!" << endl;
            exit(-1);
         }

      } else if(!strcmp(type, "growing")) {
         pbobs->ngbobs++;
         if(pbobs->ngbobs==1) {
            pbobs->gbobs=(GBobsDescr*) malloc(sizeof(GBobsDescr));
         } else {
            pbobs->gbobs=(GBobsDescr*) realloc(pbobs->gbobs, sizeof(GBobsDescr)*pbobs->ngbobs);
         }
        
         GBobsDescr* bob=&pbobs->gbobs[pbobs->ngbobs-1];
         zmem(bob->name, sizeof(bob->name));
         zmem(bob->ends_in, sizeof(bob->ends_in));
         bob->hsx=0;
         bob->hsy=0;
         bob->width=0;
         bob->height=0;
         bob->growing_speed=0;
         bob->bob_growing=0;
        
         memcpy(bob->name, file->d_name, strlen(file->d_name) < sizeof(bob->name) ? strlen(file->d_name) : sizeof(bob->name));
         int temp=s->get_int("hot_spot_x", -1);
         if(temp==-1) {
             key_missing(filename, "[NO__SEC]", "hot_spot_x");
         } 
         bob->hsx=temp;
         temp=s->get_int("hot_spot_y", -1);
         if(temp==-1) {
             key_missing(filename, "[NO__SEC]", "hot_spot_y");
         } 
         bob->hsy=temp;
         temp=s->get_int("growing_speed", -1);
         if(temp==-1) {
            key_missing(filename, "[NO__SEC]", "growing_speed");
         } 
         bob->growing_speed=temp;
         const char* ends_in=s->get_string("ends_in", "");
         if(!strcmp(ends_in, "")) {
            key_missing(filename, "[NO_SEC]", "ends_in");
         }
         memcpy(bob->ends_in, ends_in, strlen(ends_in) < sizeof(bob->ends_in) ? strlen(ends_in) : sizeof(bob->ends_in));
         
         // check clrkey
         uchar  clrk_r, clrk_g, clrk_b;
         unpack_rgb(def_clrkey, &clrk_r, &clrk_g, &clrk_b); 
         clrk_r=s->get_int("clrkey_r", clrk_r);
         clrk_g=s->get_int("clrkey_g", clrk_g);
         clrk_b=s->get_int("clrkey_b", clrk_b);
         ushort clrkey=pack_rgb(clrk_r, clrk_g, clrk_b);
         
         // check shadowclr
         uchar  shad_r, shad_g, shad_b;
         unpack_rgb(def_shadowclr, &shad_r, &shad_g, &shad_b);
         shad_r=s->get_int("shadowclr_r", shad_r);
         shad_g=s->get_int("shadowclr_g", shad_g);
         shad_b=s->get_int("shadowclr_b", shad_b);
         ushort shadowclr=pack_rgb(shad_r, shad_g, shad_b);

         // construct bob data
         char *def_name= new char[strlen(file->d_name)+strlen("_??.bmp")+1];
         strcpy(def_name, file->d_name);
         strcat(def_name, "_??.bmp");
         const char* bob_name=s->get_string("anim_name", def_name);
         char* subdir= new char[strlen(BOBS_DIR)+strlen(bob->name)+1];
         strcpy(subdir, BOBS_DIR);
         strcat(subdir, bob->name);
         load_bob_data(&bob->bob_growing, bob_name, dirname, subdir, clrkey, shadowclr, &bob->width, &bob->height);
         delete[] def_name;
         delete[] subdir;
         
         // check if hsx && hsy are inside picture
         if(bob->hsx>=bob->width) {
            cout << filename << ": hot_spot_x is bigger than bob width!" << endl;
            exit(-1);
         }
         if(bob->hsy>=bob->height) {
            cout << filename << ": hot_spot_y is bigger than bob height!" << endl;
            exit(-1);
         }             
      } else {
         cout << filename<< ": type \"" << type << "\" is not a valid bob-type for tribe files!" << endl;
         exit(-1);
      }
      
      delete p;
      delete[] filename;
   }
 
   uint i, j;
   for(i=0; i<pbobs->ngbobs; i++) {
      for(j=0; j<pbobs->ndbobs; j++) {
         if(!strcasecmp(pbobs->dbobs[j].name, pbobs->gbobs[i].ends_in)) continue;
         
         add_to_need_list(nlist, pbobs->gbobs[i].ends_in, 1);
      }
   }
   for(i=0; i<pbobs->ndbobs; i++) {
      if(pbobs->dbobs[i].ends_in[0]=='\0') continue;
      for(j=0; j<pbobs->ndbobs; j++) {
         if(!strcasecmp(pbobs->dbobs[j].name, pbobs->dbobs[i].ends_in)) continue;

         add_to_need_list(nlist, pbobs->dbobs[i].ends_in, 1);
      }
   }

   closedir(d);
   delete[] dir;

   return 0;
}

int parse_cost(const char* str, CostList* l, TribeFileWaresDescr* wares) {
   if(str[0]=='0')  { l->nitems=(ushort) -1; return 0; } // not producable

   uint i=0;
   l->nitems=1;
   for(i=0; i<strlen(str); i++) 
      if(str[i]==',') l->nitems++;

   l->list=(ushort*) malloc(sizeof(ushort)*l->nitems*2);
   ushort n=0, z=0;
   char temp[50];
   for(i=0; i<l->nitems; i++) {
      while(str[n]!='*') {
         temp[z]=str[n];
         n++; z++;
      }
      temp[z]='\0'; 
      z=0;
      l->list[i*2+1]=atoi(temp);

      n++;
      while(str[n]==' ' || str[n]=='\t') n++;
      while(str[n]!=',' && str[n]!='\n' && str[n]!=' ' && str[n]!='\t' && str[n]!='\0') {
         temp[z]=str[n];
         n++; z++;
      }
      temp[z]='\0';
      z=0;
      n++;

      // name of ware is now in temp, get index
      for(z=0; z<wares->nwares; z++) {
         if(!strcasecmp(wares->wares[z].name, temp)) break;
      }
      if(z==wares->nwares) return -1;

      l->list[i*2]=z;
      z=0;
   }
   return 0;
}

int parse_soldiers_conf(const char* dirname, ushort def_clrkey, ushort def_shadowclr, TribeFileSoldiersDescr* soldiers, TribeFileWaresDescr* wares) {
   char* dir= new char[strlen(dirname)+strlen(SOLDIERS_DIR)+1];
   char* file= new char[strlen(dirname)+strlen(SOLDIERS_DIR)+strlen(CONF_NAME)+1];

   strcpy(dir, dirname);
   strcat(dir, SOLDIERS_DIR);
   strcpy(file, dir);
   strcat(file, CONF_NAME);

   Profile p(cout, file);
   char buf[]="soldier_00";

   Section *s;

   s=p.get_section(buf);
   if(!s) sec_missing(file, buf);

   // Parse needs command
   const char* str;
   str=s->get_string("needs", "");
   if(str[0]=='\0') key_missing(file, buf, "needs"); 
   if(parse_cost(str, &soldiers->needed_items, wares)) {
      cout << file << ": Some problems with the needs cmd \"" << str << "\". Check if you spelled the wares correctly, check for syntax errors!" << endl;
      exit(-1);
   }

   // set defaults
   ushort def_walkingspeed;
   s=p.get_section("defaults");
   def_walkingspeed=5;
   def_walkingspeed=s->get_int("walking_speed", def_walkingspeed);

   // Setup soldier mem
   soldiers->soldiers=(SoldierDescr*) malloc(sizeof(SoldierDescr)*99);

   uint one,ten;
   SoldierDescr* sol;
   char* bobsdir=new char[strlen(dir)+strlen(BOBS_DIR)+41];
   char* bobsfile=new char[strlen(dir)+strlen(BOBS_DIR)+strlen(CONF_NAME)+41];
   uchar  clrk_r, clrk_g, clrk_b;
   uchar  shad_r, shad_g, shad_b;
   ushort shadowclr;
   char def_name[100], bobname[40];
   for(ten='0'; ten<='9'; ten++) {
      buf[8]=ten;
      for(one='0'; one<='9'; one++) {
         buf[9]=one;
         s=p.get_section(buf);
         if(!s) break;

         soldiers->nsoldiers++;
         sol=&soldiers->soldiers[soldiers->nsoldiers-1];

         str=s->get_string("name", "");
         if(str[0]=='\0') key_missing(file, buf, "name");
         memcpy(sol->name, str, strlen(str) < 30 ? strlen(str) : 30);

         sol->walking_speed=s->get_int("walking_speed", def_walkingspeed);
         sol->energy=s->get_int("energy", 0);
         if(sol->energy==0) key_missing(file, buf, "energy");

         // Get BobFileName
         strcpy(bobname, sol->name);
         str=s->get_string("bob", bobname);

         // Care for bob. Get bob conf name
         strcpy(bobsdir, dir);
         strcat(bobsdir, BOBS_DIR);
         strcat(bobsdir, str);
         strcat(bobsdir, SSEPERATOR);
         strcpy(bobsfile, bobsdir);
         strcat(bobsfile, CONF_NAME);
         Profile pbob(cout, bobsfile, 1);
         Section* sbob=pbob.get_next_section(0);

         // Hot spots
         sol->hsx=sbob->get_int("hot_spot_x", 0);
         if(sol->hsx==0) key_missing(bobsfile, "__NO_SEC__", "hot_spot_x");
         sol->hsy=sbob->get_int("hot_spot_y", 0);
         if(sol->hsy==0) key_missing(bobsfile, "__NO_SEC__", "hot_spot_y");

         // check clrkey
         unpack_rgb(def_clrkey, &clrk_r, &clrk_g, &clrk_b); 
         clrk_r=s->get_int("clrkey_r", clrk_r);
         clrk_g=s->get_int("clrkey_g", clrk_g);
         clrk_b=s->get_int("clrkey_b", clrk_b);
         sol->clrkey=pack_rgb(clrk_r, clrk_g, clrk_b);

         // check shadowclr
         unpack_rgb(def_shadowclr, &shad_r, &shad_g, &shad_b);
         shad_r=s->get_int("shadowclr_r", shad_r);
         shad_g=s->get_int("shadowclr_g", shad_g);
         shad_b=s->get_int("shadowclr_b", shad_b);
         shadowclr=pack_rgb(shad_r, shad_g, shad_b);

         // menupic 
         strcpy(def_name, sol->name);
         strcat(def_name, "_mp.bmp");
         str=s->get_string("menupic", def_name);
         load_bitmap(sol->menu_pic_data, sizeof(sol->menu_pic_data), str, bobsdir, 0, 24,24);

         // walking bobs
         strcpy(def_name, sol->name);
         strcat(def_name, "_wn_??.bmp");
         str=s->get_string("walk_n", def_name);
         load_bob_data(&sol->bob_walk_n, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);
         
         strcpy(def_name, sol->name);
         strcat(def_name, "_wne_??.bmp");
         str=s->get_string("walk_ne", def_name);
         load_bob_data(&sol->bob_walk_ne, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);

         strcpy(def_name, sol->name);
         strcat(def_name, "_we_??.bmp");
         str=s->get_string("walk_e", def_name);
         load_bob_data(&sol->bob_walk_e, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);

         strcpy(def_name, sol->name);
         strcat(def_name, "_wse_??.bmp");
         str=s->get_string("walk_se", def_name);
         load_bob_data(&sol->bob_walk_se, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);

         strcpy(def_name, sol->name);
         strcat(def_name, "_ws_??.bmp");
         str=s->get_string("walk_s", def_name);
         load_bob_data(&sol->bob_walk_s, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);

         strcpy(def_name, sol->name);
         strcat(def_name, "_wsw_??.bmp");
         str=s->get_string("walk_sw", def_name);
         load_bob_data(&sol->bob_walk_sw, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);

         strcpy(def_name, sol->name);
         strcat(def_name, "_ww_??.bmp");
         str=s->get_string("walk_w", def_name);
         load_bob_data(&sol->bob_walk_w, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);

         strcpy(def_name, sol->name);
         strcat(def_name, "_wnw_??.bmp");
         str=s->get_string("walk_nw", def_name);
         load_bob_data(&sol->bob_walk_nw, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);

         // fighting!
         strcpy(def_name, sol->name);
         strcat(def_name, "_work_??.bmp");
         str=s->get_string("working_anim", def_name);
         load_bob_data(&sol->bob_attackl, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);

         strcpy(def_name, sol->name);
         strcat(def_name, "_work1_??.bmp");
         str=s->get_string("working_anim1", def_name);
         load_bob_data(&sol->bob_attackr, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);

         // evade
         strcpy(def_name, sol->name);
         strcat(def_name, "_special_??.bmp");
         str=s->get_string("special_anim", def_name);
         load_bob_data(&sol->bob_evadel, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);

         strcpy(def_name, sol->name);
         strcat(def_name, "_special1_??.bmp");
         str=s->get_string("special_anim1", def_name);
         load_bob_data(&sol->bob_evader, str, bobsdir, 0, sol->clrkey, shadowclr, &sol->width, &sol->height);
      }
      if(one-1!='9') break;
   }
   delete[] bobsdir;
   delete[] bobsfile;

   soldiers->soldiers=(SoldierDescr*) realloc(soldiers->soldiers, sizeof(SoldierDescr)*soldiers->nsoldiers);

   delete[] file;
   delete[] dir;
   return 0;

}

int construct_worker(const char* file, const char* dir, WorkerDescr* w, Section* s, ushort def_walkingspeed, ushort def_clrkey, ushort def_shadowclr, TribeFileWaresDescr* wares, bool vneeds, bool nwork, bool nwork1, bool nwalk1, bool nspecial, bool nspecial1, bool vwork, bool vwork1, bool vwalk1, bool vspecial, bool vspecial1) {
   zmem(w->name, sizeof(w->name));
   w->needed_items.nitems=0;
   w->walking_speed=0;
   w->width=0;
   w->height=0;
   w->hsx=0;
   w->hsy=0;
   w->clrkey=0;
   w->bob_walk_n=0;
   w->bob_walk_ne=0;
   w->bob_walk_e=0;
   w->bob_walk_se=0;
   w->bob_walk_s=0;
   w->bob_walk_sw=0;
   w->bob_walk_w=0;
   w->bob_walk_nw=0;
   w->bob_walk1_n=0;
   w->bob_walk1_ne=0;
   w->bob_walk1_e=0;
   w->bob_walk1_se=0;
   w->bob_walk1_s=0;
   w->bob_walk1_sw=0;
   w->bob_walk1_w=0;
   w->bob_walk1_nw=0;
   w->bob_work=0;
   w->bob_work1=0;
   w->bob_special=0;
   w->bob_special1=0;

   const char* str;
   if(vneeds) {
      // Parse needs command
      str=s->get_string("needs", "");
      if(str[0]=='\0') key_missing(file, s->get_name(), "needs"); 
      if(parse_cost(str, &w->needed_items, wares)) {
         cout << file << ": Some problems with the needs cmd \"" << str << "\". Check if you spelled the wares correctly, check for syntax errors!" << endl;
         exit(-1);
      }
   }

   str=s->get_string("name", "");
   if(str[0]=='\0') key_missing(file, s->get_name(), "name");
   memcpy(w->name, str, strlen(str) < 30 ? strlen(str) : 30);

   w->walking_speed=s->get_int("walking_speed", def_walkingspeed);

   char* bobsdir=new char[strlen(dir)+strlen(BOBS_DIR)+41];
   char* bobsfile=new char[strlen(dir)+strlen(BOBS_DIR)+strlen(CONF_NAME)+41];
   uchar  clrk_r, clrk_g, clrk_b;
   uchar  shad_r, shad_g, shad_b;
   ushort shadowclr;
   char def_name[100], bobname[40];

   // Get BobFileName
   strcpy(bobname, w->name);
   str=s->get_string("bob", bobname);

   // Care for bob. Get bob conf name
   strcpy(bobsdir, dir);
   strcat(bobsdir, BOBS_DIR);
   strcat(bobsdir, str);
   strcat(bobsdir, SSEPERATOR);
   strcpy(bobsfile, bobsdir);
   strcat(bobsfile, CONF_NAME);
   Profile pbob(cout, bobsfile, 1);
   Section* sbob=pbob.get_next_section(0);

   // Hot spots
   w->hsx=sbob->get_int("hot_spot_x", 0);
   if(w->hsx==0) key_missing(bobsfile, "__NO_SEC__", "hot_spot_x");
   w->hsy=sbob->get_int("hot_spot_y", 0);
   if(w->hsy==0) key_missing(bobsfile, "__NO_SEC__", "hot_spot_y");

   // check clrkey
   unpack_rgb(def_clrkey, &clrk_r, &clrk_g, &clrk_b); 
   clrk_r=s->get_int("clrkey_r", clrk_r);
   clrk_g=s->get_int("clrkey_g", clrk_g);
   clrk_b=s->get_int("clrkey_b", clrk_b);
   w->clrkey=pack_rgb(clrk_r, clrk_g, clrk_b);

   // check shadowclr
   unpack_rgb(def_shadowclr, &shad_r, &shad_g, &shad_b);
   shad_r=s->get_int("shadowclr_r", shad_r);
   shad_g=s->get_int("shadowclr_g", shad_g);
   shad_b=s->get_int("shadowclr_b", shad_b);
   shadowclr=pack_rgb(shad_r, shad_g, shad_b);

   // menupic 
   strcpy(def_name, w->name);
   strcat(def_name, "_mp.bmp");
   str=s->get_string("menupic", def_name);
   load_bitmap(w->menu_pic_data, sizeof(w->menu_pic_data), str, bobsdir, 0, 24,24);

   // walking bobs
   strcpy(def_name, w->name);
   strcat(def_name, "_wn_??.bmp");
   str=s->get_string("walk_n", def_name);
   load_bob_data(&w->bob_walk_n, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height);

   strcpy(def_name, w->name);
   strcat(def_name, "_wne_??.bmp");
   str=s->get_string("walk_ne", def_name);
   load_bob_data(&w->bob_walk_ne, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height);

   strcpy(def_name, w->name);
   strcat(def_name, "_we_??.bmp");
   str=s->get_string("walk_e", def_name);
   load_bob_data(&w->bob_walk_e, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height);

   strcpy(def_name, w->name);
   strcat(def_name, "_wse_??.bmp");
   str=s->get_string("walk_se", def_name);
   load_bob_data(&w->bob_walk_se, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height);

   strcpy(def_name, w->name);
   strcat(def_name, "_ws_??.bmp");
   str=s->get_string("walk_s", def_name);
   load_bob_data(&w->bob_walk_s, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height);

   strcpy(def_name, w->name);
   strcat(def_name, "_wsw_??.bmp");
   str=s->get_string("walk_sw", def_name);
   load_bob_data(&w->bob_walk_sw, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height);

   strcpy(def_name, w->name);
   strcat(def_name, "_ww_??.bmp");
   str=s->get_string("walk_w", def_name);
   load_bob_data(&w->bob_walk_w, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height);

   strcpy(def_name, w->name);
   strcat(def_name, "_wnw_??.bmp");
   str=s->get_string("walk_nw", def_name);
   load_bob_data(&w->bob_walk_nw, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height);

   // walking bobs1
   if(vwalk1) {
      strcpy(def_name, w->name);
      strcat(def_name, "_wn1_??.bmp");
      str=s->get_string("walk1_n", def_name);
      load_bob_data(&w->bob_walk1_n, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nwalk1);

      strcpy(def_name, w->name);
      strcat(def_name, "_wne1_??.bmp");
      str=s->get_string("walk1_ne", def_name);
      load_bob_data(&w->bob_walk1_ne, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nwalk1);

      strcpy(def_name, w->name);
      strcat(def_name, "_we1_??.bmp");
      str=s->get_string("walk1_e", def_name);
      load_bob_data(&w->bob_walk1_e, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nwalk1);

      strcpy(def_name, w->name);
      strcat(def_name, "_wse1_??.bmp");
      str=s->get_string("walk1_se", def_name);
      load_bob_data(&w->bob_walk1_se, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nwalk1);

      strcpy(def_name, w->name);
      strcat(def_name, "_ws1_??.bmp");
      str=s->get_string("walk1_s", def_name);
      load_bob_data(&w->bob_walk1_s, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nwalk1);

      strcpy(def_name, w->name);
      strcat(def_name, "_wsw1_??.bmp");
      str=s->get_string("walk1_sw", def_name);
      load_bob_data(&w->bob_walk1_sw, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nwalk1);

      strcpy(def_name, w->name);
      strcat(def_name, "_ww1_??.bmp");
      str=s->get_string("walk1_w", def_name);
      load_bob_data(&w->bob_walk1_w, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nwalk1);

      strcpy(def_name, w->name);
      strcat(def_name, "_wnw1_??.bmp");
      str=s->get_string("walk1_nw", def_name);
      load_bob_data(&w->bob_walk1_nw, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nwalk1);
   }

   // work!
   if(vwork) {
      strcpy(def_name, w->name);
      strcat(def_name, "_work_??.bmp");
      str=s->get_string("working_anim", def_name);
      load_bob_data(&w->bob_work, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nwork);
   }

   // work1
   if(vwork1) {
      strcpy(def_name, w->name);
      strcat(def_name, "_work1_??.bmp");
      str=s->get_string("working_anim1", def_name);
      load_bob_data(&w->bob_work1, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nwork1);
   }

   // special
   if(vspecial) {
      strcpy(def_name, w->name);
      strcat(def_name, "_special_??.bmp");
      str=s->get_string("special_anim", def_name);
      load_bob_data(&w->bob_special, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nspecial);
   }

   // special1
   if(vspecial1) {
      strcpy(def_name, w->name);
      strcat(def_name, "_special1_??.bmp");
      str=s->get_string("special_anim1", def_name);
      load_bob_data(&w->bob_special1, str, bobsdir, 0, w->clrkey, shadowclr, &w->width, &w->height, nspecial1);
   }

   delete[] bobsdir;
   delete[] bobsfile;

   return 0;
}

int parse_workers_conf(const char* dirname, ushort def_clrkey, ushort def_shadowclr, TribeFileWorkersDescr* workers, TribeFileWaresDescr* wares) {
   char* dir= new char[strlen(dirname)+strlen(WORKERS_DIR)+1];
   char* file= new char[strlen(dirname)+strlen(WORKERS_DIR)+strlen(CONF_NAME)+1];

   strcpy(dir, dirname);
   strcat(dir, WORKERS_DIR);
   strcpy(file, dir);
   strcat(file, CONF_NAME);

   Profile p(cout, file);
   char buf[]="worker_00";

   Section *s;

   s=p.get_section(buf);
   if(!s) sec_missing(file, buf);

   // set defaults
   ushort def_walkingspeed;
   s=p.get_section("defaults");
   def_walkingspeed=5;
   def_walkingspeed=s->get_int("walking_speed", def_walkingspeed);

   // Setup mem
   workers->workers=(WorkerDescr*) malloc(sizeof(WorkerDescr)*99);

   // defaults: carrier
   s=p.get_section("carrier");
   if(!s) sec_missing(file, "carrier");
   construct_worker(file, dir, &workers->workers[0], s, def_walkingspeed, def_clrkey, def_shadowclr, wares, 0, 
                                             0, 0, 1, 0, 0, 
                                             0, 0, 1, 1, 1);
   workers->nworkers++;
  
   // defaults: builder
   s=p.get_section("builder");
   if(!s) sec_missing(file, "builder");
   construct_worker(file, dir, &workers->workers[1], s, def_walkingspeed, def_clrkey, def_shadowclr, wares, 1, 
                                             1, 1, 0, 0, 0, 
                                             1, 1, 0, 0, 0);
   workers->nworkers++;

   // defaults: planer
   s=p.get_section("planer");
   if(!s) sec_missing(file, "planer");
   construct_worker(file, dir, &workers->workers[2], s, def_walkingspeed, def_clrkey, def_shadowclr, wares, 1, 
                                             1, 0, 0, 0, 0, 
                                             1, 0, 0, 0, 0);
   workers->nworkers++;

   // defaults: explorer
   s=p.get_section("explorer");
   if(!s) sec_missing(file, "explorer");
   construct_worker(file, dir, &workers->workers[3], s, def_walkingspeed, def_clrkey, def_shadowclr, wares, 1, 
                                             0, 0, 0, 0, 0, 
                                             0, 0, 0, 0, 0);
   workers->nworkers++;

   // defaults: geologist
   s=p.get_section("geologist");
   if(!s) sec_missing(file, "geologist");
   construct_worker(file, dir, &workers->workers[4], s, def_walkingspeed, def_clrkey, def_shadowclr, wares, 1, 
                                             1, 1, 0, 0, 0, 
                                             1, 1, 0, 0, 0);
   workers->nworkers++;

   // optional defaults: carrier_1
   s=p.get_section("carrier_1");
   if(s) construct_worker(file, dir, &workers->workers[5], s, def_walkingspeed, def_clrkey, def_shadowclr, wares, 1, 
                                             0, 0, 1, 0, 0, 
                                             0, 0, 1, 1, 1);
   workers->nworkers++;

   // optional defaults: carrier_2
   s=p.get_section("carrier_2");
   if(s) construct_worker(file, dir, &workers->workers[6], s, def_walkingspeed, def_clrkey, def_shadowclr, wares, 1, 
                                             0, 0, 1, 0, 0, 
                                             0, 0, 1, 1, 1);
   workers->nworkers++;

   char one, ten;
   for(ten='0'; ten<='9'; ten++) {
      buf[7]=ten;
      for(one='0'; one<='9'; one++) {
         buf[8]=one;
            s=p.get_section(buf);
            if(!s) break;
            workers->nworkers++;
            construct_worker(file, dir, &workers->workers[workers->nworkers-1], s, def_walkingspeed, def_clrkey, def_shadowclr, wares, 1,
                  0, 0, 0, 0, 0,
                  1, 1, 1, 0, 0);
      }
      if(one-1!='9') break;
   }

   workers->workers=(WorkerDescr*) realloc(workers->workers, sizeof(WorkerDescr)*workers->nworkers);

   delete[] file;
   delete[] dir;
   return 0;

}

void parse_building(const char* file, Section* s, ushort def_clrkey, ushort def_shadowclr, ushort def_see, ushort def_stock, const char* def_cat, TribeFileBobsDescr* bobs, TribeFileWaresDescr* wares, TribeFileWorkersDescr* workers, BuildingDescr* b, TribeFileNeedList* nlist, bool nbuild, bool ntype, bool nis_a=true) {
   
   zmem(b->name, sizeof(b->name));
   zmem(b->category, sizeof(b->category));
   b->is_a=0;
   b->type=0;
   b->nfors=0;
   b->forlist=0;
   b->build_time=0;
   b->build_cost.nitems=0;
   b->worker=0;
   b->work_need_type=0;
   b->nwork_needs=0;
   b->nwork_needs_list=0; 
   b->nproducts=0;
   b->nproduct_list=0; 
   b->working_time=0;
   b->idle_time=0;
   b->order_worker=0;
   b->working_area=0;
   b->is_enabled=0;
   b->working_area=0;
   b->see_area=0;
   b->bob_build=0;
   b->bob_idle=0;
   b->bob_working=0;

   const char *str;
   str=s->get_string("name", "");
   if(str[0]=='\0') key_missing(file, s->get_name(), "name");
   memcpy(b->name, str, strlen(str) < 30 ? strlen(str) : 30) ;

   str=s->get_string("category", def_cat);
   memcpy(b->category, str, strlen(str) < 30 ? strlen(str) : 30) ;

   b->see_area=s->get_int("see_area", def_see);
   
   if(nbuild) {
      if(nis_a) {
         str=s->get_string("is_a", "");
         if(str[0]=='\0') key_missing(file, s->get_name(), "is_a");

         if(!strcasecmp(str, "small")) b->is_a=1;
         else if(!strcasecmp(str, "medium")) b->is_a=2;
         else if(!strcasecmp(str, "big")) b->is_a=3;
         else if(!strcasecmp(str, "mining")) b->is_a=4;
         else {
            cout << file << ": \"" << str << "\" is not a valid is_a value! (either small, medium, big, mining)" << endl;
            exit(-1);
         }
      }
      b->build_time=s->get_int("build_time", 0);
      if(b->build_time==0) key_missing(file, s->get_name(), "build_time");
      
      str=s->get_string("build_cost", "");
      if(str[0]=='\0') key_missing(file, s->get_name(), "build_cost");
   }

   if(ntype) {
      str=s->get_string("type","");
      if(str[0]=='\0') key_missing(file, s->get_name(), "type");

      if(!strcasecmp(str, "dig")) b->type=0;
      else if(!strcasecmp(str, "search")) b->type=1;
      else if(!strcasecmp(str, "plant")) b->type=2;
      else if(!strcasecmp(str, "grow")) b->type=3;
      else if(!strcasecmp(str, "sit")) b->type=4;
      else if(!strcasecmp(str, "science")) b->type=5;
      else {
         cout << file << ": \"" << str << "\" is not a valid type value! (dig, search, plant, grow, sit, science)" << endl;
         exit(-1);
      }

      if(b->type==1 || b->type==2 || b->type==3) {
         // for is a bobs list
         str=s->get_string("for", "");
         if(str[0]=='\0') key_missing(file, s->get_name(), "for");
         char *buf= new char[strlen(str)+1];
         strcpy(buf, str);
         uint nc=1;
         uint n=0;
         while(n<strlen(str)) {
            if(str[n]==',') { 
               nc++; 
               buf[n]='\0'; 
               n++;
               while(buf[n]=='\t' || buf[n]==' ' || buf[n]=='\n') { 
                  buf[n]='\0'; n++; 
               }
               continue;
            }
            n++;
         }
         n=0; 
         b->nfors=nc;
         b->forlist=(char*) malloc(30*nc);
         zmem(b->forlist, 30*nc);
         uint z=0;
         for(n=0; n<strlen(str); n++) {
            memcpy(b->forlist+z*30, &buf[n], strlen(&buf[n]) < 30 ?  strlen(&buf[n]) : 30);
            z++;
            while(buf[n]!='\0') n++;
            while(buf[n]=='\0') n++;
            n--;
         }
         
         for(z=0; z<b->nfors; z++) {
            for(n=0; n<bobs->ngbobs; n++) {
               if(!strcasecmp(bobs->gbobs[n].name, b->forlist+z*30)) break;
            }
            if(n!=bobs->ngbobs) break;
            for(n=0; n<bobs->ndbobs; n++) {
               if(!strcasecmp(bobs->dbobs[n].name, b->forlist+z*30)) break;
            }
            if(n!=bobs->ndbobs) break;
            add_to_need_list(nlist, b->forlist+z*30, 1);
            delete[] buf;
         }

         b->working_area=s->get_int("working_area", 0);
         if(!b->working_area) key_missing(file, s->get_name(), "working_area");

      } else if(b->type==0) {
         // for is ONE resource
         str=s->get_string("for", "");
         if(str[0]=='\0') key_missing(file, s->get_name(), "for");
         b->nfors=1;
         b->forlist=(char*) malloc(30);
         zmem(b->forlist, 30);
         memcpy(b->forlist, str, strlen(str) < 30 ?  strlen(str) : 30);
         add_to_need_list(nlist, b->forlist, 0);
      }
   }

   

}

int parse_buildings(const char* file, Profile* p, ushort def_clrkey, ushort def_shadowclr, TribeFileBobsDescr* bobs, TribeFileWaresDescr* wares, TribeFileWorkersDescr* workers, TribeFileBuildingsDescr* buildings, TribeFileNeedList* nlist) {
   ushort def_see=4;
   ushort def_stock=8;
   const char rdef_cat[]="Misc";
   const char* def_cat;

   // Setting defaults
   Section* s=p->get_section("defaults");
   if(s) {
      def_cat=s->get_string("category", rdef_cat);
      def_see=s->get_int("see_area", def_see);
      def_stock=s->get_int("stock", def_stock);
   }
     
   buildings->nbuildings=0;
   buildings->buildings=(BuildingDescr*) malloc(sizeof(BuildingDescr)*99);
   
   // default buildings
   s=p->get_section("hq");
   if(!s) sec_missing(file, "hq");
   parse_building(file, s, def_clrkey, def_shadowclr, def_see, def_stock, def_cat, bobs, wares, workers, &buildings->buildings[0], nlist, 0,0);
   buildings->nbuildings++;
   
   s=p->get_section("store");
   if(!s) sec_missing(file, "store");
   parse_building(file, s, def_clrkey, def_shadowclr, def_see, def_stock, def_cat, bobs, wares, workers, &buildings->buildings[1], nlist, 1, 0);
   buildings->nbuildings++;

   s=p->get_section("dockyard");
   if(!s) sec_missing(file, "dockyard");
   parse_building(file, s, def_clrkey, def_shadowclr, def_see, def_stock, def_cat, bobs, wares, workers, &buildings->buildings[2], nlist, 1, 0);
   buildings->buildings[3].type=1;
   buildings->nbuildings++;
  
   s=p->get_section("port");
   if(!s) sec_missing(file, "port");
   parse_building(file, s, def_clrkey, def_shadowclr, def_see, def_stock, def_cat, bobs, wares, workers, &buildings->buildings[3], nlist, 1, 0, 0);
   buildings->buildings[3].is_a=5;
   buildings->nbuildings++;
  
   uint one, ten;
   char buf[]="building_00";
   for(ten='0'; ten<='9'; ten++) {
      buf[9]=ten;
      for(one='0'; one<='9'; one++) {
         buf[10]=one;
         s=p->get_section(buf);
         if(!s) break;
         buildings->nbuildings++;
         parse_building(file, s, def_clrkey, def_shadowclr, def_see, def_stock, def_cat, bobs, wares, workers, &buildings->buildings[buildings->nbuildings-1], nlist, 1, 1);      
      }
      if(one-1!='9') break;
   }
         
   buildings->buildings=(BuildingDescr*) realloc(buildings->buildings, sizeof(BuildingDescr)*buildings->nbuildings);
   return 0;
}

int parse_buildings_conf(const char* dirname, ushort def_clrkey, ushort def_shadowclr, TribeFileBobsDescr* bobs, TribeFileWaresDescr* wares, TribeFileWorkersDescr* workers, TribeFileBuildingsDescr* buildings, TribeFileNeedList* nlist) {

   char* dir= new char[strlen(dirname)+strlen(BUILDINGS_DIR)+1];
   char* file= new char[strlen(dirname)+strlen(BUILDINGS_DIR)+strlen(CONF_NAME)+1];

   strcpy(dir, dirname);
   strcat(dir, BUILDINGS_DIR);
   strcpy(file, dir);
   strcat(file, CONF_NAME);

   Profile p(cout, file);
 
   parse_buildings(file, &p, def_clrkey, def_shadowclr, bobs, wares, workers, buildings, nlist);
//   parse_mil_buildings(filename, &p, def_clrkey, def_shadowclr, wares, buildings);
//   parse_cannons(filename, &p, def_clrkey, def_shadowclr, workers, wares, buildings);
   
   return 0;
}

int main(int argc, char* argv[])
{
   char dirname[1024];
   char outfile[1024];
	
   cout << "Widelands Tribe File Creator VERSION " <<  VERSION_MAJOR(WLTF_VERSION) << "." << VERSION_MINOR(WLTF_VERSION) << endl;

   if (argc != 2)
	{
		usage();
      return -1;
	}

   
	// get input dir name
	strcpy(dirname, argv[1]);
	if (dirname[strlen(dirname) - 1] != SEPERATOR)
	{
		dirname[strlen(dirname)] = SEPERATOR;
		dirname[strlen(dirname) + 1] = 0;
	}
	// get output file name
   strcpy(outfile, dirname);
   outfile[strlen(outfile)-1] = 0;
   strcat(outfile, WTF_EXT);


   // Start work
   ushort def_clrkey=pack_rgb(255,255,255);
   ushort def_shadowclr=pack_rgb(0,0,0);
	
   TribeFileHeader header;
   TribeFileNeedList nlist;
   TribeFileRegentDescr regent;
   TribeFileBobsDescr bobs;

   if(parse_root_conf(dirname, &def_clrkey, &def_shadowclr, &header, &regent)) {
      return -1;
   }

   // next: Bobs
   // We init the need header, for it is used here for the first time
   memcpy(nlist.magic1, "BNeed\0", sizeof(nlist.magic1));
   nlist.nbobs_needed=0;
   nlist.bobs_name=0;
   memcpy(nlist.magic2, "RNeed\0", sizeof(nlist.magic2));
   nlist.nres_needed=0;
   nlist.res_name=0;
   // We also init the bobs description
   memcpy(bobs.magic1, "DBobs\0", sizeof(bobs.magic1));
   bobs.ndbobs=0;
   bobs.dbobs=0;
   memcpy(bobs.magic2, "GBobs\0", sizeof(bobs.magic2));
   bobs.ngbobs=0;
   bobs.gbobs=0;

   if(parse_bobs_conf(dirname, def_clrkey, def_shadowclr, &bobs, &nlist)) {
      return -1;
   }
   
   // next: wares
   TribeFileWaresDescr wares;
   memcpy(wares.magic, "Wares\0", sizeof(wares.magic));
   wares.nwares=0;
   wares.wares=0;
   if(parse_wares_conf(dirname, def_clrkey, &wares)) {
      return -1;
   }
   
   // next: soldiers
   TribeFileSoldiersDescr soldiers;
   memcpy(soldiers.magic, "Soldiers\0\0", sizeof(soldiers.magic));
   soldiers.needed_items.nitems=0;
   soldiers.needed_items.list=0;
   soldiers.soldiers=0;
   soldiers.nsoldiers=0;
   if(parse_soldiers_conf(dirname, def_clrkey, def_shadowclr, &soldiers, &wares)) {
      return -1;
   }

   // next: workers
   TribeFileWorkersDescr workers;
   memcpy(workers.magic, "Workers\0", sizeof(workers.magic));
   workers.nworkers=0;
   workers.workers=0;
   if(parse_workers_conf(dirname, def_clrkey, def_shadowclr, &workers, &wares)) {
      return -1;
   }

   // next: buildings
   TribeFileBuildingsDescr buildings;
   memcpy(buildings.magic, "Buildings\0", sizeof(buildings.magic));
   memcpy(buildings.magic1, "MilBuilds\0", sizeof(buildings.magic1));
   memcpy(buildings.magic2, "Cannons\0", sizeof(buildings.magic2));
   buildings.nbuildings=0;
   buildings.buildings=0;
   buildings.nmil_buildings=0;
   buildings.mil_buildings=0;
   buildings.ncannons=0;
   buildings.cannons=0;
   if(parse_buildings_conf(dirname, def_clrkey, def_shadowclr, &bobs, &wares, &workers, &buildings, &nlist)) {
      return -1;
   }

   // next: science TODO

   // construct need_list, inform user of it
   uint i=0;
   cout << "The tribe \"" << header.name << "\" needs:" << endl;
   for(i=0; i<nlist.nbobs_needed; i++) 
      cout << " \"" << &nlist.bobs_name[i*30] << "\" which is a Bob!" << endl;
   for(i=0; i<nlist.nres_needed; i++) 
      cout << " \"" << &nlist.res_name[i*30] << "\" which is a Resource!" << endl;
   cout << "Please validate the need list!" << endl;
  
   
   // ALL DATA COLLECTED AND CONSTRUCTED
   // take the blocks, calculate MD5 checksum for header

   // write file

   // WORK DONE!
   
#ifdef WIN32
   // Wait for button hit
   getchar();
#endif

    
   return 0;
}
