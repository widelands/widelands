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

#include <boost/scoped_ptr.hpp>

#include "image.h"
#include "surface.h"

#include "in_memory_image.h"

using namespace std;

// NOCOM(#sirver): documnent me
class InMemoryImage : public Image {
public:
	InMemoryImage(const string& hash, Surface* surf) :
		hash_(hash), surf_(surf) {}
	virtual ~InMemoryImage() {}


	// Implements Image.
	virtual uint16_t width() const {return surf_->width();}
	virtual uint16_t height() const {return surf_->height();}
	virtual const string& hash() const {return hash_;}
	virtual Surface* surface() const {return surf_.get();}

private:
	const string hash_;
	boost::scoped_ptr<Surface> surf_;
};

const Image* new_in_memory_image(const string& hash, Surface* surf) {
	return new InMemoryImage(hash, surf);
}




