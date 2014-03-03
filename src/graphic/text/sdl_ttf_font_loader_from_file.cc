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

#include <string>

#include <boost/format.hpp>

#include "graphic/text/rt_errors.h"
#include "graphic/text/sdl_ttf_font_impl.h"

using namespace std;
using namespace boost;

namespace RT {

class SDLTTF_FontLoaderFromFile : public IFontLoader {
public:
	SDLTTF_FontLoaderFromFile(const string& dir);
	virtual ~SDLTTF_FontLoaderFromFile();
	virtual IFont * load(const string& name, int ptsize) override;

private:
	const string dir_;
};

SDLTTF_FontLoaderFromFile::SDLTTF_FontLoaderFromFile(const string& dir)
	: dir_(dir)
{
	TTF_Init();
}
SDLTTF_FontLoaderFromFile::~SDLTTF_FontLoaderFromFile() {
	TTF_Quit();
}

IFont* SDLTTF_FontLoaderFromFile::load(const string& face, int ptsize) {
	TTF_Font * mfont = TTF_OpenFontIndex((dir_ + "/" + face).c_str(), ptsize, 0);
	if (!mfont)
		throw BadFont((format("Font loading error for %s, %i pts: %s") % face % ptsize % TTF_GetError()).str());

	return new SDLTTF_Font(mfont, face, ptsize);
}

IFontLoader * ttf_fontloader_from_file(const std::string& dir) {
	return new SDLTTF_FontLoaderFromFile(dir);
}

}

