/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_PAM_IO_H
#define WL_GRAPHIC_PAM_IO_H

// Routines to work with PAM images.
// http://netpbm.sourceforge.net/doc/pam.html

#include <memory>

#include "graphic/surface.h"

class FileRead;
class FileWrite;

// Loads a PAM Image from 'sr'. Does not take ownership.
std::unique_ptr<Surface> load_pam(FileRead* sr);

// Saves a surface as a PAM image into fw. Does not take ownership.
void save_pam(Surface* surface, FileWrite* fw);

#endif  // end of include guard:
