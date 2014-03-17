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

// Parses a point from a string like 'p=x y' into p. Throws on error.
void parse_point(const string& def, Point* p) {
	vector<string> split_vector;
	boost::split(split_vector, def, boost::is_any_of(" "));
	if (split_vector.size() != 2)
		throw wexception("Invalid point definition: %s", def.c_str());

	p->x = boost::lexical_cast<int32_t>(split_vector[0]);
	p->y = boost::lexical_cast<int32_t>(split_vector[1]);
}

// Parses a rect from a string like 'p=x y w h' into r. Throws on error.
void parse_rect(const string& def, Rect* r) {
	vector<string> split_vector;
	boost::split(split_vector, def, boost::is_any_of(" "));
	if (split_vector.size() != 4)
		throw wexception("Invalid rect definition: %s", def.c_str());

	r->x = boost::lexical_cast<int32_t>(split_vector[0]);
	r->y = boost::lexical_cast<int32_t>(split_vector[1]);
	r->w = boost::lexical_cast<uint32_t>(split_vector[2]);
	r->h = boost::lexical_cast<uint32_t>(split_vector[3]);
}

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

struct SfxCues {
	void read(const string & directory, Section & s);
	void trigger_soundfx(uint32_t framenumber, uint32_t stereo_position) const;

private:
	/// mapping of soundeffect name to frame number, indexed by frame number .
	map<uint32_t, string> cues_;
};

/**
 * Read mapping from frame numbers to sound effect names and load effects
 */
void SfxCues::read(const string & directory, Section & s)
{
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
				cues_.find(frame_number);
			if (it != cues_.end())
				throw wexception
					("redefinition for frame %u to \"%s\" (previously defined to "
					 "\"%s\")",
					 frame_number, parameters, it->second.c_str());
		} catch (const _wexception & e) {
			throw wexception("sfx: %s", e.what());
		}
		cues_[frame_number] = parameters;
	}
}

void SfxCues::trigger_soundfx(uint32_t framenumber, uint32_t stereo_position) const
{
	const map<uint32_t, string>::const_iterator sfx_cue = cues_.find(framenumber);
	if (sfx_cue != cues_.end())
		g_sound_handler.play_fx(sfx_cue->second, stereo_position, 1);
}


/**
 * Animation in blits-format; that is, a list of blit rectangles
 * is maintained for every frame.
 */
class BlitsAnimation : public Animation {
public:
	BlitsAnimation(const string & directory, Section & s);
	virtual ~BlitsAnimation() {}

	// Implements Animation.
	virtual uint16_t width() const {return width_;}
	virtual uint16_t height() const {return height_;}
	virtual uint16_t nr_frames() const {return frames_.size();}
	virtual uint32_t frametime() const {return frametime_;}
	virtual const Point& hotspot() const {return hotspot_;};
	virtual const Image& representative_image(const RGBColor& clr) const;
	void blit(uint32_t time, const Point&, const Rect& srcrc, const RGBColor* clr, Surface*) const;
	virtual void trigger_soundfx(uint32_t framenumber, uint32_t stereo_position) const;

private:
	struct Blit {
		uint16_t srcx, srcy;
		uint16_t w, h;
		int16_t dstx, dsty;
	};

	// loaded from game data
	string hash_;
	vector<vector<Blit> > frames_;
	uint32_t frametime_;
	SfxCues sfx_cues_;
	const Image * spritemap_;
	const Image * spritemap_pc_;

	// derived data
	uint16_t width_, height_;
	Point hotspot_;
};

BlitsAnimation::BlitsAnimation(const string & directory, Section & s) :
	frametime_(FRAME_LENGTH)
{
	hash_ = directory + s.get_name();

	sfx_cues_.read(directory, s);

	const int32_t fps = s.get_int("fps");
	if (fps < 0)
		throw wexception("fps is %i, must be non-negative", fps);
	if (fps > 0)
		frametime_ = 1000 / fps;

	// Load the graphis
	string spritemap = directory + s.get_safe_string("spritemap");
	spritemap_ = g_gr->images().get(spritemap + ".png");
	if (g_fs->FileExists(spritemap + "_pc.png")) {
		spritemap_pc_ = g_gr->images().get(spritemap + "_pc.png");
	} else {
		spritemap_pc_ = 0;
	}

	// Parse frames
	const int32_t nrframes = s.get_int("nrframes");
	if (nrframes <= 0)
		throw wexception("nrframes must be at least 1");

	int16_t minx = numeric_limits<int16_t>::max();
	int16_t miny = numeric_limits<int16_t>::max();
	int16_t maxx = numeric_limits<int16_t>::min();
	int16_t maxy = numeric_limits<int16_t>::min();

	for (int32_t framenr = 0; framenr < nrframes; ++framenr) {
		vector<Blit> blits;
		string framedata = s.get_safe_string(boost::lexical_cast<string>(framenr));
		vector<string> blitsdata;
		boost::split(blitsdata, framedata, boost::is_any_of(";"));

		BOOST_FOREACH(const string & it, blitsdata) {
			vector<string> coords;
			boost::split(coords, it, boost::is_any_of(",@"));
			if (coords.size() != 6)
				throw wexception("blit code '%s' is malformed", it.c_str());

			Blit blt;
			blt.srcx = boost::lexical_cast<int16_t>(coords[0]);
			blt.srcy = boost::lexical_cast<int16_t>(coords[1]);
			blt.w = boost::lexical_cast<int16_t>(coords[2]);
			blt.h = boost::lexical_cast<int16_t>(coords[3]);
			blt.dstx = boost::lexical_cast<int16_t>(coords[4]);
			blt.dsty = boost::lexical_cast<int16_t>(coords[5]);
			blits.push_back(blt);

			minx = min(blt.dstx, minx);
			miny = min(blt.dsty, miny);
			maxx = max<int>(blt.dstx + blt.w, maxx);
			maxy = max<int>(blt.dsty + blt.h, maxy);
		}

		frames_.push_back(blits);
	}

	width_ = maxx - minx;
	height_ = maxy - miny;
	hotspot_ = Point(-minx, -miny);
}

void BlitsAnimation::trigger_soundfx(uint32_t time, uint32_t stereo_position) const
{
	const uint32_t framenumber = time / frametime_ % nr_frames();
	sfx_cues_.trigger_soundfx(framenumber, stereo_position);
}

const Image & BlitsAnimation::representative_image(const RGBColor& clr) const {
	const string hash =
		(boost::format("%s:%02x%02x%02x:animation_pic") % hash_ % static_cast<int>(clr.r) %
		 static_cast<int>(clr.g) % static_cast<int>(clr.b))
			.str();

	ImageCache& image_cache = g_gr->images();
	if (image_cache.has(hash))
		return *image_cache.get(hash);

	return *image_cache.insert(new AnimationImage(hash, this, clr));
}

void BlitsAnimation::blit
	(uint32_t time, const Point & dst, const Rect & srcrc, const RGBColor * clr, Surface * target) const
{
	assert(target);
	const uint32_t framenumber = time / frametime_ % nr_frames();

	const Image* use_image = spritemap_;
	if (clr && spritemap_pc_) {
		use_image = ImageTransformations::player_colored(*clr, spritemap_, spritemap_pc_);
	}

	BOOST_FOREACH(const Blit & blt, frames_[framenumber]) {
		Rect framerect(blt.dstx + hotspot_.x, blt.dsty + hotspot_.y, blt.w, blt.h);
		Rect bsrc(blt.srcx, blt.srcy, blt.w, blt.h);
		Point bdst(dst.x + hotspot_.x + blt.dstx - srcrc.x, dst.y + hotspot_.y + blt.dsty - srcrc.y);

		if (srcrc.x > framerect.x) {
			if (srcrc.x >= framerect.x + int(framerect.w))
				continue;
			int16_t delta = srcrc.x - framerect.x;
			framerect.x += delta;
			framerect.w -= delta;
			bsrc.x += delta;
			bsrc.w -= delta;
			bdst.x += delta;
		}
		if (srcrc.x + int(srcrc.w) <= framerect.x)
			continue;
		if (srcrc.x + int(srcrc.w) < framerect.x + int(framerect.w)) {
			bsrc.w = srcrc.x + srcrc.w - framerect.x;
		}

		if (srcrc.y > framerect.y) {
			if (srcrc.y >= framerect.y + int(framerect.h))
				continue;
			int16_t delta = srcrc.y - framerect.y;
			framerect.y += delta;
			framerect.h -= delta;
			bsrc.y += delta;
			bsrc.h -= delta;
			bdst.y += delta;
		}
		if (srcrc.y + int(srcrc.h) <= framerect.y)
			continue;
		if (srcrc.y + int(srcrc.h) < framerect.y + int(framerect.h)) {
			bsrc.h = srcrc.y + srcrc.h - framerect.y;
		}

		target->blit(bdst, use_image->surface(), bsrc);
	}
}


/**
 * Implements the Animation interface for a packed animation, that is an animation
 * that is contained in a singular image (plus one for player color).
 */
class PackedAnimation : public Animation {
public:
	virtual ~PackedAnimation() {}
	PackedAnimation(const string& directory, Section & s);

	// Implements Animation.
	virtual uint16_t width() const {return width_;}
	virtual uint16_t height() const {return height_;}
	virtual uint16_t nr_frames() const {return nr_frames_;}
	virtual uint32_t frametime() const {return frametime_;}
	virtual const Point& hotspot() const {return hotspot_;};
	virtual const Image& representative_image(const RGBColor& clr) const;
	void blit(uint32_t time, const Point&, const Rect& srcrc, const RGBColor* clr, Surface*) const;
	virtual void trigger_soundfx(uint32_t framenumber, uint32_t stereo_position) const;

private:
	struct Region {
		Point target_offset;
		uint16_t w, h;
		vector<Point> source_offsets;  // indexed by frame nr.
	};

	uint16_t width_, height_;
	uint16_t nr_frames_;
	uint32_t frametime_;
	Point hotspot_;
	Point base_offset_;

	const Image* image_;  // Not owned
	const Image* pcmask_;  // Not owned
	vector<Region> regions_;
	string hash_;

	SfxCues sfx_cues;
};

PackedAnimation::PackedAnimation(const string& directory, Section& s)
		: width_(0), height_(0), nr_frames_(0), frametime_(FRAME_LENGTH), image_(NULL), pcmask_(NULL) {
	hash_ = directory + s.get_name();

	sfx_cues.read(directory, s);

	const int32_t fps = s.get_int("fps");
	if (fps < 0)
		throw wexception("fps is %i, must be non-negative", fps);
	if (fps > 0)
		frametime_ = 1000 / fps;

	hotspot_ = s.get_Point("hotspot");

	// Load the graphis
	string pic_fn = directory + s.get_safe_string("pics");
	image_ = g_gr->images().get(pic_fn);
	boost::replace_all(pic_fn, ".png", "");
	if (g_fs->FileExists(pic_fn + "_pc.png")) {
		pcmask_ = g_gr->images().get(pic_fn + "_pc.png");
	}

	// Parse dimensions.
	Point p;
	parse_point(s.get_string("dimensions"), &p);
	width_ = p.x;
	height_ = p.y;

	// Parse base_offset.
	parse_point(s.get_string("base_offset"), &base_offset_);

	// Parse regions
	NumberGlob glob("region_??");
	string region_name;
	while (glob.next(&region_name)) {
		string value = s.get_string(region_name.c_str(), "");
		if (value.empty())
			break;

		boost::trim(value);
		vector<string> split_vector;
		boost::split(split_vector, value, boost::is_any_of(":"));
		if (split_vector.size() != 2)
			throw wexception("%s: line is ill formatted. Should be <rect>:<offsets>", region_name.c_str());

		vector<string> offset_strings;
		boost::split(offset_strings, split_vector[1], boost::is_any_of(";"));
		if (nr_frames_ && nr_frames_ != offset_strings.size())
			throw wexception
				("%s: region has different number of frames than previous (%i != %" PRIuS ").",
				 region_name.c_str(), nr_frames_, offset_strings.size());
		nr_frames_ = offset_strings.size();

		Rect region_rect;
		parse_rect(split_vector[0], &region_rect);

		Region r;
		r.target_offset.x = region_rect.x;
		r.target_offset.y = region_rect.y;
		r.w = region_rect.w;
		r.h = region_rect.h;

		BOOST_FOREACH(const string& offset_string, offset_strings) {
			parse_point(offset_string, &p);
			r.source_offsets.push_back(p);
		}
		regions_.push_back(r);
	}

	if (regions_.empty())  // No regions? Only one frame then.
		nr_frames_ = 1;
}

void PackedAnimation::trigger_soundfx(uint32_t time, uint32_t stereo_position) const
{
	const uint32_t framenumber = time / frametime_ % nr_frames();
	sfx_cues.trigger_soundfx(framenumber, stereo_position);
}

const Image& PackedAnimation::representative_image(const RGBColor& clr) const {
	const string hash =
		(boost::format("%s:%02x%02x%02x:animation_pic") % hash_ % static_cast<int>(clr.r) %
		 static_cast<int>(clr.g) % static_cast<int>(clr.b))
			.str();

	ImageCache& image_cache = g_gr->images();
	if (image_cache.has(hash))
		return *image_cache.get(hash);

	return *image_cache.insert(new AnimationImage(hash, this, clr));
}

void PackedAnimation::blit
	(uint32_t time, const Point& dst, const Rect& srcrc, const RGBColor* clr, Surface* target) const
{
	assert(target);
	const uint32_t framenumber = time / frametime_ % nr_frames();

	const Image* use_image = image_;
	if (clr && pcmask_) {
		use_image = ImageTransformations::player_colored(*clr, image_, pcmask_);
	}

	target->blit
		(dst, use_image->surface(), Rect(base_offset_.x + srcrc.x, base_offset_.y + srcrc.y, srcrc.w, srcrc.h));

	BOOST_FOREACH(const Region& r, regions_) {
		Rect rsrc = Rect(r.source_offsets[framenumber], r.w, r.h);
		Point rdst = dst + r.target_offset - srcrc;

		if (srcrc.x > r.target_offset.x) {
			rdst.x += srcrc.x - r.target_offset.x;
			rsrc.x += srcrc.x - r.target_offset.x;
			rsrc.w -= srcrc.x - r.target_offset.x;
			if (rsrc.w > r.w)
				continue;
		}
		if (srcrc.y > r.target_offset.y) {
			rdst.y += srcrc.y - r.target_offset.y;
			rsrc.y += srcrc.y - r.target_offset.y;
			rsrc.h -= srcrc.y - r.target_offset.y;
			if (rsrc.h > r.h)
				continue;
		}
		if (r.target_offset.x + rsrc.w > srcrc.x + srcrc.w) {
			rsrc.w = srcrc.x + srcrc.w - r.target_offset.x;
		}
		if (r.target_offset.y + rsrc.h > srcrc.y + srcrc.h) {
			rsrc.h = srcrc.y + srcrc.h - r.target_offset.y;
		}

		target->blit(rdst, use_image->surface(), rsrc);
	}
}

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
	string format = s.get_string("format", "");
	if (format == "blits") {
		m_animations.push_back(new BlitsAnimation(directory, s));
	} else if (s.get_bool("packed", false)) {
		m_animations.push_back(new PackedAnimation(directory, s));
	} else {
		m_animations.push_back(new NonPackedAnimation(directory, s));
	}
	const uint32_t id = m_animations.size();

	return id;
}

const Animation& AnimationManager::get_animation(uint32_t id) const
{
	if (!id || id > m_animations.size())
		throw wexception("Requested unknown animation with id: %i", id);

	return *m_animations[id - 1];
}
