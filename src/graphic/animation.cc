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
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/diranimations.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/surface.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/bob.h"
#include "logic/instances.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"


using namespace std;


namespace  {

/// A class that makes iteration over filename_?.png templates easy.
class NumberGlob {
public:
	NumberGlob(const std::string& pictmp);

	/// If there is a next filename, puts it in 's' and returns true.
	bool next(std::string* s);

private:
	std::string templ_;
	std::string fmtstr_;
	std::string replstr_;
	uint32_t cur_;
	uint32_t max_;

	DISALLOW_COPY_AND_ASSIGN(NumberGlob);
};

/**
 * Implemantation for NumberGlob.
 */
NumberGlob::NumberGlob(const string& pictmp) : templ_(pictmp), cur_(0) {
	int nchars = count(pictmp.begin(), pictmp.end(), '?');
	fmtstr_ = "%0" + boost::lexical_cast<string>(nchars) + "i";

	max_ = 1;
	for (int i = 0; i < nchars; ++i) {
		max_ *= 10;
		replstr_ += "?";
	}
	max_ -= 1;
}

bool NumberGlob::next(string* s) {
	if (cur_ > max_)
		return false;

	if (max_) {
		*s = boost::replace_last_copy(templ_, replstr_, (boost::format(fmtstr_) % cur_).str());
	} else {
		*s = templ_;
	}
	++cur_;
	return true;
}


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
	const std::string& representative_image_from_disk_filename() const override;
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

		const std::string templatedirname = table.get_string("directory");
		const std::string templatefilename = table.get_string("template");

		//  In the filename template, the last sequence of '?' characters (if any)
		//  is replaced with a number, for example the template "idle_??" is
		//  replaced with "idle_00". Then the code looks for a PNG file with that
		//  name, increments the number and continues on until it cannot find any
		//  file. Then it is assumed that there are no more frames in the animation.
		std::string picnametempl = templatedirname + templatefilename;

		// Strip the .png extension if it has one.
		boost::replace_all(picnametempl, ".png", "");

		NumberGlob glob(picnametempl);
		string filename_wo_ext;
		while (glob.next(&filename_wo_ext)) {
			const std::string filename = filename_wo_ext + ".png";
			if (!g_fs->file_exists(filename)) {
				break;
			}
			image_files_.push_back(filename);

			const std::string pc_filename = filename_wo_ext + "_pc.png";
			if (g_fs->file_exists(pc_filename)) {
				hasplrclrs_ = true;
				pc_mask_image_files_.push_back(pc_filename);
			} else if (hasplrclrs_) {
				throw wexception("Animation in directory %s is missing player color file: %s",
									  templatedirname.c_str(), pc_filename.c_str());
			}
		}

		if (image_files_.empty()) {
			throw wexception("Animation %s without pictures in directory %s. "
								  "Make sure that the directory has a trailing slash. "
								  "The template should look similar to this: idle_?? for idle_00.png etc.",
								  templatefilename.c_str(), templatedirname.c_str());
		} else if (table.has_key("fps")) {
			if (image_files_.size() == 1) {
				throw wexception("Animation %s with one picture in directory %s must not have 'fps'",
									  templatefilename.c_str(), templatedirname.c_str());
			}
			frametime_ = 1000 / get_positive_int(table, "fps");
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

const std::string& NonPackedAnimation::representative_image_from_disk_filename() const {
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
		::blit(Rect(dst.x, dst.y, srcrc.w, srcrc.h),
		     *frames_.at(idx),
		     srcrc,
		     1.,
		     BlendMode::UseAlpha,
		     target);
	} else {
		blit_blended(Rect(dst.x, dst.y, srcrc.w, srcrc.h),
		             *frames_.at(idx),
		             *pcmasks_.at(idx),
		             srcrc,
		             *clr,
		             target);
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
	m_animations.push_back(new NonPackedAnimation(table));
	return m_animations.size();
}

const Animation& AnimationManager::get_animation(uint32_t id) const
{
	if (!id || id > m_animations.size())
		throw wexception("Requested unknown animation with id: %i", id);

	return *m_animations[id - 1];
}

AnimationManager::~AnimationManager()
{
	for (vector<Animation*>::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
		delete *it;
}
