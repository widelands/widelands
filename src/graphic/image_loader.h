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

#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <string>

class IPicture;

/// This only knows how to load an image. It is the only thing that is
/// implementation dependant in the ImageCache.
class IImageLoader {
public:
	virtual ~IImageLoader() {}

	virtual IPicture* load(const std::string& fn, bool alpha) const = 0;
};

#endif /* end of include guard: IMAGE_LOADER_H */

