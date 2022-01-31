/*
 * Copyright (C) 2017-2022 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXT_TEXTURE_CACHE_H
#define WL_GRAPHIC_TEXT_TEXTURE_CACHE_H

#include <memory>

#include "graphic/image.h"
#include "graphic/text/transient_cache.h"

class TextureCache : public TransientCache<Image> {
public:
	explicit TextureCache(uint32_t max_size_in_bytes) : TransientCache<Image>(max_size_in_bytes) {
	}

	std::shared_ptr<const Image> insert(const std::string& hash,
	                                    std::shared_ptr<const Image> entry) override {
		return TransientCache<Image>::insert(hash, entry, entry->width() * entry->height() * 4);
	}
};

#endif  // end of include guard: WL_GRAPHIC_TEXT_TEXTURE_CACHE_H
