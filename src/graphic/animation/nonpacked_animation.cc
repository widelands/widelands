/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "graphic/animation/nonpacked_animation.h"

#include <cassert>
#include <cstdio>
#include <limits>
#include <memory>

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>

#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/animation/animation.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/playercolor.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_table.h"

namespace {
const std::set<float> kSupportedScales { 0.5, 1, 2, 4};
} // namespace

NonPackedAnimation::MipMapEntry::MipMapEntry(float scale, const LuaTable& table) : hasplrclrs(false) {
	if (scale <= 0.0f) {
		throw wexception("Animation scales must be positive numbers. Found %.2f", scale);
	}

	// TODO(GunChleoc): We want to rename these from "pictures" to "files", because we'll have spritesheets etc. in the future, and this naming will be clearer.
	// We don't want to convert them in bulk right now though - it will take care of itself as we convert to mipmaps.
	image_files = (table.has_key("files") ? table.get_table("files") : table.get_table("pictures"))->array_entries<std::string>();

	if (image_files.empty()) {
		throw wexception("Animation without image files. For a scale of 1.0, the template should look similar to this:"
		                 " 'directory/idle_1_??.png' for 'directory/idle_1_00.png' etc.");
	}

	for (std::string image_file : image_files) {
		boost::replace_last(image_file, ".png", "_pc.png");
		if (g_fs->file_exists(image_file)) {
			hasplrclrs = true;
			pc_mask_image_files.push_back(image_file);
		} else if (hasplrclrs) {
			throw wexception("Animation is missing player color file: %s", image_file.c_str());
		}
	}

	assert(!image_files.empty());
	assert(pc_mask_image_files.size() == image_files.size() || pc_mask_image_files.empty());
}

NonPackedAnimation::NonPackedAnimation(const LuaTable& table)
   : Animation(table, Animation::Type::kNonPacked) {
	try {
		// Images
		if (table.has_key("mipmap")) {
			std::unique_ptr<LuaTable> mipmaps_table = table.get_table("mipmap");
			for (const int key : mipmaps_table->keys<int>()) {
				std::unique_ptr<LuaTable> current_scale_table = mipmaps_table->get_table(key);
				const float current_scale = current_scale_table->get_double("scale");
				if (kSupportedScales.count(current_scale) != 1) {
					std::string supported_scales = "";
					for (const float supported_scale : kSupportedScales) {
						supported_scales = (boost::format("%s %.1f") % supported_scales % supported_scale).str();
					}
					throw wexception(
						"Animation has unsupported scale '%.1f' in mipmap - supported scales are:%s", current_scale, supported_scales.c_str());
				}
				mipmaps_.insert(std::make_pair(current_scale, std::unique_ptr<MipMapEntry>(new MipMapEntry(current_scale, *current_scale_table))));
			}
		} else {
			mipmaps_.insert(std::make_pair(1.0f, std::unique_ptr<MipMapEntry>(new MipMapEntry(1.0f, table))));
		}

		// Frames
		nr_frames_ = mipmaps_.begin()->second->image_files.size();
		if (nr_frames_ == 1 && frametime() != kFrameLength) {
			throw wexception(
				"Animation with one picture %s must not have 'fps'", mipmaps_.begin()->second->image_files[0].c_str());
		}

		// Perform some checks to make sure that the data is complete and consistent
		const bool should_have_playercolor = mipmaps_.begin()->second->hasplrclrs;
		for (const auto& mipmap : mipmaps_) {
			if (mipmap.second->image_files.size() != nr_frames_) {
				throw wexception("Mismatched number of images for different scales in animation table: %" PRIuS " vs. %u at scale %.2f",
									  mipmap.second->image_files.size(),
									  nr_frames_,
									  mipmap.first);
			}
			if (mipmap.second->hasplrclrs != should_have_playercolor) {
				throw wexception("Mismatched existence of player colors in animation table for scales %.2f and %.2f",
									  mipmaps_.begin()->first,
									  mipmap.first);
			}
		}
		if (mipmaps_.count(1.0f) != 1) {
			throw wexception("All animations must provide images for the neutral scale (1.0)");
		}
	} catch (const LuaError& e) {
		throw wexception("Error in animation table: %s", e.what());
	}
	assert(nr_frames_ > 0);
}

float NonPackedAnimation::find_best_scale(float scale) const {
	assert(!mipmaps_.empty());
	float result = mipmaps_.begin()->first;
	for (const auto& mipmap : mipmaps_) {
		// The map is reverse sorted, so we can break as soon as we are lower than the wanted scale
		if (mipmap.first < scale) {
			break;
		}
		result = mipmap.first;
	}
	return result;
}

void NonPackedAnimation::ensure_graphics_are_loaded() const {
	if (mipmaps_.begin()->second->frames.empty()) {
		const_cast<NonPackedAnimation*>(this)->load_graphics();
	}
}

void NonPackedAnimation::load_graphics() {
	for (const auto& entry : mipmaps_) {
		MipMapEntry* mipmap = entry.second.get();

		if (mipmap->image_files.empty()) {
			throw wexception("animation without image files at promised scale %.2f.", entry.first);
		}
		if (mipmap->pc_mask_image_files.size() && mipmap->pc_mask_image_files.size() != mipmap->image_files.size()) {
			throw wexception("animation has %" PRIuS " frames but playercolor mask has %" PRIuS " frames for scale %.2f",
								  mipmap->image_files.size(), mipmap->pc_mask_image_files.size(), entry.first);
		}

		for (const std::string& filename : mipmap->image_files) {
			const Image* image = g_gr->images().get(filename);
			if (mipmap->frames.size() &&
				 (mipmap->frames[0]->width() != image->width() || mipmap->frames[0]->height() != image->height())) {
				throw wexception("wrong size: (%u, %u), should be (%u, %u) like the first frame",
									  image->width(), image->height(), mipmap->frames[0]->width(),
									  mipmap->frames[0]->height());
			}
			mipmap->frames.push_back(image);
		}

		for (const std::string& filename : mipmap->pc_mask_image_files) {
			// TODO(unknown): Do not load playercolor mask as opengl texture or use it as
			//     opengl texture.
			const Image* pc_image = g_gr->images().get(filename);
			if (mipmap->frames[0]->width() != pc_image->width() || mipmap->frames[0]->height() != pc_image->height()) {
				// TODO(unknown): see bug #1324642
				throw wexception("playercolor mask has wrong size: (%u, %u), should "
									  "be (%u, %u) like the animation frame",
									  pc_image->width(), pc_image->height(), mipmap->frames[0]->width(),
									  mipmap->frames[0]->height());
			}
			mipmap->pcmasks.push_back(pc_image);
		}
	}
}

float NonPackedAnimation::height() const {
	ensure_graphics_are_loaded();
	return mipmaps_.at(1.0f)->frames.at(0)->height();
}

float NonPackedAnimation::width() const {
	ensure_graphics_are_loaded();
	return mipmaps_.at(1.0f)->frames.at(0)->width();
}

std::vector<const Image*> NonPackedAnimation::images(float scale) const {
	ensure_graphics_are_loaded();
	return mipmaps_.at(scale)->frames;
}

std::vector<const Image*> NonPackedAnimation::pc_masks(float scale) const {
	ensure_graphics_are_loaded();
	return mipmaps_.at(scale)->pcmasks;
}

const Image* NonPackedAnimation::representative_image(const RGBColor* clr) const {
	const MipMapEntry& mipmap = *mipmaps_.at(1.0f);
	std::vector<std::string> images = mipmap.image_files;
	assert(!images.empty());
	const Image* image = (mipmap.hasplrclrs && clr) ? playercolor_image(*clr, images[0]) :
	                                            g_gr->images().get(images[0]);

	const int w = image->width();
	const int h = image->height();

	Texture* rv = new Texture(w, h);
	rv->blit(
	   Rectf(0.f, 0.f, w, h), *image, Rectf(0.f, 0.f, w, h), 1., BlendMode::Copy);
	return rv;
}

// TODO(GunChleoc): This is only here for the font renderers.
const std::string& NonPackedAnimation::representative_image_filename() const {
	return mipmaps_.at(1.0f)->image_files[0];
}


Rectf NonPackedAnimation::source_rectangle(const int percent_from_bottom, float scale) const {
	ensure_graphics_are_loaded();
	const Image* first_frame = mipmaps_.at(find_best_scale(scale))->frames.at(0);
	float h = percent_from_bottom * first_frame->height() / 100;
	return Rectf(0.f, first_frame->height() - h, first_frame->width(), h);
}

Rectf NonPackedAnimation::destination_rectangle(const Vector2f& position,
                                                const Rectf& source_rect,
                                                const float scale) const {
	ensure_graphics_are_loaded();
	const float best_scale = find_best_scale(scale);
	// Using floor + ceil for pixel perfect positioning
	return Rectf(std::floor(position.x - hotspot().x * scale - source_rect.x),
	             std::floor(position.y - hotspot().y * scale - source_rect.y),
	             std::ceil(source_rect.w * scale / best_scale), std::ceil(source_rect.h * scale / best_scale));
}

void NonPackedAnimation::blit(uint32_t time,
                              const Rectf& source_rect,
                              const Rectf& destination_rect,
                              const RGBColor* clr,
                              Surface* target, float scale) const {
	ensure_graphics_are_loaded();
	assert(target);
	const uint32_t idx = current_frame(time);
	assert(idx < nr_frames());

	const MipMapEntry& mipmap = *mipmaps_.at(find_best_scale(scale));
	if (!mipmap.hasplrclrs || clr == nullptr) {
		target->blit(destination_rect, *mipmap.frames.at(idx), source_rect, 1., BlendMode::UseAlpha);
	} else {
		target->blit_blended(
		   destination_rect, *mipmap.frames.at(idx), *mipmap.pcmasks.at(idx), source_rect, *clr);
	}
	// TODO(GunChleoc): Stereo position would be nice.
	trigger_sound(time, 128);
}
