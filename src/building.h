/*
 * Copyright (C) 2002 by Florian Bluemel
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

#ifndef __S__BUILDING_H
#define __S__BUIlDING_H


//
// This class describes a in-game building
//
class Building {

};

/*
 * This is the base class for every building, a tribe can 
 * build.
 * It is pure virtual, mainly a interface class.
 */
class Building_Descr {
   public:
      Building_Descr(void);
      virtual ~Building_Descr(void);

//      virtual is_a(void);
      

};

#endif // __S__BUILDING_H
