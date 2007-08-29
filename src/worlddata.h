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
#define PACK_VERSION(major, minor) (static_cast<ushort>((static_cast<uchar>(major))<<8) | ((static_cast<uchar>(minor))))
#endif

#ifndef VERSION_MAJOR
#define VERSION_MAJOR(vers) (static_cast<ushort>((static_cast<ushort>(vers)>>8) & 0xff))
#endif

#ifndef VERSION_MINOR
#define VERSION_MINOR(vers) (static_cast<ushort>((static_cast<ushort>(vers)) & 0xff))
#endif

#define WLWF_VERSION PACK_VERSION(1, 0)


#define TERRAIN_DRY        1
// #define TERRAIN_GREEN      1
#define TERRAIN_WATER      2
#define TERRAIN_ACID       4
#define TERRAIN_MOUNTAIN   8
// fine TERRAIN_DEAD       5
#define TERRAIN_UNPASSABLE 16


#endif /* __S__TRIBEDATA_H */
