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

#ifndef __S__NEED_LIST_H
#define __S__NEED_LIST_H

#include <iostream>
#include "../src/myfile.h"
#include "../src/helper.h"

/*
 * This is the need list, valid for tribe files
 */
class Tribe_File_Need_List  {
   public:
      enum Type {
         IS_GROWING_BOB,
         IS_DIMINISHING_BOB,
         IS_CRITTER_BOB,
         IS_RESOURCE,
         IS_SOME_BOB
      };
         
      Tribe_File_Need_List(void);
      ~Tribe_File_Need_List(void);

      void add_provide(const char* name, Type is_a);
      void add_need(const char* name, Type is_a);
      int validate(ostream& out, ostream& err);
      int write(Binary_file* f);

   private:
      struct Need {
         Type is;
         char name[30];
      };
      Need* needs;
      ushort nneeds;
      Need* provides;
      ushort nprovides;
};

extern Tribe_File_Need_List needl;


#endif

