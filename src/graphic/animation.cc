/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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

#include <boost/algorithm/string/replace.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/diranimations.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/surface.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/map_object.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"


using namespace std;


namespace  {
// Parses an array { 12, 23 } into a point.
void get_point(const LuaTable& table, Point* p) {
	std::vector<int> pts = table.array_entries<int>();
	if (pts.size() != 2) {
		throw wexception("Expected 2 entries, but got %" PRIuS ".", pts.size());
	}
	p->x = pts[0];
	p->y = pts[1];
}

/**
 * Implements the Animation interface for an animation that is unpacked on disk, that
 * is every frame and every pc color frame is an singular file on disk.
 */
class NonPackedAnimation : public Animation {
public:
	virtual ~NonPackedAnimation() {}
	NonPackedAnimation(const LuaTable& table);

	// Implements Animation.
	uint16_t width() const override;
	uint16_t height() const override;
	uint16_t nr_frames() const override;
	uint32_t frametime() const override;
	const Point& hotspot() const override;
	Image* representative_image(const RGBColor* clr) const override;
	const std::string& representative_image_filename() const override;
	virtual void blit(uint32_t time, const Point&, const Rect& srcrc, const RGBColor* clr, Surface*)
		const override;
	void trigger_soundfx(uint32_t framenumber, uint32_t stereo_position) const override;


private:
	// Loads the graphics if they are not yet loaded.
	void ensure_graphics_are_loaded() const;

	// Load the needed graphics from disk.
	void load_graphics();

	uint32_t current_frame(uint32_t time) const;

	uint32_t frametime_;
	Point hotspot_;
	bool hasplrclrs_;
	std::vector<std::string> image_files_;
	std::vector<std::string> pc_mask_image_files_;

	vector<const Image*> frames_;
	vector<const Image*> pcmasks_;

	// name of sound effect that will be played at frame 0.
	// TODO(sirver): this should be done using playFX in a program instead of
	// binding it to the animation.
	string sound_effect_;
	bool play_once_;
};

NonPackedAnimation::NonPackedAnimation(const LuaTable& table)
		: frametime_(FRAME_LENGTH),
		  hasplrclrs_(false),
		  play_once_(false) {
	try {
		get_point(*table.get_table("hotspot"), &hotspot_);

		if (table.has_key("sound_effect")) {
			std::unique_ptr<LuaTable> sound_effects = table.get_table("sound_effect");

			const std::string name = sound_effects->get_string("name");
			const std::string directory = sound_effects->get_string("directory");
			sound_effect_ = directory + "/" + name;
			g_sound_handler.load_fx_if_needed(directory, name, sound_effect_);
		}

		if (table.has_key("play_once")) {
			play_once_ = table.get_bool("play_once");
		}

		image_files_ = table.get_table("pictures")->array_entries<std::string>();

		if (image_files_.empty()) {
			throw wexception("Animation without pictures. The template should look similar to this:"
								  " 'directory/idle_??.png' for 'directory/idle_00.png' etc.");
		} else if (table.has_key("fps")) {
			if (image_files_.size() == 1) {
				throw wexception("Animation with one picture %s must not have 'fps'", image_files_[0].c_str());
			}
			frametime_ = 1000 / get_positive_int(table, "fps");
		}

		for (std::string image_file : image_files_) {
			boost::replace_all(image_file, ".png", "_pc.png");
			if (g_fs->file_exists(image_file)) {
				hasplrclrs_ = true;
				pc_mask_image_files_.push_back(image_file);
			} else if (hasplrclrs_) {
				throw wexception("Animation is missing player color file: %s", image_file.c_str());
			}
		}
		assert(!image_files_.empty());
		assert(pc_mask_image_files_.size() == image_files_.size() || pc_mask_image_files_.empty());

	} catch (const LuaError& e) {
		throw wexception("Error in animation table: %s", e.what());
	}
}

void NonPackedAnimation::ensure_graphics_are_loaded() const {
	if (frames_.empty()) {
		const_cast<NonPackedAnimation*>(this)->load_graphics();
	}
}

void NonPackedAnimation::load_graphics() {
	if (image_files_.empty())
		throw wexception("animation without pictures.");

	if (pc_mask_image_files_.size() && pc_mask_image_files_.size() != image_files_.size())
		throw wexception
			("animation has %" PRIuS " frames but playercolor mask has %" PRIuS " frames",
			 image_files_.size(), pc_mask_image_files_.size());

	for (const std::string& filename : image_files_) {
		const Image* image = g_gr->images().get(filename);
		if (frames_.size() &&
		    (frames_[0]->width() != image->width() || frames_[0]->height() != image->height())) {
			throw wexception("wrong size: (%u, %u), should be (%u, %u) like the first frame",
			                 image->width(),
			                 image->height(),
			                 frames_[0]->width(),
			                 frames_[0]->height());
		}
		frames_.push_back(image);
	}

	for (const std::string& filename : pc_mask_image_files_) {
		// TODO(unknown): Do not load playercolor mask as opengl texture or use it as
		//     opengl texture.
		const Image* pc_image = g_gr->images().get(filename);
		if (frames_[0]->width() != pc_image->width() || frames_[0]->height() != pc_image->height()) {
			// TODO(unknown): see bug #1324642
			throw wexception("playercolor mask has wrong size: (%u, %u), should "
			                 "be (%u, %u) like the animation frame",
			                 pc_image->width(),
			                 pc_image->height(),
			                 frames_[0]->width(),
			                 frames_[0]->height());
		}
		pcmasks_.push_back(pc_image);
	}
}

uint16_t NonPackedAnimation::width() const {
	ensure_graphics_are_loaded();
	return frames_[0]->width();
}

uint16_t NonPackedAnimation::height() const {
	ensure_graphics_are_loaded();
	return frames_[0]->height();
}

uint16_t NonPackedAnimation::nr_frames() const {
	ensure_graphics_are_loaded();
	return frames_.size();
}

uint32_t NonPackedAnimation::frametime() const {
	return frametime_;
}

const Point& NonPackedAnimation::hotspot() const {
	return hotspot_;
}

Image* NonPackedAnimation::representative_image(const RGBColor* clr) const {
	assert(!image_files_.empty());

	const Image* image = g_gr->images().get(image_files_[0]);
	int w = image->width();
	int h = image->height();

	Texture* rv = new Texture(w, h);

	// Initialize the rectangle
	rv->fill_rect(Rect(Point(0, 0), w, h), RGBAColor(255, 255, 255, 0));

	if (!hasplrclrs_ || clr == nullptr) {
		rv->blit(Rect(Point(0, 0), w, h), *image, Rect(Point(0, 0), w, h), 1., BlendMode::UseAlpha);
	} else {
		rv->blit_blended(Rect(Point(0, 0), w, h), *image,
		                 *g_gr->images().get(pc_mask_image_files_[0]), Rect(Point(0, 0), w, h), *clr);
	}
	return rv;
}

const std::string& NonPackedAnimation::representative_image_filename() const {
	return image_files_[0];
}

uint32_t NonPackedAnimation::current_frame(uint32_t time) const {
	if (nr_frames() > 1) {
		return (play_once_ && time / frametime_ > static_cast<uint32_t>(nr_frames() - 1)) ?
					static_cast<uint32_t>(nr_frames() - 1) :
					time / frametime_ % nr_frames();
	}
	return 0;
}

void NonPackedAnimation::trigger_soundfx(uint32_t time, uint32_t stereo_position) const {
	if (sound_effect_.empty()) {
		return;
	}

	const uint32_t framenumber = current_frame(time);

	if (framenumber == 0) {
		g_sound_handler.play_fx(sound_effect_, stereo_position, 1);
	}
}

void NonPackedAnimation::blit
	(uint32_t time, const Point& dst, const Rect& srcrc, const RGBColor* clr, Surface* target) const
{
	assert(target);

	const uint32_t idx = current_frame(time);
	assert(idx < nr_frames());

	if (!hasplrclrs_ || clr == nullptr) {
		target->blit(
		   Rect(dst.x, dst.y, srcrc.w, srcrc.h), *frames_.at(idx), srcrc, 1., BlendMode::UseAlpha);
	} else {
		target->blit_blended(
		   Rect(dst.x, dst.y, srcrc.w, srcrc.h), *frames_.at(idx), *pcmasks_.at(idx), srcrc, *clr);
	}
}

}  // namespace


/*
==============================================================================

DirAnimations IMPLEMENTAION

==============================================================================
*/

DirAnimations::DirAnimations
	(uint32_t dir1,
	 uint32_t dir2,
	 uint32_t dir3,
	 uint32_t dir4,
	 uint32_t dir5,
	 uint32_t dir6)
{
	m_animations[0] = dir1;
	m_animations[1] = dir2;
	m_animations[2] = dir3;
	m_animations[3] = dir4;
	m_animations[4] = dir5;
	m_animations[5] = dir6;
}

/*
==============================================================================

AnimationManager IMPLEMENTATION

==============================================================================
*/

uint32_t AnimationManager::load(const LuaTable& table) {
	animations_.push_back(std::unique_ptr<Animation>(new NonPackedAnimation(table)));
	return animations_.size();
}

const Animation& AnimationManager::get_animation(uint32_t id) const
{
	if (!id || id > animations_.size())
		throw wexception("Requested unknown animation with id: %i", id);

	return *animations_[id - 1].get();
}

const Image* AnimationManager::get_representative_image(uint32_t id, const RGBColor* clr) {
	if (representative_images_.count(id) != 1) {
		representative_images_.insert(
					std::make_pair(
						id,
						std::unique_ptr<Image>(g_gr->animations().get_animation(id).representative_image(clr))));
	}
	return representative_images_.at(id).get();
}
