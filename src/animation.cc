/*
 * Copyright (C) 2002 by The Widelands Development Team
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

#include <stdio.h>
#include "animation.h"
#include "bob.h"
#include "error.h"
#include "constants.h"
#include "profile.h"
#include "wexception.h"

/*
==============================================================================

EncodeData IMPLEMENTAION

==============================================================================
*/

/*
===============
EncodeData::clear

Reset the EncodeData to defaults (no special colors)
===============
*/
void EncodeData::clear()
{
	hasclrkey = false;
	hasshadow = false;
	hasplrclrs = false;
}


/*
===============
EncodeData::parse

Parse color codes from section, the following keys are currently known:

clrkey_[r,g,b]		Color key
shadowclr_[r,g,b]	color for shadow pixels
===============
*/
void EncodeData::parse(Section *s)
{
	int i;
	int r, g, b;

	// Read color key
	r = s->get_int("clrkey_r", -1);
	g = s->get_int("clrkey_g", -1);
	b = s->get_int("clrkey_b", -1);
	if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
		hasclrkey = true;
		clrkey.set(r, g, b);
	}

	// Read shadow color
	r = s->get_int("shadowclr_r", -1);
	g = s->get_int("shadowclr_g", -1);
	b = s->get_int("shadowclr_b", -1);
	if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
		hasshadow = true;
		shadow.set(r, g, b);
	}

	// Read player color codes
	for(i = 0; i < 4; i++) {
		char keyname[32];

		snprintf(keyname, sizeof(keyname), "plrclr%i_r", i);
		r = s->get_int(keyname, -1);
		snprintf(keyname, sizeof(keyname), "plrclr%i_g", i);
		g = s->get_int(keyname, -1);
		snprintf(keyname, sizeof(keyname), "plrclr%i_b", i);
		b = s->get_int(keyname, -1);

		if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
			break;

		plrclr[i].set(r, g, b);
	}

	if (i == 4)
		hasplrclrs = true;
}


/*
===============
EncodeData::add

Add another encode data. Already existing color codes are overwritten
===============
*/
void EncodeData::add(const EncodeData *other)
{
	if (other->hasclrkey) {
		hasclrkey = true;
		clrkey = other->clrkey;
	}

	if (other->hasshadow) {
		hasshadow = true;
		shadow = other->shadow;
	}

	if (other->hasplrclrs) {
		hasplrclrs = true;
		for(int i = 0; i < 4; i++)
			plrclr[i] = other->plrclr[i];
	}
}


/*
==============================================================================

AnimationManager IMPLEMENTATION

==============================================================================
*/

AnimationManager g_anim;


AnimationManager::AnimationManager()
{
}

AnimationManager::~AnimationManager()
{
}

/*
===============
AnimationManager::flush

Remove all animations
===============
*/
void AnimationManager::flush()
{
	m_animations.resize(0);
}

/*
===============
AnimationManager::get

Read in basic information about the information.
The graphics are loaded later by the graphics subsystem.

The animation resides in the given directory and is described by the given
section.

This function looks for pictures in this order:
	key 'pics', if present
	picnametempl, if not null
	<sectionname>_??.bmp
===============
*/
uint AnimationManager::get(const char *directory, Section *s, const char *picnametempl,
	                        const EncodeData *encdefaults)
{
	uint id;
	AnimationData* ad;

	m_animations.push_back(AnimationData());
	id = m_animations.size();

	ad = &m_animations[id-1];
	ad->frametime = FRAME_LENGTH;
	ad->hotspot.x = 0;
	ad->hotspot.y = 0;
	ad->encdata.clear();
	ad->picnametempl = "";

	// Determine picture name template
	char templbuf[256]; // used when picnametempl == 0
	char pictempl[256];

	if (s->get_string("pics"))
		picnametempl = s->get_string("pics");
	else {
		if (!picnametempl) {
			snprintf(templbuf, sizeof(templbuf), "%s_??.png", s->get_name());
			picnametempl = templbuf;
		}
	}

	snprintf(pictempl, sizeof(pictempl), "%s/%s", directory, picnametempl);

	ad->picnametempl = pictempl;

   if(ad->picnametempl[strlen(pictempl)-4]=='.') {
      // delete extension
      ad->picnametempl[strlen(pictempl)-4]='\0';
   }

	// Get descriptive data
	if (encdefaults)
		ad->encdata.add(encdefaults);

	ad->encdata.parse(s);

	int fps = s->get_int("fps");
	if (fps > 0)
		ad->frametime = 1000 / fps;

	// TODO: Frames of varying size / hotspot?
	ad->hotspot.x = s->get_int("hot_spot_x", 0);
	ad->hotspot.y = s->get_int("hot_spot_y", 0);

	return id;
}


/*
===============
AnimationManager::get

Return the number of animations.
===============
*/
uint AnimationManager::get_nranimations() const
{
	return m_animations.size();
}


/*
===============
AnimationManager::get_animation

Return AnimationData for this animation.
Returns 0 if the animation doesn't exist.
===============
*/
const AnimationData* AnimationManager::get_animation(uint id) const
{
	if (!id || id > m_animations.size())
		return 0;

	return &m_animations[id-1];
}


/*
==============================================================================

DirAnimations IMPLEMENTAION

==============================================================================
*/

/*
===============
DirAnimations::DirAnimations
DirAnimations::~DirAnimations
===============
*/
DirAnimations::DirAnimations()
{
}

DirAnimations::~DirAnimations()
{
}

/*
===============
DirAnimations::parse

Parse an animation from the given directory and config.
sectnametempl is of the form "foowalk_??", where ?? will be replaced with
nw, ne, e, se, sw and w to get the section names for the animations.

If defaults is not zero, the additional sections are not actually necessary.
If they don't exist, the data is taken from defaults and the bitmaps foowalk_??_nn.bmp
are used.
===============
*/
void DirAnimations::parse(Bob_Descr* b, const char *directory, Profile *prof, const char *sectnametempl,
                          Section *defaults, const EncodeData *encdefaults)
{
	char dirpictempl[256];
	char sectnamebase[256];
	char *repl;
	const char* string;

	if (strchr(sectnametempl, '%'))
		throw wexception("sectnametempl %s contains %%", sectnametempl);

	snprintf(sectnamebase, sizeof(sectnamebase), "%s", sectnametempl);
	repl = strstr(sectnamebase, "??");
	if (!repl)
		throw wexception("DirAnimations section name template %s does not contain %%s", sectnametempl);
	strncpy(repl, "%s", 2);

	if(defaults) string = defaults->get_string("dirpics", 0);
   else string=0;

	if (string) {
		snprintf(dirpictempl, sizeof(dirpictempl), "%s", string);
		repl = strstr(dirpictempl, "!!");
		if (!repl)
			throw wexception("DirAnimations dirpics name templates %s does not contain !!", dirpictempl);
		strncpy(repl, "%s", 2);
	} else {
		snprintf(dirpictempl, sizeof(dirpictempl), "%s_??.png", sectnamebase);
	}

	for(int dir = 1; dir <= 6; dir++) {
		static const char *dirstrings[6] = { "ne", "e", "se", "sw", "w", "nw" };
		char sectname[300];
		Section *s;


		snprintf(sectname, sizeof(sectname), sectnamebase, dirstrings[dir-1]);

      std::string anim_name=sectname;
      
		s = prof->get_section(sectname);
		if (!s) {
			if (!defaults)
				throw wexception("Section [%s] missing and no default supplied", sectname);
			s = defaults;
		}

		snprintf(sectname, sizeof(sectname), dirpictempl, dirstrings[dir-1]);
      m_animations[dir-1] = g_anim.get(directory, s, sectname, encdefaults);
      b->add_animation(anim_name.c_str(), m_animations[dir-1]);
   }
}

