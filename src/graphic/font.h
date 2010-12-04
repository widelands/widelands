/*
 * Copyright (C) 2002-2010 by the Widelands Development Team
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

#ifndef FONT_H
#define FONT_H

#include <boost/shared_ptr.hpp>

#include <SDL_ttf.h>

#include "io/fileread.h"

namespace UI {

struct Font {
	static void shutdown();
	static Font * get(const std::string & name, int size);

	TTF_Font * get_ttf_font() const {return m_font;}

private:
	Font(const std::string & name, int size);
	~Font();

	FileRead m_fontfile;
	TTF_Font * m_font;
};

} // namespace UI

#endif // FONT_H
