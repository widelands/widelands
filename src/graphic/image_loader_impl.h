/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#ifndef IMAGE_LOADER_IMPL_H
#define IMAGE_LOADER_IMPL_H

#include <string>

#include "graphic/image_loader.h"

class ImageLoaderImpl : public IImageLoader {
public:
	virtual ~ImageLoaderImpl() {}

	Surface* load(const std::string& fname, FileSystem* fs = nullptr) const override;
};


#endif /* end of include guard: IMAGE_LOADER_IMPL_H */

