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

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "constants.h"
#include "container_iterate.h"
#include "graphic/diranimations.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/image_transformations.h"
#include "graphic/surface.h"
#include "graphic/surface_cache.h"
#include "helper.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "logic/bob.h"
#include "logic/instances.h"  // For Map_Object_Descr.
#include "profile/profile.h"
#include "sound/sound_handler.h"
#include "wexception.h"


using namespace std;


namespace  {

/**
 * An Image Implementation that draws a static animation into a surface.
 */
class AnimationImage : public Image {
public:
	AnimationImage
		(const string& ghash, const Animation* anim, const RGBColor& clr)
		: hash_(ghash), anim_(anim), clr_(clr) {}
	virtual ~AnimationImage() {}

	// Implements Image.
	virtual uint16_t width() const {return anim_->width();}
	virtual uint16_t height() const {return anim_->height();}
	virtual const string& hash() const {return hash_;}
	virtual Surface* surface() const {
		SurfaceCache& surface_cache = g_gr->surfaces();
		Surface* surf = surface_cache.get(hash_);
		if (surf)
			return surf;

		// Blit the animation on a freshly wiped surface.
		surf = Surface::create(width(), height());
		surf->fill_rect(Rect(0, 0, surf->width(), surf->height()), RGBAColor(255, 255, 255, 0));
		anim_->blit(0, Point(0, 0), Rect(0, 0, width(), height()), &clr_, surf);
		surface_cache.insert(hash_, surf, true);

		return surf;
	}

private:
	const string hash_;
	const Animation* const anim_;   // Not owned.
	const RGBColor clr_;
};

/**
 * Implements the Animation interface for an animation that is unpacked on disk, that
 * is every frame and every pc color frame is an singular file on disk.
 */
class NonPackedAnimation : public Animation {
public:
	virtual ~NonPackedAnimation() {}
	NonPackedAnimation(const string& directory, Section & s);

	// Implements Animation.
	virtual uint16_t width() const override;
	virtual uint16_t height() const override;
	virtual uint16_t nr_frames() const override;
	virtual uint32_t frametime() const override;
	virtual const Point& hotspot() const override;
	virtual const Image& representative_image(const RGBColor& clr) const override;
	virtual void blit(uint32_t time, const Point&, const Rect& srcrc, const RGBColor* clr, Surface*)
	   const override;
	virtual void trigger_soundfx(uint32_t framenumber, uint32_t stereo_position) const override;


private:
	// Load the needed graphics from disk.
	void load_graphics();

	// Returns the given frame image with the given clr (if not NULL).
	const Image& get_frame(uint32_t time, const RGBColor* playercolor = NULL) const;

	uint32_t frametime_;
	Point hotspot_;
	bool hasplrclrs_;
	std::string picnametempl_;

	vector<const Image*> frames_;
	vector<const Image*> pcmasks_;

	/// mapping of soundeffect name to frame number, indexed by frame number .
	map<uint32_t, string> sfx_cues;
};

NonPackedAnimation::NonPackedAnimation(const string& directory, Section& s)
		: frametime_(FRAME_LENGTH),
		  hasplrclrs_(false) {
	// Read mapping from frame numbers to sound effect names and load effects
	while (Section::Value * const v = s.get_next_val("sfx")) {
		char * parameters = v->get_string(), * endp;
		string fx_name;
		unsigned long long int const value = strtoull(parameters, &endp, 0);
		const uint32_t frame_number = value;
		try {
			if (endp == parameters or frame_number != value)
				throw wexception("expected %s but found \"%s\"", "frame number", parameters);
			parameters = endp;
			force_skip(parameters);

			fx_name = string(directory) + "/" + string(parameters);
			g_sound_handler.load_fx_if_needed(directory, parameters, fx_name);
			map<uint32_t, string>::const_iterator const it =
				sfx_cues.find(frame_number);
			if (it != sfx_cues.end())
				throw wexception
					("redefinition for frame %u to \"%s\" (previously defined to "
					 "\"%s\")",
					 frame_number, parameters, it->second.c_str());
		} catch (const _wexception & e) {
			throw wexception("sfx: %s", e.what());
		}
		sfx_cues[frame_number] = fx_name;
	}

	int32_t const fps = s.get_int("fps");
	if (fps < 0)
		throw wexception("fps is %i, must be non-negative", fps);
	if (fps > 0)
		frametime_ = 1000 / fps;

	hotspot_ = s.get_Point("hotspot");

	//  In the filename template, the last sequence of '?' characters (if any)
	//  is replaced with a number, for example the template "idle_??" is
	//  replaced with "idle_00". Then the code looks if there is a PNG with that
	//  name, increments the number and continues . on until it can not find any
	//  file. Then it is assumed that there are no more frames in the animation.
	if (char const * const pics = s.get_string("pics")) {
		picnametempl_ = directory + pics;
	} else {
		picnametempl_ = directory + s.get_name();
	}
	// Strip the .png extension if it has one.
	boost::replace_all(picnametempl_, ".png", "");
}

void NonPackedAnimation::load_graphics() {
	NumberGlob glob(picnametempl_);
	string filename_wo_ext;
	while (glob.next(&filename_wo_ext)) {
		const string filename = filename_wo_ext + ".png";
		if (!g_fs->FileExists(filename))
			break;

		const Image* image = g_gr->images().get(filename);
		if
			(frames_.size() &&
			 (frames_[0]->width() != image->width() or frames_[0]->height() != image->height()))
					throw wexception
						("wrong size: (%u, %u), should be (%u, %u) like the first frame",
						 image->width(), image->height(), frames_[0]->width(), frames_[0]->height());
		frames_.push_back(image);

		//TODO Do not load playercolor mask as opengl texture or use it as
		//     opengl texture.
		const string pc_filename = filename_wo_ext + "_pc.png";
		if (g_fs->FileExists(pc_filename)) {
			hasplrclrs_ = true;
			const Image* pc_image = g_gr->images().get(pc_filename);
			if (frames_[0]->width() != pc_image->width() or frames_[0]->height() != pc_image->height())
				throw wexception
					("playercolor mask has wrong size: (%u, %u), should "
					 "be (%u, %u) like the animation frame",
					 pc_image->width(), pc_image->height(), frames_[0]->width(), frames_[0]->height());
			pcmasks_.push_back(pc_image);
		}
	}

	if (frames_.empty())
		throw wexception("animation %s has no frames", picnametempl_.c_str());

	if (pcmasks_.size() and pcmasks_.size() != frames_.size())
		throw wexception
			("animation has %" PRIuS " frames but playercolor mask has %" PRIuS " frames",
			 frames_.size(), pcmasks_.size());
}

uint16_t NonPackedAnimation::width() const {
	if (frames_.empty()) {
		const_cast<NonPackedAnimation*>(this)->load_graphics();
	}
	return frames_[0]->width();
}

uint16_t NonPackedAnimation::height() const {
	if (frames_.empty()) {
		const_cast<NonPackedAnimation*>(this)->load_graphics();
	}
	return frames_[0]->height();
}

uint16_t NonPackedAnimation::nr_frames() const {
	if (frames_.empty()) {
		const_cast<NonPackedAnimation*>(this)->load_graphics();
	}
	return frames_.size();
}

uint32_t NonPackedAnimation::frametime() const {
	return frametime_;
}

const Point& NonPackedAnimation::hotspot() const {
	return hotspot_;
}

const Image& NonPackedAnimation::representative_image(const RGBColor& clr) const {
	return get_frame(0, &clr);
}


void NonPackedAnimation::trigger_soundfx
	(uint32_t time, uint32_t stereo_position) const {
	const uint32_t framenumber = time / frametime_ % nr_frames();
	const map<uint32_t, string>::const_iterator sfx_cue = sfx_cues.find(framenumber);
	if (sfx_cue != sfx_cues.end())
		g_sound_handler.play_fx(sfx_cue->second, stereo_position, 1);
}

void NonPackedAnimation::blit
	(uint32_t time, const Point& dst, const Rect& srcrc, const RGBColor* clr, Surface* target) const
{
	assert(target);

	const Image& frame = get_frame(time, clr);
	target->blit(dst, frame.surface(), srcrc);
}

const Image& NonPackedAnimation::get_frame(uint32_t time, const RGBColor* playercolor) const {
	if (frames_.empty()) {
		const_cast<NonPackedAnimation*>(this)->load_graphics();
	}
	const uint32_t framenumber = time / frametime_ % nr_frames();
	assert(framenumber < nr_frames());
	const Image* original = frames_[framenumber];

	if (!hasplrclrs_ || !playercolor)
		return *original;

	assert(frames_.size() == pcmasks_.size());
	return *ImageTransformations::player_colored(*playercolor, original, pcmasks_[framenumber]);
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


/**
 * Load direction animations of the given name.
 *
 * If a section of the given name exists, it is expected to contain a 'dirpics'
 * key and assorted information of the old direction animation format.
 *
 * Otherwise, sections with the names 'name_??', with ?? replaced
 * by nw, ne, e, se, sw, and w are expected to exist and describe
 * the corresponding animations.
 *
 * @param optional No error if animations do not exist
 */
void DirAnimations::parse
	(Widelands::Map_Object_Descr & b,
	 const string & directory,
	 Profile & prof,
	 const string & name,
	 bool optional,
	 const string & default_dirpics)
{
	if (Section * section = prof.get_section(name)) {
		// NOTE: deprecate this format eventually
		char dirpictempl[256];
		char * repl;

		snprintf
			(dirpictempl, sizeof(dirpictempl), "%s",
			 section->get_string("dirpics", default_dirpics.c_str()));
		repl = strstr(dirpictempl, "!!");
		if (!repl)
			throw wexception
				("DirAnimations dirpics name templates %s does not contain !!",
				 dirpictempl);
		strncpy(repl, "%s", 2);

		for (int32_t dir = 0; dir < 6; ++dir) {
			static char const * const dirstrings[6] =
				{"ne", "e", "se", "sw", "w", "nw"};

			// Fake the section name here, so that the animation loading code is
			// using the correct glob pattern to load the images from.
			char pictempl[256];
			snprintf(pictempl, sizeof(pictempl), dirpictempl, dirstrings[dir]);
			section->set_name(pictempl);
			m_animations[dir] = g_gr->animations().load(directory, *section);

			char animname[256];
			snprintf(animname, sizeof(animname), "%s_%s", name.c_str(), dirstrings[dir]);
			b.add_animation(animname, m_animations[dir]);
		}
	} else {
		for (int32_t dir = 0; dir < 6; ++dir) {
			static char const * const dirstrings[6] =
				{"ne", "e", "se", "sw", "w", "nw"};

			char animname[256];
			snprintf(animname, sizeof(animname), "%s_%s", name.c_str(), dirstrings[dir]);
			Section * dirsection = prof.get_section(animname);
			if (dirsection) {
				m_animations[dir] = g_gr->animations().load(directory, *dirsection);
				b.add_animation(animname, m_animations[dir]);
			} else {
				if (!optional)
					throw wexception("DirAnimations: did not find section %s", animname);
			}
		}
	}
}


/*
==============================================================================

AnimationManager IMPLEMENTATION

==============================================================================
*/

uint32_t AnimationManager::load(const string& directory, Section & s) {
	m_animations.push_back(new NonPackedAnimation(directory, s));
	return m_animations.size();
}

const Animation& AnimationManager::get_animation(uint32_t id) const
{
	if (!id || id > m_animations.size())
		throw wexception("Requested unknown animation with id: %i", id);

	return *m_animations[id - 1];
}
