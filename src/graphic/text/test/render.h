/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXT_TEST_RENDER_H
#define WL_GRAPHIC_TEXT_TEST_RENDER_H

#include <memory>
#include <string>

#include "graphic/text/rt_render.h"

// A stand alone richtext renderer for tests and binaries.
class StandaloneRenderer {
public:
	StandaloneRenderer();
	~StandaloneRenderer();

	RT::Renderer* renderer();

private:
	std::unique_ptr<TextureCache> texture_cache_;
	std::unique_ptr<ImageCache> image_cache_;
	std::unique_ptr<RT::Renderer> renderer_;
};

#endif  // end of include guard:
