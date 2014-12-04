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

#include "graphic/image_transformations.h"

#include <string>

#include <SDL.h>
#include <boost/format.hpp>

#include "base/macros.h"
#include "graphic/color.h"
#include "graphic/graphic.h"
#include "graphic/texture.h"
#include "graphic/texture_cache.h"

using namespace std;

namespace {

// This table is used to transform colors.
uint32_t luminance_table_r[0x100];
uint32_t luminance_table_g[0x100];
uint32_t luminance_table_b[0x100];

// Encodes the given Image into the corresponding image for player color.
// Takes the neutral set of images and the player color mask.
Texture* make_playerclr_texture(Texture& original_texture,
                                         Texture& pcmask_texture,
                                         const RGBColor& color) {
	Texture* new_texture = new Texture(original_texture.width(), original_texture.height());

	const SDL_PixelFormat & fmt = original_texture.format();
	const SDL_PixelFormat & fmt_pc = pcmask_texture.format();
	const SDL_PixelFormat & destfmt = new_texture->format();

	original_texture.lock(Surface::Lock_Normal);
	pcmask_texture.lock(Surface::Lock_Normal);
	new_texture->lock(Surface::Lock_Discard);
	// This could be done significantly faster, but since we
	// cache the result, let's keep it simple for now.
	for (uint32_t y = 0; y < original_texture.height(); ++y) {
		for (uint32_t x = 0; x < original_texture.width(); ++x) {
			RGBAColor source;
			RGBAColor mask;
			RGBAColor product;

			source.set(fmt, original_texture.get_pixel(x, y));
			mask.set(fmt_pc, pcmask_texture.get_pixel(x, y));

			if
				(uint32_t const influence =
				 static_cast<uint32_t>(mask.r) * mask.a)
				{
					uint32_t const intensity =
						(luminance_table_r[source.r] +
						 luminance_table_g[source.g] +
						 luminance_table_b[source.b] +
						 8388608U) //  compensate for truncation:  .5 * 2^24
						>> 24;
					RGBAColor plrclr;

					plrclr.r = (color.r * intensity) >> 8;
					plrclr.g = (color.g * intensity) >> 8;
					plrclr.b = (color.b * intensity) >> 8;

					product.r =
						(plrclr.r * influence + source.r * (65536 - influence)) >> 16;
					product.g =
						(plrclr.g * influence + source.g * (65536 - influence)) >> 16;
					product.b =
						(plrclr.b * influence + source.b * (65536 - influence)) >> 16;
					product.a = source.a;
				} else {
					product = source;
				}

			new_texture->set_pixel(x, y, product.map(destfmt));
		}
	}
	original_texture.unlock(Surface::Unlock_NoChange);
	pcmask_texture.unlock(Surface::Unlock_NoChange);
	new_texture->unlock(Surface::Unlock_Update);

	return new_texture;
}

// An Image implementation that is the transformation of another Image. Uses
// the TextureCache to avoid recalculating the transformation too often. No
// ownerships are taken.
class TransformedImage : public Image {
public:
	TransformedImage(const string& ghash, const Image& original, TextureCache* texture_cache) :
		hash_(ghash), original_(original), texture_cache_(texture_cache) {}
	virtual ~TransformedImage() {}

	// Implements Image.
	uint16_t width() const override {return original_.width();}
	uint16_t height() const override {return original_.height();}
	const string& hash() const override {return hash_;}
	Texture* texture() const override {
		Texture* texture = texture_cache_->get(hash_);
		if (texture)
			return texture;

		texture = recalculate_texture();
		texture_cache_->insert(hash_, texture, true);
		return texture;
	}

	virtual Texture* recalculate_texture() const = 0;

protected:
	const string hash_;
	const Image& original_;
	TextureCache* const texture_cache_;  // not owned
};

// A copy with applied player colors. Also needs a mask - ownership is not
// taken.
class PlayerColoredImage : public TransformedImage {
public:
	PlayerColoredImage
		(const string& ghash, const Image& original,
		 TextureCache* texture_cache, const RGBColor& color, const Image& mask)
		: TransformedImage(ghash, original, texture_cache), color_(color), mask_(mask)
		{}
	virtual ~PlayerColoredImage() {}

	// Implements TransformedImage.
	Texture* recalculate_texture() const override {
		return make_playerclr_texture(*original_.texture(), *mask_.texture(), color_);
	}

private:
	const RGBColor& color_;
	const Image& mask_;
};

}

namespace ImageTransformations {

void initialize() {
	// Initialize the table used to create grayed image
	for
		(uint32_t i = 0, r = 0, g = 0, b = 0;
		 i < 0x100;
		 ++i, r += 5016388U, g += 9848226U, b += 1912603U)
		{
			luminance_table_r[i] = r;
			luminance_table_g[i] = g;
			luminance_table_b[i] = b;
		}
}

const Image* player_colored(const RGBColor& clr, const Image* original, const Image* mask) {
	const string new_hash =
		(boost::format("%s:%02x%02x%02x") % original->hash() % static_cast<int>(clr.r) %
		 static_cast<int>(clr.g) % static_cast<int>(clr.b))
			.str();
	if (g_gr->images().has(new_hash))
		return g_gr->images().get(new_hash);
	return
		g_gr->images().insert
			(new PlayerColoredImage(new_hash, *original, &g_gr->textures(), clr, *mask));
}

}  // namespace ImageTransformations
