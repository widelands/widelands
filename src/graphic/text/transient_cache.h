/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_GRAPHIC_TEXT_TRANSIENT_CACHE_H
#define WL_GRAPHIC_TEXT_TRANSIENT_CACHE_H

#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <string>

#include <SDL_timer.h>

#include "base/macros.h"

// The implementation took inspiration from
// https://timday.bitbucket.io/lru.html, but our use case here is a little
// different.

/// Caches transient rendered text. The entries will be kept until the memory limit is reached,
/// then the stalest entries will be deleted to make room for new entries.
///
/// We use shared_ptr so that other objects can hold on to the textures if they need them more
/// permanently.
template <typename T> class TransientCache {
public:
	/// Create a new cache in which the combined data size for all transient entries is always below
	/// the 'max_size_in_arbitrary_unit'.
	explicit TransientCache(uint32_t max_size_in_arbitrary_unit);
	virtual ~TransientCache();

	/// Deletes all entries in the cache, leaving it as if it were just created.
	void flush();

	/// Returns an entry if it is cached, nullptr otherwise.
	std::shared_ptr<const T> get(const std::string& hash);

	/// Inserts this entry of type T into the cache. Returns the given T for convenience.
	/// When overriding this function, calculate the size of 'entry' and then call
	/// insert(hash, entry, entry_size_in_size_unit).
	virtual std::shared_ptr<const T> insert(const std::string& hash,
	                                        std::shared_ptr<const T> entry) = 0;

protected:
	/// Inserts this entry of type T into the cache. asserts() that there is no entry with this hash
	/// already cached. Returns the given T for convenience.
	std::shared_ptr<const T> insert(const std::string& hash,
	                                std::shared_ptr<const T> entry,
	                                uint32_t entry_size_in_size_unit);

private:
	/// Drop the oldest entry
	void drop();

	using AccessHistory = std::list<std::string>;
	struct Entry {
		std::shared_ptr<const T> entry;
		uint32_t size;
		uint32_t last_access;  // Mainly for debugging and analysis.
		const AccessHistory::iterator list_iterator;
	};

	uint32_t max_size_in_size_unit_;
	uint32_t size_in_size_unit_;
	std::map<std::string, Entry> entries_;
	AccessHistory access_history_;

	DISALLOW_COPY_AND_ASSIGN(TransientCache);
};

// Implementation

template <typename T>
TransientCache<T>::TransientCache(uint32_t max_size_in_arbitrary_unit)
   : max_size_in_size_unit_(max_size_in_arbitrary_unit), size_in_size_unit_(0) {
}
template <typename T> TransientCache<T>::~TransientCache() {
	flush();
}

template <typename T> void TransientCache<T>::flush() {
	access_history_.clear();
	size_in_size_unit_ = 0;
	entries_.clear();
}

/// Returns an entry if it is cached, nullptr otherwise.
template <typename T> std::shared_ptr<const T> TransientCache<T>::get(const std::string& hash) {
	const auto it = entries_.find(hash);
	if (it == entries_.end()) {
		return std::shared_ptr<const T>(nullptr);
	}

	// Move this to the back of the access list to signal that we have used this
	// recently and update last access time.
	access_history_.splice(access_history_.end(), access_history_, it->second.list_iterator);
	it->second.last_access = SDL_GetTicks();
	return it->second.entry;
}

template <typename T>
std::shared_ptr<const T> TransientCache<T>::insert(const std::string& hash,
                                                   std::shared_ptr<const T> entry,
                                                   uint32_t entry_size_in_size_unit) {
	assert(entries_.find(hash) == entries_.end());

	while (!entries_.empty() &&
	       size_in_size_unit_ + entry_size_in_size_unit > max_size_in_size_unit_) {
		drop();
	}

	// Record hash as most-recently-used.
	AccessHistory::iterator it = access_history_.insert(access_history_.end(), hash);
	size_in_size_unit_ += entry_size_in_size_unit;
	return entries_
	   .insert(make_pair(hash, Entry{std::move(entry), entry_size_in_size_unit, SDL_GetTicks(), it}))
	   .first->second.entry;
}

template <typename T> void TransientCache<T>::drop() {
	assert(!access_history_.empty());

	// Identify least recently used key
	const auto it = entries_.find(access_history_.front());
	assert(it != entries_.end());

	size_in_size_unit_ -= it->second.size;
	// TODO(GunChleoc): Remove the following line once everything is converted to the new font
	// renderer and all testing has been done.
	// log("TransientCache: Dropping %d bytes, new size %d. Hash: %s\n", it->second.size,
	//    size_in_size_unit_, it->first.c_str());

	// Erase both elements to completely purge record
	entries_.erase(it);
	access_history_.pop_front();
}

#endif  // end of include guard: WL_GRAPHIC_TEXT_TRANSIENT_CACHE_H
