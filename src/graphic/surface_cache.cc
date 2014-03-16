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

#include "graphic/surface_cache.h"

#include <cassert>
#include <list>
#include <map>
#include <memory>

#include <SDL.h>

#include "graphic/surface.h"

using namespace std;

// I took inspiration from http://timday.bitbucket.org/lru.html, but our use
// case here is a little different.
namespace  {
class SurfaceCacheImpl : public SurfaceCache {
public:
	SurfaceCacheImpl(uint32_t max_transient_memory) :
		max_transient_memory_(max_transient_memory), used_transient_memory_(0) {}
	virtual ~SurfaceCacheImpl();

	// Implements SurfaceCache.
	virtual void flush() override;
	virtual Surface* get(const string& hash) override;
	virtual Surface* insert(const string& hash, Surface*, bool) override;

private:
	void drop();

	typedef list<string> AccessHistory;
	struct Entry {
		Entry(Surface* gs, const AccessHistory::iterator& it, bool transient) :
			surface(gs), is_transient(transient), last_access(SDL_GetTicks()), list_iterator(it) {}

		std::unique_ptr<Surface> surface;
		bool is_transient;
		uint32_t last_access;  // Mainly for debugging and analysis.
		const AccessHistory::iterator list_iterator;  // Only valid if is_transient is true.
	};
	typedef map<string, Entry*> Container;

	uint32_t max_transient_memory_;
	uint32_t used_transient_memory_;
	Container entries_;
	AccessHistory access_history_;
};

SurfaceCacheImpl::~SurfaceCacheImpl() {
	flush();
}

void SurfaceCacheImpl::flush() {
	for (Container::iterator it = entries_.begin(); it != entries_.end(); ++it) {
		delete it->second;
	}
	entries_.clear();
	access_history_.clear();
	used_transient_memory_ = 0;
}

Surface* SurfaceCacheImpl::get(const string& hash) {
	const Container::iterator it = entries_.find(hash);
	if (it == entries_.end())
		return nullptr;

	// Move this to the back of the access list to signal that we have used this
	// recently and update last access time.
	if (it->second->is_transient) {
		access_history_.splice(access_history_.end(), access_history_, it->second->list_iterator);
		it->second->last_access = SDL_GetTicks();
	}
	return it->second->surface.get();
}

Surface* SurfaceCacheImpl::insert(const string& hash, Surface* surf, bool transient) {
	assert(entries_.find(hash) == entries_.end());

	if (transient) {
		const uint32_t surface_size = surf->width() * surf->height() * 4;
		while (used_transient_memory_ + surface_size > max_transient_memory_) {
			drop();
		}

		// Record hash as most-recently-used.
		AccessHistory::iterator it = access_history_.insert(access_history_.end(), hash);
		used_transient_memory_ += surface_size;
		entries_.insert(make_pair(hash, new Entry(surf, it, true)));
	} else {
		entries_.insert(make_pair(hash, new Entry(surf, access_history_.end(), false)));
	}

	return surf;
}

void SurfaceCacheImpl::drop() {
	assert(!access_history_.empty());

	// Identify least recently used key
	const Container::iterator it = entries_.find(access_history_.front());
	assert(it != entries_.end());
	assert(it->second->is_transient);

	const uint32_t surface_size = it->second->surface->width() * it->second->surface->height() * 4;
	used_transient_memory_ -= surface_size;

	// Erase both elements to completely purge record
	delete it->second;
	entries_.erase(it);
	access_history_.pop_front();
}

}  // namespace

SurfaceCache* create_surface_cache(uint32_t transient_memory_in_bytes) {
	return new SurfaceCacheImpl(transient_memory_in_bytes);
}

