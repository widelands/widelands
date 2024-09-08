/*
 * Copyright (C) 2016-2024 by the Widelands Development Team
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

#include "graphic/playercolor.h"

#include <cassert>
#include <memory>

#include "base/string.h"
#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"

const Image* playercolor_image(const RGBColor& clr, const std::string& image_filename) {
	const std::string hash = image_filename + "+pc" + clr.hex_value();

	// Get from cache if we already have it
	if (g_image_cache->has(hash)) {
		return g_image_cache->get(hash, false);
	}

	// Check whether we have a player color mask
	std::string color_mask_filename = image_filename;
	replace_last(color_mask_filename, ".png", "_pc.png");
	if (!g_fs->file_exists(color_mask_filename)) {
		return g_image_cache->get(image_filename, false);
	}

	// Now calculate the image and add it to the cache
	const Image* image = g_image_cache->get(image_filename, false);
	const Image* color_mask = g_image_cache->get(color_mask_filename, false);
	const int w = image->width();
	const int h = image->height();
	Texture* pc_image = new Texture(w, h);
	pc_image->fill_rect(Rectf(0.f, 0.f, w, h), RGBAColor(0, 0, 0, 0));
	pc_image->blit_blended(Rectf(0.f, 0.f, w, h), *image, *color_mask, Rectf(0.f, 0.f, w, h), clr);
	g_image_cache->insert(hash, std::unique_ptr<const Texture>(pc_image));
	assert(g_image_cache->has(hash));
	return g_image_cache->get(hash, false);
}

const Image* playercolor_image(int player_number, const std::string& image_filename) {
	return playercolor_image(kPlayerColors[player_number], image_filename);
}

std::pair<const Image*, float>
playercolor_image(const RGBColor& clr, const std::string& image_filename, const float scale) {
	const uint8_t mipmap_bitset = g_image_cache->get_mipmap_bitset(image_filename);

	if (mipmap_bitset == 0) {  // No mipmaps
		return {playercolor_image(clr, image_filename), 1.f};
	}

	// Logic is the same as in InteractiveBase::get_buildhelp_overlay
	int best_s = -1;
	for (int s = ImageCache::kScalesCount - 1; s >= 0; --s) {
		if ((mipmap_bitset & (1 << s)) == 0) {
			// No mipmap at this scale
			continue;
		}

		if (best_s < 0) {
			best_s = s;
			continue;
		}

		if (ImageCache::kScales[s].first < scale) {
			break;
		}

		best_s = s;
	}

	assert(best_s >= 0);
	std::string resolved = image_filename;
	resolved.insert(resolved.rfind('.'), ImageCache::kScales[best_s].second);
	return {playercolor_image(clr, resolved), ImageCache::kScales[best_s].first};
}
