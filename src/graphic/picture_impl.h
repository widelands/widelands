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

#ifndef PICTURE_IMPL_H
#define PICTURE_IMPL_H

#include "picture.h"

class IBlitableSurface;

// NOCOM(#sirver): document this
class ImageImpl : public IPicture {
public:
	virtual ~ImageImpl() {}

	// Implements IPicture.
	virtual uint32_t get_w() const = 0;
	virtual uint32_t get_h() const = 0;
	virtual const std::string& hash() const = 0;
	virtual IBlitableSurface* surface() const = 0;
};

#endif /* end of include guard: PICTURE_IMPL_H */

