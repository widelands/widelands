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

#include "auto_pic.h"
#include "fileloc.h"
#include "output.h"
#include <stdio.h>

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
      const char *effname = g_fileloc.locate_file(first->filename, TYPE_PIC);
      if (!effname) {
         sprintf(buf, "%s: File not found. Check your installation and --searchdir.", first->filename);
         tell_user(buf);
         exit(0);
      }
      first->load(effname);
      if ((first->desw && first->desw != (int)first->get_w()) ||
            (first->desh && first->desh != (int)first->get_h())) {
         char buf[1024];
         sprintf(buf, "%s: should be %ix%i pixels.", first->filename, first->desw, first->desh);
         tell_user(buf);
         exit(0);
      }
      first = first->next;
   }
}

