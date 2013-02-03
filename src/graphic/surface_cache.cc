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

#include "surface_cache.h"

typedef map<string, Surface*> SurfaceMap;
class SurfaceCache::Impl {
	SurfaceMap surfaces;
};

SurfaceCache::SurfaceCache() : p_(new SurfaceCache::Impl()) {}
SurfaceCache::~SurfaceCache() {}

// NOCOM(#sirver): implement clever hashing here.
Surface* SurfaceCache::get(const std::string& hash) {
	ImageMap::const_iterator it = p_.surfaces.find(hash);
	if (it == p_.surfaces.end()) {
		return NULL;
	}
	return it->second;
}
Surface* SurfaceCache::insert(const std::string& hash, Surface* surf) {
	images_.insert(make_pair(hash, surf));
	return surf;
}

