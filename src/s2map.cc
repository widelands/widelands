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
#include "game.h"
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
            if(w->load_world(buf)) {
               assert(0);
            }
            strcpy(hd.uses_world, "greenland");
         }
         break;

      case 1:
         // black world
         if(!w || strcmp(w->get_name(), "blackland")) {
            buf=g_fileloc.locate_file("blackland.wwf", TYPE_WORLD);
            if(!buf) assert(0);
            w= new World();
            if(w->load_world(buf)) {
               assert(0);
            }
            strcpy(hd.uses_world, "blackland");
         }
         break;

      case 2:
         // winter world
         if(!w || strcmp(w->get_name(), "winterland")) {
            buf=g_fileloc.locate_file("winterland.wwf", TYPE_WORLD);
            if(!buf) assert(0);
            w= new World();
            if(w->load_world(buf)) {
               assert(0);
            }
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
int Map::load_s2mf(const char* filen, Game *game) {
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
            if(w->load_world(buf)) {
               assert(0);
            }
            strcpy(hd.uses_world, "greenland");
         }
         break;

      case 1:
         // black world
         if(!w || strcmp(w->get_name(), "blackland")) {
            buf=g_fileloc.locate_file("blackland.wwf", TYPE_WORLD);
            if(!buf) assert(0);
            w=new World();
            if(w->load_world(buf)) {
               assert(0);
            }
            strcpy(hd.uses_world, "blackland");
         }
         break;

      case 2:
         // winter world
         if(!w || strcmp(w->get_name(), "winterland")) {
            buf=g_fileloc.locate_file("winterland.wwf", TYPE_WORLD);
            if(!buf) assert(0);
            w=new World();
            if(w->load_world(buf)) {
               assert(0);
            }
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
				game->warp_building(x, y, bobs[i], 0);
			}
      }
   }
   free(section);

   // S E C T I O N 7  -------- Animals
   // 0x01 == Bunny
   // 0x02 == fox
   // 0x03 == reindeer
   // 0x04 == deer
   // 0x05 == duck
   // 0x06 == sheep
   // New section??
   section = load_s2mf_section(&file, hd.width, hd.height);
   if (!section) {
      cerr << "Section 7 --> NOT FOUND in file" << endl;
      return ERR_FAILED;
   }
   for(y=0; y<hd.height; y++) {
      i=y*hd.width;
      for(x=0; x<hd.width; x++, i++) {
			const char *bobname = 0;
			
         // ignore everything but HQs
         switch(section[i]) {
				case 0:
					break;
            case 0x01: bobname = "bunny"; break;
				case 0x02: bobname = "fox"; break;
				case 0x03: bobname = "reindeer"; break;
				case 0x04: bobname = "deer"; break;
				case 0x05: bobname = "duck"; break;
				case 0x06: bobname = "Sheep"; break;
				default:
					cerr << "Unsupported animal: " << (int)section[i] << endl;
					break;
         }
			
			if (bobname) {
				for(uint z=0; z<CRITTER_PER_DEFINITION; z++)
					game->create_bob(x, y, w->get_bob(bobname));
			}
      }
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
   for(y=0; y<hd.height; y++) {
      for(x=0; x<hd.width; x++) {
			const char *bobname = 0;
		 
      //   if(a_MapGetField(x,y)->can_build_way== 0x80) {
      //    a_BuildingSet(x, y, TYPE_HQ, STATE_FINISH);
      //      continue;
      //   }

         
         c=bobs[y*hd.width + x];
         if(buildings[y*hd.width +x]==0x78) {
            switch(c) {
               case BOB_STONE1: bobname = "stones1"; break;
               case BOB_STONE2: bobname = "stones1"; break;
               case BOB_STONE3: bobname = "stones1"; break;
               case BOB_STONE4: bobname = "stones1"; break;
               case BOB_STONE5: bobname = "stones1"; break;
               case BOB_STONE6: bobname = "stones1"; break;
					default: break;
				}
				if (bobname) {
					game->create_bob(x, y, w->get_bob(bobname));
					continue;
				}
         }

         switch (c) {
            case BOB_NONE :
               // DO nothing
               break;

            case BOB_PEBBLE1: bobname = "pebble1"; break;
            case BOB_PEBBLE2: bobname = "pebble2"; break;
            case BOB_PEBBLE3: bobname = "pebble3"; break;
            case BOB_PEBBLE4: bobname = "pebble4"; break;
            case BOB_PEBBLE5: bobname = "pebble5"; break;
            case BOB_PEBBLE6: bobname = "pebble6"; break;
            
            case BOB_MUSHROOM1: bobname = "mushroom1"; break;
            case BOB_MUSHROOM2: bobname = "mushroom2"; break;
            
				case BOB_DEADTREE1: bobname = "deadtree1"; break;
				case BOB_DEADTREE2: bobname = "deadtree2"; break;
				case BOB_DEADTREE3: bobname = "deadtree3"; break;
				case BOB_DEADTREE4: bobname = "deadtree4"; break;
            
				case BOB_TREE1: bobname = "tree1"; break;
				case BOB_TREE2: bobname = "tree2"; break;
				case BOB_TREE3: bobname = "tree3"; break;
				case BOB_TREE4: bobname = "tree4"; break;
				case BOB_TREE5: bobname = "tree5"; break;
				case BOB_TREE6: bobname = "tree6"; break;
				case BOB_TREE7: bobname = "tree7"; break;
				case BOB_TREE8: bobname = "tree8"; break;
				case BOB_TREE9: bobname = "tree9"; break;
				case BOB_TREE10: bobname = "tree10"; break;
				case BOB_TREE11: bobname = "tree11"; break;
				case BOB_TREE12: bobname = "tree12"; break;
				case BOB_TREE13: bobname = "tree13"; break;
				case BOB_TREE14: bobname = "tree14"; break;
				case BOB_TREE15: bobname = "tree15"; break;
				case BOB_TREE16: bobname = "tree16"; break;
				case BOB_TREE17: bobname = "tree17"; break;
				case BOB_TREE18: bobname = "tree18"; break;
				case BOB_TREE19: bobname = "tree19"; break;
				case BOB_TREE20: bobname = "tree20"; break;
				case BOB_TREE21: bobname = "tree21"; break;
				case BOB_TREE22: bobname = "tree22"; break;
				case BOB_TREE23: bobname = "tree23"; break;
				case BOB_TREE24: bobname = "tree24"; break;
				case BOB_TREE25: bobname = "tree25"; break;
				case BOB_TREE26: bobname = "tree26"; break;
				case BOB_TREE27: bobname = "tree27"; break;
				case BOB_TREE28: bobname = "tree28"; break;
				case BOB_TREE29: bobname = "tree29"; break;
				case BOB_TREE30: bobname = "tree30"; break;
				case BOB_TREE31: bobname = "tree31"; break;
				case BOB_TREE32: bobname = "tree32"; break;
            
            case BOB_GRASS1: bobname = "grass1"; break;
            case BOB_GRASS2: bobname = "grass2"; break;
            case BOB_GRASS3: bobname = "grass3"; break;
            
				case BOB_STANDING_STONES1: bobname = "sstones1"; break;
				case BOB_STANDING_STONES2: bobname = "sstones2"; break;
				case BOB_STANDING_STONES3: bobname = "sstones3"; break;
				case BOB_STANDING_STONES4: bobname = "sstones4"; break;
				case BOB_STANDING_STONES5: bobname = "sstones5"; break;
				case BOB_STANDING_STONES6: bobname = "sstones6"; break;
				case BOB_STANDING_STONES7: bobname = "sstones7"; break;
				
            case BOB_SKELETON1: bobname = "skeleton1"; break;
            case BOB_SKELETON2: bobname = "skeleton2"; break;
            case BOB_SKELETON3: bobname = "skeleton3"; break;
            
				case BOB_CACTUS1: bobname = "cactus1"; break;
				case BOB_CACTUS2: bobname = "cactus2"; break;
				
            case BOB_BUSH1: bobname = "bush1"; break;
            case BOB_BUSH2: bobname = "bush2"; break;
            case BOB_BUSH3: bobname = "bush3"; break;
            case BOB_BUSH4: bobname = "bush4"; break;
            case BOB_BUSH5: bobname = "bush5"; break;
            
				default:
               // N("Unknown bob in file! %x (%i,%i)\n", c, x, y);
               assert(!"Unknow bob in file");
               break;
         }
			
			if (bobname) {
				game->create_bob(x, y, w->get_bob(bobname));
			}
      }
   }
   


   free(bobs);
   free(buildings);
   return RET_OK;
}


