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

#include <map>
#include <list>

#include <SDL.h>
#include <boost/scoped_ptr.hpp>

#include "surface.h"
#include "log.h"

#include "surface_cache.h"

using namespace std;

// I took inspiration from http://timday.bitbucket.org/lru.html, but our use
// case here is a little different.
namespace  {
class SurfaceCacheImpl : public SurfaceCache {
public:
	SurfaceCacheImpl(uint32_t max_memory) :
		max_memory_(max_memory), used_memory_(0) {}
	virtual ~SurfaceCacheImpl();

	// Implements SurfaceCache.
	virtual void flush();
	virtual Surface* get(const std::string& hash);
	virtual Surface* insert(const std::string& hash, Surface*);

private:
	void drop();

	typedef list<string> AccessHistory;
	struct Entry {
		Entry(Surface* gs, const AccessHistory::iterator& it) :
			surface(gs), last_access(SDL_GetTicks()), list_iterator(it) {}

		boost::scoped_ptr<Surface> surface;
		uint32_t last_access;  // Mainly for debugging and analysis.
		const AccessHistory::iterator list_iterator;
	};
	typedef map<string, Entry*> Container;

	uint32_t max_memory_;
	uint32_t used_memory_;
	Container map_;
	AccessHistory hist_;
};

SurfaceCacheImpl::~SurfaceCacheImpl() {
	flush();
}

void SurfaceCacheImpl::flush() {
	for (Container::iterator it = map_.begin(); it != map_.end(); ++it) {
		delete it->second;
	}
	map_.clear();
	hist_.clear();
	used_memory_ = 0;
}

Surface* SurfaceCacheImpl::get(const std::string& hash) {
	const Container::iterator it = map_.find(hash);
	if (it == map_.end())
		return NULL;

	// Move this to the back of the access list to signal that we have used this
	// recently and update last access time.
	hist_.splice(hist_.end(), hist_, it->second->list_iterator);
	it->second->last_access = SDL_GetTicks();
	return it->second->surface.get();
}

Surface* SurfaceCacheImpl::insert(const std::string& hash, Surface* surf) {
	assert(map_.find(hash) == map_.end());

	uint32_t surface_size = surf->width() * surf->height() * 4;
	while (used_memory_ + surface_size > max_memory_)
		drop();

	// Record hash as most-recently-used.
	AccessHistory::iterator it = hist_.insert(hist_.end(), hash);
	used_memory_ += surface_size;
	map_.insert(make_pair(hash, new Entry(surf, it)));

	log("SurfaceCache: inserted %s, now using %.2f mb.\n", hash.c_str(), used_memory_ / 1048576.0);
	return surf;
}

void SurfaceCacheImpl::drop() {
	assert(!hist_.empty());

	// Identify least recently used key
	const Container::iterator it = map_.find(hist_.front());
	assert(it != map_.end());

	uint32_t surface_size = it->second->surface->width() * it->second->surface->height() * 4;
	used_memory_ -= surface_size;

	log
		("SurfaceCache: dropping %s, which was unused for %.2f sec. Now using %.2f mb.\n",
			hist_.front().c_str(), (SDL_GetTicks() - it->second->last_access) / 1000., used_memory_ / 1048576.0);

	// Erase both elements to completely purge record
	delete it->second;
	map_.erase(it);
	hist_.pop_front();
}

}  // namespace

SurfaceCache* create_surface_cache(uint32_t memory) {
	return new SurfaceCacheImpl(memory);
}

