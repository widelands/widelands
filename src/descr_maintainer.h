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

#ifndef __S__DESCR_MAINTAINER_H
#define __S__DESCR_MAINTAINER_H

#include <string.h>

/*
 * This template is used to have a type save mainting class for Bob_Descr, Worker_Descr 
 * and so on
 */
template <class T> class Descr_Maintainer {
   public:
      Descr_Maintainer(void) { nitems=0; items=0; }
      ~Descr_Maintainer(void) ; 

      T* exists(const char* name);
      T* start_enum(void) { n=0; if(nitems) return items[0]; return NULL; }
      T* get_nitem(void) { n++; if(n<nitems) return items[n]; return NULL; }
      void add(T* item);
      ushort get_nitems(void) { return nitems; }
      ushort get_index(const char* name);
     
      inline T* get(uint idx) { assert(idx<nitems); return items[idx]; }

   private:
      uint n;
      uint nitems;
      T** items;
};

template <class T>
ushort Descr_Maintainer<T>::get_index(const char* name) {
  
   ushort i;
   for(i=0; i<nitems; i++) {
      if(!strcasecmp(name, items[i]->get_name())) return i;
   }

   
   // assert(0); // we should never be here!
   return 0;
}

/*template <class T> 
T* Descr_Maintainer<T>::get(const char* name) {
   uint i;

   for(i=0; i<nitems; i++) {
      if(!strcasecmp(name, items[i]->get_name())) return items[i];
   }

   nitems++;
   if(nitems==1) {
      items=(T**) malloc(sizeof(T*)*nitems);
   } else {
      items=(T**) realloc(items, sizeof(T*)*nitems);
   } 
   items[nitems-1]= new T(name);
   
   return items[nitems-1]; 
}*/
      
template <class T>
void Descr_Maintainer<T>::add(T* item) {
   nitems++;
   if(nitems==1) {
      items=(T**) malloc(sizeof(T*)*nitems);
   } else {
      items=(T**) realloc(items, sizeof(T*)*nitems);
   } 
   items[nitems-1]=item;
}

//
//returns elemt if it exists, NULL if it doesnt
//
template <class T> 
T* Descr_Maintainer<T>::exists(const char* name) {
   uint i;

   for(i=0; i<nitems; i++) {
      if(!strcasecmp(name, items[i]->get_name())) return items[i];
   }
   return NULL;
}

template <class T>
Descr_Maintainer<T>::~Descr_Maintainer(void) {
   uint i;
   for(i=0; i<nitems; i++) {
      delete items[i];
   }
   free(items);
}

#endif
