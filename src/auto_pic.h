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

#ifndef __S__AUTO_PIC_H
#define __S__AUTO_PIC_H

#include "pic.h"

/** class AutoPic
 *
 * This class provides a picture that is automatically loaded on startup.
 * Used for UI graphics.
 * Do not use for local or dynamically allocated objects.
 */
class AutoPic : public Pic {
   static AutoPic *first;
   AutoPic *next;
   const char *filename;
   int desw, desh; // desired width & height
   public:
   AutoPic(const char *mfilename, int mdesw = 0, int mdesh = 0) {
      next = first; first = this;
      filename = mfilename; desw = mdesw; desh = mdesh;
   }
   static void load_all();
};

#endif 

