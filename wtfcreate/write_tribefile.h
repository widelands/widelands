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


#ifndef __S__WRITE_TRIBEFILE_H
#define __S__WRITE_TRIBEFILE_H

#include "../src/md5file.h"
#include "building_descr.h"
#include "tribe_descr.h"

int write_tribefile(MD5_Binary_file* f, Buildings_Descr* buildings, Tribe_Header* header, Regent_Descr* regent);

#endif

