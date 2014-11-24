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

#include "graphic/image_cache.h"

#include <cassert>
#include <string>

#include "base/log.h"
#include "graphic/image.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "graphic/texture_cache.h"

namespace  {

// Image Implementation that loads images from disc when they should be drawn.
// Uses TextureCache. These images are meant to be cached in ImageCache.
class FromDiskImage : public Image {
public:
	FromDiskImage(const std::string& filename, TextureCache* texture_cache) :
		filename_(filename),
		texture_cache_(texture_cache) {
			Texture* texture = reload_image_();
			w_ = texture->width();
			h_ = texture->height();
		}
	virtual ~FromDiskImage() {}

	// Implements Image.
	uint16_t width() const override {return w_; }
	uint16_t height() const override {return h_;}
	const std::string& hash() const override {return filename_;}
	Texture* texture() const override {
		Texture* texture = texture_cache_->get(filename_);
		if (texture)
			return texture;
		return reload_image_();
	}

private:
	Texture* reload_image_() const {
		Texture* texture = texture_cache_->insert(filename_, load_image(filename_), false);
		return texture;
	}
	uint16_t w_, h_;
	const std::string filename_;

	TextureCache* const texture_cache_;  // Not owned.
};

}  // namespace

ImageCache::ImageCache(TextureCache* const texture_cache) : texture_cache_(texture_cache) {
}

ImageCache::~ImageCache() {
	for (ImageMap::value_type& p : images_) {
		delete p.second;
	}
	images_.clear();
}

bool ImageCache::has(const std::string& hash) const {
	return images_.count(hash);
}

const Image* ImageCache::insert(const Image* image) {
	assert(!has(image->hash()));
	images_.insert(make_pair(image->hash(), image));
	return image;
}

const Image* ImageCache::get(const std::string& hash) {
	ImageMap::const_iterator it = images_.find(hash);
	if (it == images_.end()) {
		images_.insert(make_pair(hash, new FromDiskImage(hash, texture_cache_)));
		return get(hash);
	}
	return it->second;
}
