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
#include <iostream>
#include "need_list.h"

Tribe_File_Need_List needl;


/*
 * This is the need list, valid for tribe files
*/
Tribe_File_Need_List::Tribe_File_Need_List(void) {
   needs=0;
   nneeds=0;
   provides=0;
   nprovides=0;
};
Tribe_File_Need_List::~Tribe_File_Need_List(void) {
   if(nneeds) free(needs);
   if(nprovides) free(provides);
}

void Tribe_File_Need_List::add_need(const char* name, Type is_a) {
   uint i;
   for(i=0; i<nneeds; i++) 
      if(!strcasecmp(name, needs[i].name)) return ; // already in list
   nneeds++;

   if(nneeds==1) {
      needs=(Need*) malloc(sizeof(Need)*nneeds);
   } else {
      needs=(Need*) realloc(needs, sizeof(Need)*nneeds);
   }
   
   needs[nneeds-1].is=is_a;
   zmem(needs[nneeds-1].name, sizeof(needs[nneeds-1].name));
   memcpy(needs[nneeds-1].name, name, strlen(name) < sizeof(needs[nneeds-1].name) ? strlen(name) : sizeof(needs[nneeds-1].name)-1);
}

void Tribe_File_Need_List::add_provide(const char* name, Type is_a) {
   
   uint i;
   for(i=0; i<nprovides; i++) 
      if(!strcasecmp(name, provides[i].name)) return ; // already in list

   nprovides++;

   if(nprovides==1) {
      provides=(Need*) malloc(sizeof(Need)*nprovides);
   } else {
      provides=(Need*) realloc(provides, sizeof(Need)*nprovides);
   }
   
   provides[nprovides-1].is=is_a;
   zmem(provides[nprovides-1].name, sizeof(provides[nprovides-1].name));
   memcpy(provides[nprovides-1].name, name, strlen(name) < sizeof(provides[nprovides-1].name) ? strlen(name) : sizeof(provides[nprovides-1].name)-1);
}

int Tribe_File_Need_List::validate(ostream& out, ostream& err) {
   
   uint i, z;
   Need* n;
   for(i=0; i<nneeds; i++) {
      n=&needs[i];
      for(z=0; z<nprovides; z++) {
         if(strcasecmp(n->name, provides[z].name)) continue;
         
         if(n->is == IS_SOME_BOB) {
            if(provides[z].is == IS_RESOURCE) {
               err << "<" << n->name << "> needs a bob, but is defined as resource!" << endl;
               return -1;
            }
         } else {
            if(n->is !=  provides[z].is) {
               err << "<" << n->name << "> is defined as an incompatible bob type!" << endl;
               return -1;
            }
         }

         break;
      }
      if(z!=nprovides) {
         // was found, overwrite it with the last one in needs array and redo this i
         memcpy(&needs[i], &needs[nneeds-1], sizeof(Need));
         nneeds--;
         i--;
      } 
   }
  
   for(i=0; i<nneeds; i++) {
      out << "<" << needs[i].name << "> is needed by this tribe!" << endl;
   }   
   out << "Need List finished. please validate it!" << endl;
   return 0;
}
   
int Tribe_File_Need_List::write(Binary_file *f) {
   // out magic
   f->write("NeedList\0", 9);
   
   // out number of needs
   f->write(&nneeds, sizeof(nneeds));
   
   int i, z;
   ushort is_a;
   for(i=0; i<nneeds; i++) {
      for(z=0; z<10; z++) {
         if(needs[i].is==z) {
            is_a=z;
            f->write(&is_a, sizeof(is_a));
            f->write(needs[i].name, sizeof(needs[i].name));
         }
      }
   }
            
   return 0;
}
