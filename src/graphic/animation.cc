/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "graphic/animation.h"

#include <cassert>
#include <cstdio>
#include <limits>
#include <memory>
#include <set>

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "graphic/diranimations.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/playercolor.h"
#include "graphic/texture.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"
#include "sound/note_sound.h"
#include "sound/sound_handler.h"

namespace {
// The mipmap scales supported by the engine.
// Ensure that this always matches supported_scales in data/scripting/mapobjects.lua.
const std::set<float> kSupportedScales{0.5, 1, 2, 4};

/**
 * Implements the Animation interface for an animation that is unpacked on disk, that
 * is every frame and every pc color frame is an singular file on disk.
 */
class NonPackedAnimation : public Animation {
public:
	struct MipMapEntry {
		explicit MipMapEntry(std::vector<std::string> files);

		// Loads the graphics if they are not yet loaded.
		void ensure_graphics_are_loaded() const;

		// Load the needed graphics from disk.
		void load_graphics();

		void blit(uint32_t idx,
		          const Rectf& source_rect,
		          const Rectf& destination_rect,
		          const RGBColor* clr,
		          Surface* target) const;

		// Whether this image set has player color masks provided
		bool has_playercolor_masks;

		// Image files on disk
		std::vector<std::string> image_files;

		// Loaded images for each frame
		std::vector<const Image*> frames;

	private:
		// Player color mask files on disk
		std::vector<std::string> playercolor_mask_image_files;

		// Loaded player color mask images for each frame
		std::vector<const Image*> playercolor_mask_frames;
	};

	~NonPackedAnimation() override {
	}
	explicit NonPackedAnimation(const LuaTable& table, const std::string& basename);

	// Implements Animation.
	float height() const override;
	Rectf source_rectangle(int percent_from_bottom, float scale) const override;
	Rectf destination_rectangle(const Vector2f& position,
	                            const Rectf& source_rect,
	                            float scale) const override;
	uint16_t nr_frames() const override;
	uint32_t frametime() const override;
	const Image* representative_image(const RGBColor* clr) const override;
	virtual void blit(uint32_t time,
	                  const Widelands::Coords& coords,
	                  const Rectf& source_rect,
	                  const Rectf& destination_rect,
	                  const RGBColor* clr,
	                  Surface* target,
	                  float scale) const override;
	void load_default_scale_and_sounds() const override;

private:
	float find_best_scale(float scale) const;

	// TODO(unknown): The chosen semantics of animation sound effects is problematic:
	// What if the game runs very slowly or very quickly?
	void trigger_sound(uint32_t framenumber, const Widelands::Coords& coords) const override;

	uint32_t current_frame(uint32_t time) const;

	uint32_t frametime_;
	uint16_t nr_frames_;

	Vector2i hotspot_;

	struct MipMapCompare {
		bool operator()(const float lhs, const float rhs) const {
			return lhs > rhs;
		}
	};
	std::map<float, std::unique_ptr<MipMapEntry>, MipMapCompare> mipmaps_;

	// ID of sound effect that will be played at frame 0.
	FxId sound_effect_;
	int32_t sound_priority_;
	bool play_once_;
};

/*
==============================================================================

NonPackedAnimation::MipMapEntry IMPLEMENTATION

==============================================================================
*/

NonPackedAnimation::MipMapEntry::MipMapEntry(std::vector<std::string> files)
   : has_playercolor_masks(false), image_files(files) {
	if (image_files.empty()) {
		throw Widelands::GameDataError(
		   "Animation without image files. For a scale of 1.0, the template should look similar to "
		   "this:"
		   " 'directory/idle_1_??.png' for 'directory/idle_1_00.png' etc.");
	}

	for (std::string image_file : image_files) {
		boost::replace_last(image_file, ".png", "_pc.png");
		if (g_fs->file_exists(image_file)) {
			has_playercolor_masks = true;
			playercolor_mask_image_files.push_back(image_file);
		} else if (has_playercolor_masks) {
			throw Widelands::GameDataError(
			   "Animation is missing player color file: %s", image_file.c_str());
		}
	}

	assert(!image_files.empty());
	assert(playercolor_mask_image_files.size() == image_files.size() ||
	       playercolor_mask_image_files.empty());
}

// Loads the graphics if they are not yet loaded.
void NonPackedAnimation::MipMapEntry::ensure_graphics_are_loaded() const {
	if (frames.empty()) {
		const_cast<MipMapEntry*>(this)->load_graphics();
	}
}

// Load the needed graphics from disk.
void NonPackedAnimation::MipMapEntry::load_graphics() {
	if (image_files.empty()) {
		throw Widelands::GameDataError("animation without image files.");
	}
	if (playercolor_mask_image_files.size() &&
	    playercolor_mask_image_files.size() != image_files.size()) {
		throw Widelands::GameDataError(
		   "animation has %" PRIuS " frames but playercolor mask has %" PRIuS
		   " frames. First image is %s",
		   image_files.size(), playercolor_mask_image_files.size(), image_files.front().c_str());
	}

	for (const std::string& filename : image_files) {
		const Image* image = g_gr->images().get(filename);
		if (frames.size() && (frames.front()->width() != image->width() ||
		                      frames.front()->height() != image->height())) {
			throw Widelands::GameDataError(
			   "wrong size: (%u, %u) for file %s, should be (%u, %u) like the first frame",
			   image->width(), image->height(), filename.c_str(), frames.front()->width(),
			   frames.front()->height());
		}
		frames.push_back(image);
	}

	for (const std::string& filename : playercolor_mask_image_files) {
		// TODO(unknown): Do not load playercolor mask as opengl texture or use it as
		//     opengl texture.
		const Image* pc_image = g_gr->images().get(filename);
		if (frames.front()->width() != pc_image->width() ||
		    frames.front()->height() != pc_image->height()) {
			throw Widelands::GameDataError("playercolor mask %s has wrong size: (%u, %u), should "
			                               "be (%u, %u) like the animation frame",
			                               filename.c_str(), pc_image->width(), pc_image->height(),
			                               frames.front()->width(), frames.front()->height());
		}
		playercolor_mask_frames.push_back(pc_image);
	}
}

void NonPackedAnimation::MipMapEntry::blit(uint32_t idx,
                                           const Rectf& source_rect,
                                           const Rectf& destination_rect,
                                           const RGBColor* clr,
                                           Surface* target) const {
	assert(!frames.empty());
	assert(target);
	assert(idx < frames.size());

	if (!has_playercolor_masks || clr == nullptr) {
		target->blit(destination_rect, *frames.at(idx), source_rect, 1., BlendMode::UseAlpha);
	} else {
		target->blit_blended(
		   destination_rect, *frames.at(idx), *playercolor_mask_frames.at(idx), source_rect, *clr);
	}
}

/*
==============================================================================

NonPackedAnimation IMPLEMENTATION

==============================================================================
*/

NonPackedAnimation::NonPackedAnimation(const LuaTable& table, const std::string& basename)
   : Animation(table.has_key("representative_frame") ? table.get_int("representative_frame") : 0),
     frametime_(FRAME_LENGTH),
     hotspot_(table.get_vector<std::string, int>("hotspot")),
     sound_effect_(kNoSoundEffect),
     sound_priority_(kFxPriorityLowest),
     play_once_(false) {
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

		// Repetition
		if (table.has_key("play_once")) {
			play_once_ = table.get_bool("play_once");
		}

		// Get image files
		if (table.has_key("pictures")) {
			// TODO(GunChleoc): Old code - remove this option once conversion has been completed
			mipmaps_.insert(std::make_pair(
			   1.0f, std::unique_ptr<MipMapEntry>(
			            new MipMapEntry(table.get_table("pictures")->array_entries<std::string>()))));
		} else {
			if (basename.empty() || !table.has_key("directory")) {
				throw Widelands::GameDataError(
				   "Animation did not define both a basename and a directory for its image files");
			}
			const std::string directory = table.get_string("directory");

			// List files for the given scale, and if we have any, add a mipmap entry for them.
			auto add_scale = [this, basename, directory](
			                    float scale_as_float, const std::string& scale_as_string) {
				std::vector<std::string> filenames =
				   g_fs->get_sequential_files(directory, basename + scale_as_string, "png");
				if (!filenames.empty()) {
					mipmaps_.insert(std::make_pair(
					   scale_as_float, std::unique_ptr<MipMapEntry>(new MipMapEntry(filenames))));
				}
			};
			add_scale(0.5f, "_0.5");
			add_scale(1.0f, "_1");
			add_scale(2.0f, "_2");
			add_scale(4.0f, "_4");

			if (mipmaps_.count(1.0f) == 0) {
				// There might be only 1 scale
				add_scale(1.0f, "");
				if (mipmaps_.count(1.0f) == 0) {
					// No files found at all
					throw Widelands::GameDataError(
					   "Animation in directory '%s' with basename '%s' has no images for mandatory "
					   "scale '1' in mipmap - supported scales are: 0.5, 1, 2, 4",
					   directory.c_str(), basename.c_str());
				}
			}
		}

		// Frames and FPS
		nr_frames_ = mipmaps_.begin()->second->image_files.size();
		if (table.has_key("fps")) {
			if (nr_frames_ == 1) {
				throw Widelands::GameDataError("Animation with one picture %s must not have 'fps'",
				                               mipmaps_.begin()->second->image_files.front().c_str());
			}
			frametime_ = 1000 / get_positive_int(table, "fps");
		}

		if (representative_frame_ < 0 || representative_frame_ > nr_frames_ - 1) {
			throw wexception("Animation has %d as its representative frame, but the frame indices "
			                 "available are 0 - %d",
			                 representative_frame_, nr_frames_ - 1);
		}

		// Perform some checks to make sure that the data is complete and consistent
		const bool should_have_playercolor = mipmaps_.begin()->second->has_playercolor_masks;
		for (const auto& mipmap : mipmaps_) {
			if (mipmap.second->image_files.size() != nr_frames_) {
				throw Widelands::GameDataError(
				   "Mismatched number of images for different scales in animation table: %" PRIuS
				   " vs. %u at scale %.2f",
				   mipmap.second->image_files.size(), nr_frames_, static_cast<double>(mipmap.first));
			}
			if (mipmap.second->has_playercolor_masks != should_have_playercolor) {
				throw Widelands::GameDataError(
				   "Mismatched existence of player colors in animation table for scales %.2f and %.2f",
				   static_cast<double>(mipmaps_.begin()->first), static_cast<double>(mipmap.first));
			}
		}
		if (mipmaps_.count(1.0f) != 1) {
			throw Widelands::GameDataError(
			   "All animations must provide images for the neutral scale (1.0)");
		}
	} catch (const LuaError& e) {
		throw Widelands::GameDataError("Error in animation table: %s", e.what());
	}
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

float NonPackedAnimation::height() const {
	const MipMapEntry& mipmap = *mipmaps_.at(1.0f);
	mipmap.ensure_graphics_are_loaded();
	return mipmap.frames.front()->height();
}

uint16_t NonPackedAnimation::nr_frames() const {
	return nr_frames_;
}

uint32_t NonPackedAnimation::frametime() const {
	return frametime_;
}

const Image* NonPackedAnimation::representative_image(const RGBColor* clr) const {
	const MipMapEntry& mipmap = *mipmaps_.at(1.0f);
	std::vector<std::string> images = mipmap.image_files;
	assert(!images.empty());
	const Image* image = (mipmap.has_playercolor_masks && clr) ?
	                        playercolor_image(*clr, images[representative_frame_]) :
	                        g_gr->images().get(images[representative_frame_]);

	const int w = image->width();
	const int h = image->height();

	Texture* rv = new Texture(w, h);
	rv->blit(Rectf(0.f, 0.f, w, h), *image, Rectf(0.f, 0.f, w, h), 1., BlendMode::Copy);
	return rv;
}

uint32_t NonPackedAnimation::current_frame(uint32_t time) const {
	if (nr_frames() > 1) {
		return (play_once_ && time / frametime_ > static_cast<uint32_t>(nr_frames() - 1)) ?
		          static_cast<uint32_t>(nr_frames() - 1) :
		          time / frametime_ % nr_frames();
	}
	return 0;
}

void NonPackedAnimation::trigger_sound(uint32_t time, const Widelands::Coords& coords) const {
	if (sound_effect_ == kNoSoundEffect || coords == Widelands::Coords::null()) {
		return;
	}

	const uint32_t framenumber = current_frame(time);

	if (framenumber == 0) {
		Notifications::publish(
		   NoteSound(SoundType::kAmbient, sound_effect_, coords, sound_priority_));
	}
}

Rectf NonPackedAnimation::source_rectangle(const int percent_from_bottom, float scale) const {
	const MipMapEntry& mipmap = *mipmaps_.at(find_best_scale(scale));
	mipmap.ensure_graphics_are_loaded();
	const Image* first_frame = mipmap.frames.front();
	const float h = percent_from_bottom * first_frame->height() / 100;
	// Using floor for pixel perfect positioning
	return Rectf(0.f, std::floor(first_frame->height() - h), first_frame->width(), h);
}

Rectf NonPackedAnimation::destination_rectangle(const Vector2f& position,
                                                const Rectf& source_rect,
                                                const float scale) const {
	const float best_scale = find_best_scale(scale);
	return Rectf(position.x - (hotspot_.x - source_rect.x / best_scale) * scale,
	             position.y - (hotspot_.y - source_rect.y / best_scale) * scale,
	             source_rect.w * scale / best_scale, source_rect.h * scale / best_scale);
}

void NonPackedAnimation::blit(uint32_t time,
                              const Widelands::Coords& coords,
                              const Rectf& source_rect,
                              const Rectf& destination_rect,
                              const RGBColor* clr,
                              Surface* target,
                              float scale) const {
	mipmaps_.at(find_best_scale(scale))
	   ->blit(current_frame(time), source_rect, destination_rect, clr, target);
	trigger_sound(time, coords);
}

void NonPackedAnimation::load_default_scale_and_sounds() const {
	mipmaps_.at(1.0f)->ensure_graphics_are_loaded();
	if (sound_effect_ != kNoSoundEffect && !SoundHandler::is_backend_disabled()) {
		g_sh->load_fx(SoundType::kAmbient, sound_effect_);
	}
}

}  // namespace

/*
==============================================================================

DirAnimations IMPLEMENTAION

==============================================================================
*/

DirAnimations::DirAnimations(
   uint32_t dir1, uint32_t dir2, uint32_t dir3, uint32_t dir4, uint32_t dir5, uint32_t dir6) {
	animations_[0] = dir1;
	animations_[1] = dir2;
	animations_[2] = dir3;
	animations_[3] = dir4;
	animations_[4] = dir5;
	animations_[5] = dir6;
}

/*
==============================================================================

AnimationManager IMPLEMENTATION

==============================================================================
*/
uint32_t AnimationManager::load(const LuaTable& table, const std::string& basename) {
	animations_.push_back(std::unique_ptr<Animation>(new NonPackedAnimation(table, basename)));
	return animations_.size();
}
uint32_t AnimationManager::load(const std::string& map_object_name,
                                const LuaTable& table,
                                const std::string& basename) {
	animations_.push_back(std::unique_ptr<Animation>(new NonPackedAnimation(table, basename)));
	const size_t result = animations_.size();
	representative_animations_by_map_object_name_.insert(std::make_pair(map_object_name, result));
	return result;
}

const Animation& AnimationManager::get_animation(uint32_t id) const {
	if (!id || id > animations_.size())
		throw Widelands::GameDataError("Requested unknown animation with id: %i", id);

	return *animations_[id - 1];
}

const Image* AnimationManager::get_representative_image(uint32_t id, const RGBColor* clr) {
	const auto hash = std::make_pair(id, clr);
	if (representative_images_.count(hash) != 1) {
		representative_images_.insert(std::make_pair(
		   hash, std::unique_ptr<const Image>(
		            std::move(g_gr->animations().get_animation(id).representative_image(clr)))));
	}
	return representative_images_.at(hash).get();
}

const Image* AnimationManager::get_representative_image(const std::string& map_object_name,
                                                        const RGBColor* clr) {
	if (representative_animations_by_map_object_name_.count(map_object_name) != 1) {
		log("Warning: %s has no animation assigned for its representative image, or it's not a known "
		    "map object\n",
		    map_object_name.c_str());
		return new Texture(0, 0);
	}
	return get_representative_image(
	   representative_animations_by_map_object_name_.at(map_object_name), clr);
}
