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

#include "graphic/animation/animation.h"

#include <cassert>
#include <memory>

#include "base/vector.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"
#include "sound/note_sound.h"
#include "sound/sound_handler.h"

const std::set<float> Animation::kSupportedScales { 0.5, 1, 2, 4};

Animation::Animation(const LuaTable& table) :
	representative_frame_(table.has_key("representative_frame") ? table.get_int("representative_frame") : 0),
	hotspot_(table.get_vector<std::string, int>("hotspot")),
	frametime_(table.has_key("fps") ? (1000 / get_positive_int(table, "fps")) : kFrameLength),
	play_once_(table.has_key("play_once") ? table.get_bool("play_once") : false),
	sound_effect_(kNoSoundEffect),
	sound_priority_(kFxPriorityLowest) {
	try {
		// Sound
		if (table.has_key("sound_effect")) {
			std::unique_ptr<LuaTable> sound_effects = table.get_table("sound_effect");
			sound_effect_ =
			   SoundHandler::register_fx(SoundType::kAmbient, sound_effects->get_string("path"));

			if (sound_effects->has_key<std::string>("priority")) {
				sound_priority_ = sound_effects->get_int("priority");
			}

			if (sound_priority_ < kFxPriorityLowest) {
				throw Widelands::GameDataError(
				   "Minmum priority for sounds is %d, but only %d was specified for %s",
				   kFxPriorityLowest, sound_priority_, sound_effects->get_string("path").c_str());
			}
		}
	} catch (const LuaError& e) {
		throw wexception("Error in animation table: %s", e.what());
	}
	assert(frametime_ > 0);
}

Rectf Animation::destination_rectangle(const Vector2f& position,
                                                const Rectf& source_rect,
                                                const float scale) const {
	const float best_scale = find_best_scale(scale);
	// Using floor + ceil for pixel perfect positioning
	return Rectf(std::floor(position.x - hotspot().x * scale - source_rect.x),
	             std::floor(position.y - hotspot().y * scale - source_rect.y),
	             std::ceil(source_rect.w * scale / best_scale), std::ceil(source_rect.h * scale / best_scale));
}

uint16_t Animation::nr_frames() const {
	assert(nr_frames_ > 0);
	return nr_frames_;
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

// TODO(unknown): The chosen semantics of animation sound effects is problematic:
// What if the game runs very slowly or very quickly?
void Animation::trigger_sound(uint32_t time, const Widelands::Coords& coords) const {
	if (sound_effect_ == kNoSoundEffect || coords == Widelands::Coords::null()) {
		return;
	}
	if (current_frame(time) == 0) {
		Notifications::publish(
		   NoteSound(SoundType::kAmbient, sound_effect_, coords, sound_priority_));
	}
}

void Animation::load_sounds() const {
	if (sound_effect_ != kNoSoundEffect && !SoundHandler::is_backend_disabled()) {
		g_sh->load_fx(SoundType::kAmbient, sound_effect_);
	}
}

int Animation::representative_frame() const {
	return representative_frame_;
}
