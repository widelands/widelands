/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__TRIGGER_IDS_H
#define __S__TRIGGER_IDS_H

/*
 * If you change anything here,
 * make sure that you also change trigger_factory.cc
 *
 * Make sure that the orders of this enum and in the
 * description structure in trigger_factory.cc stay the same
 */
enum {
   TRIGGER_TIME = 0,
   TRIGGER_NULL,
   TRIGGER_BUILDING,
};

#endif
