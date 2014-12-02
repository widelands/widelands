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

#include "graphic/image_catalog.h"

#include <cassert>
#include <map>
#include <string>

#include "base/log.h"
#include "io/filesystem/layered_filesystem.h"

ImageCatalog::ImageCatalog() {
	init();
}

ImageCatalog::~ImageCatalog() {
	entries_.clear();
}

// Register all images here
void ImageCatalog::init()  {
	entries_.clear();
	insert(Keys::kButton0, "but0.png");
	insert(Keys::kButton1, "but1.png");
	insert(Keys::kButton2, "but2.png");
	insert(Keys::kButton3, "but3.png");
	insert(Keys::kButton4, "but4.png");
}

void ImageCatalog::insert(const Keys& key, const std::string& filename)  {
	const std::string path = kBaseDir + filename;
	assert(g_fs->file_exists(path));
	entries_.emplace(key, path);
}

const std::string& ImageCatalog::filepath(const Keys& key) const {
	assert(entries_.count(key) == 1);
	return entries_.at(key);
}
