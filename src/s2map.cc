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

#include "widelands.h"
#include "s2map.h"
#include "myfile.h"
#include "map.h"
#include "fileloc.h"

// TEMP
#define hex ios::hex
#define dec ios::dec

/** uchar *Map::load_s2mf_section(Binary_file *file, int width, int height)
 *
 * Some of the original S2 maps have rather odd sizes. In that case, however,
 * width (and height?) are rounded up to some alignment. The in-file size of
 * a section is stored in the section header (I think ;)).
 * This is the work-around.
 *
 * Args: file	the file to read from
 *       width	desired width to pack to
 *       height	desired height to pack to
 *
 * Returns: Pointer to the (packed) contents of the section. 0 if the read
 *          failed.
 *          If successful, you must free() the returned are of memory
 */
uchar *Map::load_s2mf_section(Binary_file *file, int width, int height)
{
   ushort dw, dh;
   char buffer[256];
   ushort one;
   long size;

   file->read(buffer, 6);
   if ((buffer[0] != 0x10) ||
         (buffer[1] != 0x27) ||
         (buffer[2] != 0x00) ||
         (buffer[3] != 0x00) ||
         (buffer[4] != 0x00) ||
         (buffer[5] != 0x00)) {
      cerr << "Section marker not found" << endl;
      return 0;
   }

   file->read(&dw, 2);
   file->read(&dh, 2);

   file->read(&one, 2);
   assert(one == 1);
   file->read(&size, 4);
   assert(size == dw*dh);

   if (dw < width || dh < height) {
      cerr << "Section not big enough" << endl;
      return 0;
   }

   uchar *section = (uchar *)malloc(dw * dh);
   int y;
   for(y = 0; y < height; y++) {
      file->read(section + y*width, width);
      file->read(buffer, dw-width); // alignment junk
   }
   while(y < dh) {
      file->read(buffer, dw); // more alignment junk
      y++;
   }

   return section;
}

//
// load the header of a S2 map
//
int Map::load_s2mf_header(const char* filen) {
   Binary_file file;

   if(!filen) return ERR_FAILED;

   file.open(filen, File::READ);
   if(file.get_state() != File::OPEN) {
      return ERR_FAILED;
   }

   S2MapDescrHeader header;
   file.read(&header, sizeof(header));


   strncpy(hd.author, header.author, 26);
   hd.author[26]='\0';
   strcpy(hd.magic, WLMF_MAGIC);
   strncpy(hd.name, header.name, 20);
   hd.name[21]='\0';
   hd.nplayers=header.nplayers;
   hd.width=header.w;
   hd.height=header.h;
   hd.version=WLMF_VERSION;
   strcpy(hd.descr, "Bluebyte Settlers II Map. No comment defined!");

   const char* buf;
   switch(header.uses_world) {
      case 0:
         // green world
         if(!w || strcmp(w->get_name(), "greenland")) {
            buf=g_fileloc.locate_file("greenland.wwf", TYPE_WORLD);
            if(!buf) assert(0);
            w= new World();
            assert(!w->load_world(buf));
            strcpy(hd.uses_world, "greenland");
         }
         break;

      case 1:
         // black world
         if(!w || strcmp(w->get_name(), "blackland")) {
            buf=g_fileloc.locate_file("blackland.wwf", TYPE_WORLD);
            if(!buf) assert(0);
            w= new World();
            assert(!w->load_world(buf));
            strcpy(hd.uses_world, "blackland");
         }
         break;

      case 2:
         // winter world
         if(!w || strcmp(w->get_name(), "winterland")) {
            buf=g_fileloc.locate_file("winterland.wwf", TYPE_WORLD);
            if(!buf) assert(0);
            w= new World();
            assert(!w->load_world(buf));
            strcpy(hd.uses_world, "winterland");
         }
         break;
   }

   // set size
   set_size(hd.width, hd.height);


   return RET_OK;
}

/** int Map::load_s2mf(const char* filen)
 *
 * this loads a given file as a settlers 2 map file
 *
 * ***** PRIVATE FUNC ******
 *
 * Args: 	filen		filename to read
 * Returns: RET_OK or RET_FAILED
 */
int Map::load_s2mf(const char* filen, Cmd_Queue* q) {
   Binary_file file;
   uchar *section, *pc;
   uint x=0;
   uint y=0;

   if(!filen) return ERR_FAILED;

   file.open(filen, File::READ);
   if(file.get_state() != File::OPEN) {
      return ERR_FAILED;
   }

   S2MapDescrHeader header;
   file.read(&header, sizeof(header));


   strncpy(hd.author, header.author, 26);
   hd.author[26]='\0';
   strcpy(hd.magic, WLMF_MAGIC);
   strncpy(hd.name, header.name, 20);
   hd.name[21]='\0';
   hd.nplayers=header.nplayers;
   hd.width=header.w;
   hd.height=header.h;
   hd.version=WLMF_VERSION;
   strcpy(hd.descr, "Bluebyte Settlers II Map. No comment defined!");

   const char* buf;
   switch(header.uses_world) {
      case 0:
         // green world
         if(!w || strcmp(w->get_name(), "greenland")) {
            buf=g_fileloc.locate_file("greenland.wwf", TYPE_WORLD);
            if(!buf) assert(0);
            w=new World();
            assert(!w->load_world(buf));
            strcpy(hd.uses_world, "greenland");
         }
         break;

      case 1:
         // black world
         if(!w || strcmp(w->get_name(), "blackland")) {
            buf=g_fileloc.locate_file("blackland.wwf", TYPE_WORLD);
            if(!buf) assert(0);
            w=new World();
            assert(!w->load_world(buf));
            strcpy(hd.uses_world, "blackland");
         }
         break;

      case 2:
         // winter world
         if(!w || strcmp(w->get_name(), "winterland")) {
            buf=g_fileloc.locate_file("winterland.wwf", TYPE_WORLD);
            if(!buf) assert(0);
            w=new World();
            assert(!w->load_world(buf));
            strcpy(hd.uses_world, "winterland");
         }
         break;
   }

   // set size
   set_size(hd.width, hd.height);

   ////           S E C T I O N    1 : H E I G H T S
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Heights --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }

   Field *f = fields;
   pc = section;
   for(y=0; y<hd.height; y++) {
      for(x=0; x<hd.width; x++, f++, pc++) {
         f->set_height(*pc);
         f->hook_instance(0); // we bring the field in a acceptable init state 
      }
   }
   free(section);

   ////				S E C T I O N		2: Landscape
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "LANDSCAPE --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }

   f = fields;
   pc = section;
   for(y=0; y<hd.height; y++) {
      for(x=0; x<hd.width; x++, f++, pc++) {
         char c = *pc;
         c &= 0x1f;
         switch((int)c) {
            case 0x00: c=0; break;
            case 0x01: c=1; break;
            case 0x02: c=2; break;
            case 0x03: c=3; break;
            case 0x04: c=4; break;
            case 0x05: c=5; break;

            case 0x08: c=6; break;
            case 0x09: c=7; break;
            case 0x0a: c=8; break;
            case 0x0b: c=9; break;
            case 0x0c: c=10; break;
            case 0x0d: c=11; break;
            case 0x0e: c=12; break;
            case 0x0f: c=13; break;

            case 0x10: c=14; break;
            case 0x12: c=15; break;

            case 0x07: c=4; break; // Unknown texture
            case 0x13: c=4; break; // unknown texture!
            default: c = 7; cerr << "ERROR: Unknown texture1: " << hex << c << dec << " (" << x << "," << y << ") (defaults to water!)" << endl;
         }
         f->set_terraind(w->get_terrain(c));
      }
   }
   free(section);



   // S E C T I O N 3  -------- LANDSCAPE 2
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "LANDSCAPE 2 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }

   f = fields;
   pc = section;
   for(y=0; y<hd.height; y++) {
      for(x=0; x<hd.width; x++, f++, pc++) {
         char c = *pc;
         c &= 0x1f;
         switch((int)c) {
            case 0x00: c=0; break;
            case 0x01: c=1; break;
            case 0x02: c=2; break;
            case 0x03: c=3; break;
            case 0x04: c=4; break;
            case 0x05: c=5; break;

            case 0x08: c=6; break;
            case 0x09: c=7; break;
            case 0x0a: c=8; break;
            case 0x0b: c=9; break;
            case 0x0c: c=10; break;
            case 0x0d: c=11; break;
            case 0x0e: c=12; break;
            case 0x0f: c=13; break;

            case 0x10: c=14; break;
            case 0x12: c=15; break;

            case 0x07: c=4; break; // Unknown texture
            case 0x13: c=4; break; // unknown texture!
            default: c = 7; cerr << "ERROR: Unknown texture1: " << hex << c << dec << " (" << x << "," << y << ") (defaults to water!)" << endl;
         }
         f->set_terrainr(w->get_terrain(c));
      }
   }
   free(section);


   // S E C T I O N 4  -------- UNKNOWN !!! Skip
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section UNKNOWN --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }
   free(section);

   // S E C T I O N 5  -------- Landscape (rocks, stuff..)
   // New section??
   uchar* bobs;
   bobs = load_s2mf_section(&file, hd.width, hd.height);
   if (!bobs) {
      cerr << "Section 5 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }

   // S E C T I O N 6  -------- Ways
   // This describes where you can put ways
   // 0xc* == it's not possible to build ways here now
   // 0x80 == Heres a HQ. bob is Player number
   //      bob == 0 blue
   //      bob == 1 yellow
   //      bob == 2 red
   //      bob == 3 pink
   //      bob == 4 grey
   //      bob == 6 green
   //      bob == 6 orange
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section 6 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }
   uint i=0;
   for(y=0; y<hd.height; y++) {
      i=y*hd.width;
      for(x=0; x<hd.width; x++, i++) {
         // ignore everything but HQs
         if(section[i]==0x80) {
            // cerr << x << ":" << y << ": HQ here! player: " << (int) bobs[i] << endl;
            Point* p = (Point*) malloc(sizeof(Point)); 
            p->x=x;
            p->y=y;
            q->queue(SENDER_LOADER, CMD_WARP_BUILDING, bobs[i], 0, p);
         }
      }
   }
   free(section);

   // S E C T I O N 7  -------- Animals
   // 0x01 == Bunny
   // 0x02 == fox
   // 0x03 == reindeer
   // 0x04 == rein
   // 0x05 == duck
   // 0x06 == sheep
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section 7 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }
   free(section);

   // S E C T I O N 8  --------UNKNOWN
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section 8 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }
   free(section);

   // S E C T I O N 9  -------- What buildings can be build?
   // 0x01 == flags (?? )
   // 0x02 == buildings (small) (??)
   // 0x04 == buildings
   // 0x09 == flags
   // 0x0a == buildings (small) (??)
   // 0x0c == buildings (big) (??)
   // 0x0d == mining
   // 0x68 == trees
   // 0x78 == no buildings
   // New section??
   uchar* buildings = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section 9 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }

   // S E C T I O N 10  -------- UNKNOWN
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section 10 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }
   free(section);

   // S E C T I O N 11  -------- STARTING_POINT
   // I don't know what this does. It really identifies some points
   //  (6 on new maps, 1 on old)
   //  But this points don't make sense....
   //  We skip it.
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section 11 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }
   free(section);

   // S E C T I O N 12  -------- Mining
   // 0x00 == Water
   // 0x87 == ?? (but nothing)
   // 0x21 == things laying around (nothing)
   // 0x40 == nothing
   // 0x51-57 == gold 1-7
   // 0x49-4f == iron 1-7
   // 0x41-47 == cowl 1-7
   // 0x59-5f == stones 1-7
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section 12 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }


   // S E C T I O N 13  -------- Bergflanken.
   //
   // ?? for what is that ??
   // Skip
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section 13 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }
   free(section);

   // S E C T I O N 14  -------- Fieldcount
   // Describes to which island the field sticks
   //  0 == water
   //  1 == island 1
   //  2 == island 2
   //  ....
   //  fe == killing field (lava)
   //
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section 14 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }
   free(section);

   file.close();


   uchar c;
   Point* p;
   for(y=0; y<hd.height; y++) {
      for(x=0; x<hd.width; x++) {
 
      //   if(a_MapGetField(x,y)->can_build_way== 0x80) {
      //    a_BuildingSet(x, y, TYPE_HQ, STATE_FINISH);
      //      continue;
      //   }

         
         c=bobs[y*hd.width + x];
         if(buildings[y*hd.width +x]==0x78) {
            switch(c) {
               case BOB_STONE1:
                  p=new Point(x,y);
                  q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("stones1"), 0, p);
                  continue;
               case BOB_STONE2:
                  p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("stones2"), 0, p);
                  continue;
               case BOB_STONE3:
                  p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("stones3"), 0, p);
                  continue;
               case BOB_STONE4:
                  p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("stones4"), 0, p);
                  continue;
               case BOB_STONE5:
                  p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("stones5"), 0, p);
                  continue;
               case BOB_STONE6:
                  p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("stones6"), 0, p);
                  continue;

               default:
                  // Silently ignoring. S2 maps format is strange... or not understood
                  break;
            }
         }

         switch (c) {
            case BOB_NONE :
               // DO nothing
               break;

            case BOB_PEBBLE1:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("pebble1"), 0, p);
               break;
            case BOB_PEBBLE2:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("pebble2"), 0, p);
               break;
            case BOB_PEBBLE3:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("pebble3"), 0, p);
               break;
            case BOB_PEBBLE4:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("pebble4"), 0, p);
               break;
            case BOB_PEBBLE5:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("pebble5"), 0, p);
               break;
            case BOB_PEBBLE6:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("pebble6"), 0, p);
               break;

            case BOB_MUSHROOM1:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("mushroom1"), 0, p);
               break;
            case BOB_MUSHROOM2:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("mushroom2"), 0, p);
               break;

            case BOB_DEADTREE1:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("deadtree1"), 0, p);
               break;
            case BOB_DEADTREE2:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("deadtree2"), 0, p);
               break;
            case BOB_DEADTREE3:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("deadtree3"), 0, p);
               break;
            case BOB_DEADTREE4:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("deadtree4"), 0, p);
               break;

            case BOB_TREE1:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree1"), 0, p);
               break;
            case BOB_TREE2:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree2"), 0, p);
               break;
            case BOB_TREE3:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree3"), 0, p);
               break;
            case BOB_TREE4:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree4"), 0, p);
               break;
            case BOB_TREE5:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree5"), 0, p);
               break;
            case BOB_TREE6:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree6"), 0, p);
               break;
            case BOB_TREE7:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree7"), 0, p);
               break;
            case BOB_TREE8:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree8"), 0, p);
               break;
            case BOB_TREE9:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree9"), 0, p);
               break;
            case BOB_TREE10:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree10"), 0, p);
               break;
            case BOB_TREE11:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree11"), 0, p);
               break;
            case BOB_TREE12:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree12"), 0, p);
               break;
            case BOB_TREE13:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree13"), 0, p);
               break;
            case BOB_TREE14:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree14"), 0, p);
               break;
            case BOB_TREE15:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree15"), 0, p);
               break;
            case BOB_TREE16:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree16"), 0, p);
               break;
            case BOB_TREE17:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree17"), 0, p);
               break;
            case BOB_TREE18:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree18"), 0, p);
               break;
            case BOB_TREE19:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree19"), 0, p);
               break;
            case BOB_TREE20:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree20"), 0, p);
               break;
            case BOB_TREE21:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree21"), 0, p);
               break;
            case BOB_TREE22:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree22"), 0, p);
               break;
            case BOB_TREE23:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree23"), 0, p);
               break;
            case BOB_TREE24:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree24"), 0, p);
               break;
            case BOB_TREE25:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree25"), 0, p);
               break;
            case BOB_TREE26:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree26"), 0, p);
               break;
            case BOB_TREE27:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree27"), 0, p);
               break;
            case BOB_TREE28:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree28"), 0, p);
               break;
            case BOB_TREE29:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree29"), 0, p);
               break;
            case BOB_TREE30:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree30"), 0, p);
               break;
            case BOB_TREE31:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree31"), 0, p);
               break;
            case BOB_TREE32:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("tree32"), 0, p);
               break;

            case BOB_GRASS1:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("grass1"), 0, p);
               break;
            case BOB_GRASS2:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("grass2"), 0, p);
               break;
            case BOB_GRASS3:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("grass3"), 0, p);
               break;

            case BOB_STANDING_STONES1:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("sstones1"), 0, p);
               break;
            case BOB_STANDING_STONES2:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("sstones2"), 0, p);
               break;
            case BOB_STANDING_STONES3:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("sstones3"), 0, p);
               break;
            case BOB_STANDING_STONES4:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("sstones4"), 0, p);
               break;
            case BOB_STANDING_STONES5:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("sstones5"), 0, p);
               break;
            case BOB_STANDING_STONES6:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("sstones6"), 0, p);
               break;
            case BOB_STANDING_STONES7:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("sstones7"), 0, p);
               break;

            case BOB_SKELETON1:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("skeleton1"), 0, p);
               break;
            case BOB_SKELETON2:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("skeleton2"), 0, p);
               break;
            case BOB_SKELETON3:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("skeleton3"), 0, p);
               break;

            case BOB_CACTUS1:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("cactus1"), 0, p);
               break;
            case BOB_CACTUS2:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("cactus2"), 0, p);
               break;

            case BOB_BUSH1:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("bush1"), 0, p);
               break;
            case BOB_BUSH2:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("bush2"), 0, p);
               break;
            case BOB_BUSH3:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("bush3"), 0, p);
               break;
            case BOB_BUSH4:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("bush4"), 0, p);
               break;
            case BOB_BUSH5:
               p=new Point(x,y); q->queue(SENDER_LOADER, CMD_CREATE_BOB, w->get_bob("bush5"), 0, p);
               break;

            default:
               // N("Unknown bob in file! %x (%i,%i)\n", c, x, y);
               assert(!"Unknow bob in file");
               break;

         } 
      }
   }


   free(bobs);
   free(buildings);
   return RET_OK;
}


