/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "graphic/animation/animation.h"

#include <cassert>
#include <memory>

#include "base/log.h"
#include "base/math.h"
#include "base/vector.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"
#include "sound/note_sound.h"
#include "sound/sound_handler.h"

const std::map<float, std::string> Animation::kSupportedScales{
   {0.5, "_0.5"}, {1, "_1"}, {2, "_2"}, {4, "_4"}};

Animation::MipMapEntry::MipMapEntry() : has_playercolor_masks(false) {
}

Animation::Animation(const LuaTable& table)
   : nr_frames_(0),
     representative_frame_(
        table.has_key("representative_frame") ? table.get_int("representative_frame") : 0),
     hotspot_(table.get_vector<std::string, int>("hotspot")),
     frametime_(table.has_key("fps") ? (1000 / get_positive_int(table, "fps")) : kFrameLength),
     play_once_(table.has_key("play_once") ? table.get_bool("play_once") : false),
     sound_effect_(kNoSoundEffect),
     sound_priority_(kFxPriorityLowest),
     sound_allow_multiple_(false) {
	try {
		// Sound
		if (table.has_key("sound_effect")) {
			std::unique_ptr<LuaTable> sound_effects = table.get_table("sound_effect");
			sound_effect_ =
			   SoundHandler::register_fx(SoundType::kAmbient, sound_effects->get_string("path"));

			try {
				sound_priority_ = math::read_percent_to_int(sound_effects->get_string("priority"));
			} catch (const std::exception&) {
				// TODO(GunChleoc): Compatibility, remove try-catch after v1.0
				log_warn("Animation sound effect priority '%.2f' without percent symbol is deprecated",
				         sound_effects->get_double("priority"));
				sound_priority_ = std::round(100 * sound_effects->get_double("priority"));
			}

			if (sound_effects->has_key<std::string>("allow_multiple")) {
				sound_allow_multiple_ = sound_effects->get_bool("allow_multiple");
			}

			if (sound_priority_ < kFxPriorityLowest) {
				throw Widelands::GameDataError(
				   "Minimum priority for sounds is 0.01, but only %.2f was specified for %s",
				   sound_effects->get_double("priority"), sound_effects->get_string("path").c_str());
			}
		}
	} catch (const LuaError& e) {
		throw wexception("Error in animation table: %s", e.what());
	}
	assert(frametime_ > 0);
}

const Animation::MipMapEntry& Animation::mipmap_entry(float scale) const {
	assert(mipmaps_.count(scale) == 1);
	const MipMapEntry& mipmap = *mipmaps_.at(scale);
	mipmap.ensure_graphics_are_loaded();
	return mipmap;
}

Rectf Animation::source_rectangle(const int percent_from_bottom, float scale) const {
	const MipMapEntry& mipmap = mipmap_entry(find_best_scale(scale));
	const float h = percent_from_bottom * mipmap.height() / 100.f;
	// Using floor for pixel perfect positioning
	return Rectf(0.f, std::floor(mipmap.height() - h), mipmap.width(), h);
}

Rectf Animation::destination_rectangle(const Vector2f& position,
                                       const Rectf& source_rect,
                                       const float scale) const {
	const float best_scale = find_best_scale(scale);
	return Rectf(position.x - (hotspot_.x - source_rect.x / best_scale) * scale,
	             position.y - (hotspot_.y - source_rect.y / best_scale) * scale,
	             source_rect.w * scale / best_scale, source_rect.h * scale / best_scale);
}

uint16_t Animation::nr_frames() const {
	assert(nr_frames_ > 0);
	return nr_frames_;
}

int Animation::height() const {
	return mipmap_entry(1.0f).height();
}

int Animation::width() const {
	return mipmap_entry(1.0f).width();
}

uint32_t Animation::frametime() const {
	assert(frametime_ > 0);
	return frametime_;
}

const Vector2i& Animation::hotspot() const {
	return hotspot_;
}

uint32_t Animation::current_frame(uint32_t time) const {
	if (nr_frames() > 1) {
		return (play_once_ && time / frametime_ > static_cast<uint32_t>(nr_frames() - 1)) ?
                static_cast<uint32_t>(nr_frames() - 1) :
                time / frametime_ % nr_frames();
	}
	return 0;
}

void Animation::add_available_scales(const std::string& basename, const std::string& directory) {
	for (const auto& scale : kSupportedScales) {
		add_scale_if_files_present(basename, directory, scale.first, scale.second);
	}

	if (mipmaps_.count(1.0f) == 0) {
		// There might be only 1 scale
		add_scale_if_files_present(basename, directory, 1.0f, "");
		if (mipmaps_.count(1.0f) == 0) {
			// No files found at all
			throw Widelands::GameDataError(
			   "Animation in directory '%s' with basename '%s' has no images for mandatory "
			   "scale '1' in mipmap - supported scales are: 0.5, 1, 2, 4",
			   directory.c_str(), basename.c_str());
		}
	}
}

// TODO(unknown): The chosen semantics of animation sound effects is problematic:
// What if the game runs very slowly or very quickly?
void Animation::trigger_sound(uint32_t time, const Widelands::Coords& coords) const {
	if (sound_effect_ == kNoSoundEffect || coords == Widelands::Coords::null()) {
		return;
	}
	if (current_frame(time) == 0) {
		Notifications::publish(NoteSound(
		   SoundType::kAmbient, sound_effect_, coords, sound_priority_, sound_allow_multiple_));
	}
}

std::set<float> Animation::available_scales() const {
	std::set<float> result;
	for (const auto& scale : kSupportedScales) {
		if (mipmaps_.count(scale.first) == 1) {
			result.insert(scale.first);
		}
	}
	return result;
}

void Animation::blit(uint32_t time,
                     const Widelands::Coords& coords,
                     const Rectf& source_rect,
                     const Rectf& destination_rect,
                     const RGBColor* clr,
                     Surface* target,
                     float scale,
                     float opacity) const {
	mipmap_entry(find_best_scale(scale))
	   .blit(current_frame(time), source_rect, destination_rect, clr, target, opacity);
	trigger_sound(time, coords);
}

void Animation::load_default_scale_and_sounds() const {
	mipmaps_.at(1.0f)->ensure_graphics_are_loaded();
	if (sound_effect_ != kNoSoundEffect && !SoundHandler::is_backend_disabled()) {
		g_sh->load_fx(SoundType::kAmbient, sound_effect_);
	}
}

float Animation::find_best_scale(float scale) const {
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

int Animation::representative_frame() const {
	return representative_frame_;
}

std::vector<std::unique_ptr<const Texture>>
Animation::frame_textures(float scale, bool return_playercolor_masks) const {
	return mipmap_entry(scale).frame_textures(return_playercolor_masks);
}
