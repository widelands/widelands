/*
 * Copyright (C) 2002 by The Widelands Development Team
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

#include "widelands.h"
#include "auto_pic.h"

/*
==========================================================================

   AutoPic

==========================================================================
*/

AutoPic *AutoPic::first = 0;

/** AutoPic::load_all()
 *
 * Loads all AutoPics into memory.
 */
void AutoPic::load_all()
{
   while(first) {
      char buf[1024];
		snprintf(buf, sizeof(buf), "pics/%s", first->filename);
      first->load(buf);
      if ((first->desw && first->desw != (int)first->get_w()) ||
            (first->desh && first->desh != (int)first->get_h()))
			critical_error("%s: should be %ix%i pixels.", first->filename, first->desw, first->desh);
      first = first->next;
   }
}

