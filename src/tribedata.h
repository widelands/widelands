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

#ifndef __S__TRIBEDATA_H
#define __S__TRIBEDATA_H

#ifndef PACK_VERSION
#define PACK_VERSION(major, minor) (  (ushort) (((uchar) (major))<<8) | (((uchar) (minor))) ) 
#endif

#ifndef VERSION_MAJOR
#define VERSION_MAJOR(vers) ((ushort) (((ushort) (vers)>>8) & 0xff))
#endif

#ifndef VERSION_MINOR
#define VERSION_MINOR(vers) ((ushort) (((ushort) (vers)) & 0xff))
#endif

#define WLTF_VERSION			PACK_VERSION(1,0)

#define SITDIG       	   0
#define SEARCHER	         1
#define GROWER	            2
#define PLANTER	         3
#define SCIENTIST	         4
#define SOLDIER	         5
#define SPEC_DEF_CARRIER	6
#define SPEC_ADD_CARRIER	7
#define SPEC_BUILDER	      8
#define SPEC_PLANER	      9
#define SPEC_EXPLORER	   10
#define SPEC_GEOLOGIST     11

 
#endif /* __S__TRIBEDATA_H */
