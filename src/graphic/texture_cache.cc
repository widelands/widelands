/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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

#include "graphic/texture_cache.h"

#include <memory>

#include <SDL.h>
#include <stdint.h>

#include "graphic/image.h"

// The implementation took inspiration from
// http://timday.bitbucket.org/lru.html, but our use case here is a little
// different.

TextureCache::TextureCache(uint32_t max_size_in_bytes)
   : max_size_in_bytes_(max_size_in_bytes), size_in_bytes_(0) {
}

TextureCache::~TextureCache() {
	flush();
}

void TextureCache::flush() {
	entries_.clear();
	access_history_.clear();
	size_in_bytes_ = 0;
}

const Image* TextureCache::get(const std::string& hash) {
	const auto it = entries_.find(hash);
	if (it == entries_.end())
		return nullptr;

	// Move this to the back of the access list to signal that we have used this
	// recently and update last access time.
	access_history_.splice(access_history_.end(), access_history_, it->second.list_iterator);
	it->second.last_access = SDL_GetTicks();
	return it->second.texture.get();
}

const Image* TextureCache::insert(const std::string& hash, std::unique_ptr<const Image> texture) {
	assert(entries_.find(hash) == entries_.end());

	const uint32_t texture_size = texture->width() * texture->height() * 4;
	while (size_in_bytes_ + texture_size > max_size_in_bytes_) {
		drop();
	}

	// Record hash as most-recently-used.
	AccessHistory::iterator it = access_history_.insert(access_history_.end(), hash);
	size_in_bytes_ += texture_size;
	return entries_.insert(make_pair(hash, Entry{std::move(texture), SDL_GetTicks(), it}))
	   .first->second.texture.get();
}

void TextureCache::drop() {
	assert(!access_history_.empty());

	// Identify least recently used key
	const auto it = entries_.find(access_history_.front());
	assert(it != entries_.end());

	const uint32_t texture_size = it->second.texture->width() * it->second.texture->height() * 4;
	size_in_bytes_ -= texture_size;

	// Erase both elements to completely purge record
	entries_.erase(it);
	access_history_.pop_front();
}
