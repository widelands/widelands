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
#include "../wtfcreate/need_list.h"
#include "resource_descr.h"

Fabric<Resource_Descr> resf;


//
// class Resource_Descr
//
Resource_Descr::Resource_Descr(const char* gname) {
   strncpy(name, gname, 30);
   name[29]='\0';
   minh=maxh=importance=0;
   
   needl.add_provide(name, Need_List_Descr::IS_RESOURCE);
}

Resource_Descr::~Resource_Descr(void) {
}

int Resource_Descr::construct(Profile* p, Section *s) {
   
   minh=s->get_int("min_height", 0);
   maxh=s->get_int("max_height", 255);
   importance=s->get_int("importance", 0);
   
   return OK;
}
 
int Resource_Descr::write(Binary_file* f) {
   
   f->write(name, 30);
   f->write(&minh, sizeof(uchar));
   f->write(&maxh, sizeof(uchar));
   f->write(&importance, sizeof(uchar));

   return OK;
}
