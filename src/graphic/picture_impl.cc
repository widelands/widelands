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

#include <boost/scoped_ptr.hpp>
#include "surface.h"

#include "picture_impl.h"

// TODO(sirver): the name of the file is wrong!
namespace {
// TODO(sirver): document this
class SimpleImageImpl : public ImageImpl {
public:
	explicit SimpleImageImpl(Surface* surf) : surf_(surf) {}
	virtual ~SimpleImageImpl() {}

	virtual uint32_t get_w() const {return surf_->get_w();}
	virtual uint32_t get_h() const {return surf_->get_h();}

	Surface& surface() const {return *surf_.get();}

private:
	boost::scoped_ptr<Surface> surf_;
};
}  // namespace


ImageImpl* new_picture(IBlitableSurface* surf) {
	return new SimpleImageImpl(static_cast<Surface*>(surf));
}

