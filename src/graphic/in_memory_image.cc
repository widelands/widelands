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
#include "graphic/texture.h"

using namespace std;

// An Image implementation the does !not! cache its Surface in the
// TextureCache. Avoid using this whenever possible and also do not store it in
// the ImageCache.
//
// This is only used when the texture can not be easily recalculated on the fly
// or if ownership of the image is managed by the caller itself. Note that this
// is always tricky because Widelands assumes in many places that Images can be
// relied to exist forever. So when you pass out a pointer to your
// InMemoryImage, be prepared to keep it valid forever, or check all callsites
// or prepare for core dumps.
// NOCOM(#sirver): kill.
class InMemoryImage : public Image {
public:
	InMemoryImage(Texture* texture) :
		texture_(texture) {}
	virtual ~InMemoryImage() {
	}

	// Implements Image.
	int width() const override {return texture_->width();}
	int height() const override {return texture_->height();}

	int get_gl_texture() const override {
		return texture_->get_gl_texture();
	}
	const FloatRect& texture_coordinates() const override {
		return texture_->texture_coordinates();
	}

private:
	std::unique_ptr<Texture> texture_;
};

std::unique_ptr<const Image> new_in_memory_image(Texture* texture) {
	return std::unique_ptr<const Image>(new InMemoryImage(texture));
}
