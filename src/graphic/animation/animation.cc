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
#include "scripting/lua_table.h"
#include "sound/note_sound.h"
#include "sound/sound_handler.h"

namespace {

// Parses an array { 12, 23 } into a point.
void get_point(const LuaTable& table, Vector2i* p) {
	std::vector<int> pts = table.array_entries<int>();
	if (pts.size() != 2) {
		throw wexception("Expected 2 entries, but got %" PRIuS ".", pts.size());
	}
	p->x = pts[0];
	p->y = pts[1];
}

} // namespace

const std::set<float> Animation::kSupportedScales { 0.5, 1, 2, 4};

Animation::Animation(const LuaTable& table) :
	hotspot_(Vector2i::zero()),
	frametime_(table.has_key("fps") ? (1000 / get_positive_int(table, "fps")) : kFrameLength),
	play_once_(table.has_key("play_once") ? table.get_bool("play_once") : false) {
	try {
		// Sound
		if (table.has_key("sound_effect")) {
			std::unique_ptr<LuaTable> sound_effects = table.get_table("sound_effect");

			const std::string name = sound_effects->get_string("name");
			const std::string directory = sound_effects->get_string("directory");
			sound_effect_ = directory + g_fs->file_separator() + name;
			g_sound_handler.load_fx_if_needed(directory, name, sound_effect_);
		}
		get_point(*table.get_table("hotspot"), &hotspot_);
	} catch (const LuaError& e) {
		throw wexception("Error in animation table: %s", e.what());
	}
	assert(frametime_ > 0);
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

void Animation::trigger_sound(uint32_t time, uint32_t stereo_position) const {
	if (sound_effect_.empty()) {
		return;
	}
	if (current_frame(time) == 0) {
		Notifications::publish(NoteSound(sound_effect_, stereo_position, 1));
	}
}
