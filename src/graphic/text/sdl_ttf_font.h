/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#ifndef SDL_TTF_FONT_H
#define SDL_TTF_FONT_H

#include <string>

#include "graphic/text/rt_render.h"

class FileSystem;

namespace RT {
RT::IFontLoader * ttf_fontloader_from_file(const std::string&);
RT::IFontLoader * ttf_fontloader_from_filesystem(FileSystem*);
}


#endif /* end of include guard: SDL_TTF_FONT_H */

