/*
 * Copyright (C) 2002 by Holger Rapp
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

#include "pic_descr.h"
#include <string.h>

/*
 * construct a bitmap description (as used for pictures in Datafiles)
 *
 * Args: gdata     construct description in this pice of data
 *       size     size of data, if=0, then data is newly malloc()ed
 * Returns: used size of data or 0 if function fails
 */
uint Bitmap_Descr::construct_description(void* gdata, ushort size) {
   if(size<(get_h()*get_w()*2)) return 0;

   memcpy(gdata, pixels, get_h()*get_w()*2);

   return get_h()*get_w()*2;
}

