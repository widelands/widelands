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

#include "graphic/in_memory_image.h"

#include <memory>

#include "graphic/image.h"
#include "graphic/surface.h"

using namespace std;

// An Image implementation the does !not! cache its Surface in the
// SurfaceCache. Avoid using this whenever possible and also do not store it in
// the ImageCache.
//
// This is only used when the surface can not be easily recalculated on the fly
// or if ownership of the image is managed by the caller itself. Note that this
// is always tricky because Widelands assumes in many places that Images can be
// relied to exist forever. So when you pass out a pointer to your
// InMemoryImage, be prepared to keep it valid forever, or check all callsites
// or prepare for core dumps.
class InMemoryImage : public Image {
public:
	InMemoryImage(const string& ghash, Surface* surf) :
		hash_(ghash), surf_(surf) {}
	virtual ~InMemoryImage() {
	}

	// Implements Image.
	virtual uint16_t width() const override {return surf_->width();}
	virtual uint16_t height() const override {return surf_->height();}
	// Note: hash will mostly be dummy values for this implementation. It should
	// not wind up in ImageCache, otherwise the ownership question is not clear.
	virtual const string& hash() const override {return hash_;}
	virtual Surface* surface() const override {return surf_.get();}

private:
	const string hash_;
	std::unique_ptr<Surface> surf_;
};

const Image* new_in_memory_image(const string& hash, Surface* surf) {
	return new InMemoryImage(hash, surf);
}




