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

#include "widelands.h"
#include "profile.h"
#include "graphic.h"
#include "pic.h"
#include "animation.h"


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
		clrkey_r = r;
		clrkey_g = g;
		clrkey_b = b;
	}
	
	// Read shadow color
	r = s->get_int("shadowclr_r", -1);
	g = s->get_int("shadowclr_g", -1);
	b = s->get_int("shadowclr_b", -1);
	if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
		hasshadow = true;
		shadow_r = r;
		shadow_g = g;
		shadow_b = b;
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
		
		plrclr_r[i] = r;
		plrclr_g[i] = g;
		plrclr_b[i] = b;
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
		clrkey_r = other->clrkey_r;
		clrkey_g = other->clrkey_g;
		clrkey_b = other->clrkey_b;
	}
	
	if (other->hasshadow) {
		hasshadow = true;
		shadow_r = other->shadow_r;
		shadow_g = other->shadow_g;
		shadow_b = other->shadow_b;
	}
	
	if (other->hasplrclrs) {
		hasplrclrs = true;
		for(int i = 0; i < 4; i++) {
			plrclr_r[i] = other->plrclr_r[i];
			plrclr_g[i] = other->plrclr_g[i];
			plrclr_b[i] = other->plrclr_b[i];
		}
	}
}


/*
==============================================================================

Animation IMPLEMENTAION

==============================================================================
*/
			
Animation::Animation(void)
{
	npics = 0;
	pics = 0;
	m_frametime = FRAME_LENGTH;
	w = h = 0;
	hsx = hsy = 0;
}

Animation::~Animation(void)
{ 
	if(npics) {
		uint i; 
		for(i=0; i<npics; i++) {
			free(pics[i].data);
		}
		free(pics);
	}
}

/** Animation::add_pic(ushort size, ushort* data)
 *
 * Adds one frame of raw encoded data to the animation
 */
void Animation::add_pic(ushort size, ushort* data)
{
	if(!pics) {
		pics=(Animation_Pic*) malloc(sizeof(Animation_Pic));
		npics=1;
	} else {
		++npics;
		pics=(Animation_Pic*) realloc(pics, sizeof(Animation_Pic)*npics);
	}
	pics[npics-1].data=(ushort*)malloc(size);
	pics[npics-1].parent=this;
	memcpy(pics[npics-1].data, data, size);
}

/** Animation::add_pic(Pic* pic, bool hasclrkey, ushort clrkey, bool hasshadow, ushort shadowclr)
 *
 * Adds one frame to the animation
 */
void Animation::add_pic(Pic* pic, const EncodeData *encdata)
{
	if(pic->get_w() != w && pic->get_h() != h)
		throw wexception("frame must be %ix%i pixels big", w, h);

	// Pack the EncodeData colorkey&co. to 16 bit
	ushort clrkey = 0;
	ushort shadowclr = 0;
	int hasplrclrs = 0;
	ushort plrclrs[4];
	
	if (encdata->hasclrkey)
		clrkey = pack_rgb(encdata->clrkey_r, encdata->clrkey_g, encdata->clrkey_b);
	if (encdata->hasshadow)
		shadowclr = pack_rgb(encdata->shadow_r, encdata->shadow_g, encdata->shadow_b);
	if (encdata->hasplrclrs) {
		hasplrclrs = 4;
		for(int i = 0; i < 4; i++)
			plrclrs[i] = pack_rgb(encdata->plrclr_r[i], encdata->plrclr_g[i], encdata->plrclr_b[i]);
	}
	
	// Ready to encode	
   ushort* data = 0;
	uint in, out;
	int runstart = -1; // code field for normal run
	uint npix = pic->get_w()*pic->get_h();
	ushort *pixels = pic->get_pixels();
	
	try
	{
		data = (ushort*) malloc(pic->get_w()*pic->get_h()*sizeof(ushort)*2); 
		
		in = 0;
		out = 0;
		while(in < npix)
		{
			uint count;
	
			// Deal with transparency
			if (encdata->hasclrkey && pixels[in] == clrkey) {
				if (runstart >= 0) { // finish normal run
					data[runstart] = out-runstart-1;
					runstart = -1;
				}
					
				count = 1;
				in++;
				while(in < npix && pixels[in] == clrkey) {
					count++;
					in++;
				}
				
				data[out++] = (1<<14) | count;
				continue;
			}
			
			// Deal with shadow
			if (encdata->hasshadow && pixels[in] == shadowclr) {
				if (runstart >= 0) { // finish normal run
					data[runstart] = out-runstart-1;
					runstart = -1;
				}
					
				count = 1;
				in++;
				while(in < npix && pixels[in] == shadowclr) {
					count++;
					in++;
				}
				
				data[out++] = (3<<14) | count;
				continue;
   		}
			
			// Check if it's a player color
			if (hasplrclrs) {
				int idx;
				
	         for(idx = 0; idx < hasplrclrs; idx++) {
            	if(pixels[in] == plrclrs[idx])
						break;
				}
				
				if (idx < hasplrclrs) {
					if (runstart >= 0) { // finish normal run
						data[runstart] = out-runstart-1;
						runstart = -1;
					}
					
					count = 1;
					in++;
					while(in < npix && pixels[in] == shadowclr) {
						count++;
						in++;
					}
					
					data[out++] = (2<<14) | count;
					data[out++] = idx;
					continue;
            }
         }
			
			// Normal run of pixels
			if (runstart < 0)
				runstart = out++;
			
			data[out++] = pixels[in++];
      }
   
		if (runstart >= 0) { // finish normal run
			data[runstart] = out-runstart-1;
			runstart = -1;
		}
		
		// Store the frame
		add_pic(out*sizeof(short), data);
	} catch(...) {
		if (data)
			free(data);
		throw;
	}
	
	free(data);
}


/*
===============
Animation::parse

Read an animation which sits in the given directory.
The animation is described by the given section.

This function looks for pictures in this order:
	key 'pics', if present
	picnametempl, if not null
	<sectionname>_??.bmp
===============
*/
void Animation::parse(const char *directory, Section *s, const char *picnametempl, const EncodeData *encdefaults)
{
	char templbuf[256]; // used when picnametempl == 0
	char pictempl[256];

	if (s->get_string("pics"))
		picnametempl = s->get_string("pics");
	else {
		if (!picnametempl) {
			snprintf(templbuf, sizeof(templbuf), "%s_??.bmp", s->get_name());
			picnametempl = templbuf;
		}
	}
	
	snprintf(pictempl, sizeof(pictempl), "%s/%s", directory, picnametempl);

	// Get colorkey, shadow color and hotspot
	EncodeData encdata;
	
	encdata.clear();
	
	if (encdefaults)
		encdata.add(encdefaults);
	
	encdata.parse(s);

	// Get animation speed
	int fps = s->get_int("fps");
	if (fps > 0)
		m_frametime = 1000 / fps;
	
	// TODO: Frames of varying size / hotspot?
	hsx = s->get_int("hot_spot_x", 0);
	hsy = s->get_int("hot_spot_y", 0);
	
	// Read frames in one by one
	for(;;) {
		char fname[256];
		int nr = npics;
		char *p;
		
		// create the file name by reverse-scanning for '?' and replacing
		strcpy(fname, pictempl);
		p = fname + strlen(fname);
		while(p > fname) {
			if (*--p != '?')
				continue;
			
			*p = '0' + (nr % 10);
			nr = nr / 10;
		}
		
		if (nr) // cycled up to maximum possible frame number
			break;
		
		// is the frame actually there?
		if (!g_fs->FileExists(fname))
			break;
	
		Pic* pic = new Pic();
		if (pic->load(fname)) {
			delete pic;
			break;
		}

		if (!npics) {
			w = pic->get_w();
			h = pic->get_h();
		}
		
		if(pic->get_w() != w && pic->get_h() != h) {
			delete pic;
			throw wexception("%s: frame must be %ix%i pixels big", fname, w, h);
		}
		
		try {
			add_pic(pic, &encdata);
		} catch(...) {
			delete pic;
			throw;
		}
		
		delete pic;
	}
	
	if (!npics)
		throw wexception("Animation %s has no frames", pictempl);
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
void DirAnimations::parse(const char *directory, Profile *prof, const char *sectnametempl,
                          Section *defaults, const EncodeData *encdefaults)
{
	char sectnamebase[256];
	char *repl;
	
	if (strchr(sectnametempl, '%'))
		throw wexception("sectnametempl %s contains %%", sectnametempl);
	
	snprintf(sectnamebase, sizeof(sectnamebase), "%s", sectnametempl);
	repl = strstr(sectnamebase, "??");
	if (!repl)
		throw wexception("DirAnimations section name template %s does not contain %%s", sectnametempl);
	strncpy(repl, "%s", 2);
	
	for(int dir = 1; dir <= 6; dir++) {
		static const char *dirstrings[6] = { "ne", "e", "se", "sw", "w", "nw" };
		char sectname[300];
		Section *s;
		
		snprintf(sectname, sizeof(sectname), sectnamebase, dirstrings[dir-1]);
		
		s = prof->get_section(sectname);
		if (!s) {
			if (!defaults)
				throw wexception("Section [%s] missing and no default supplied", sectname);
			s = defaults;
		}
		
		strcat(sectname, "_??.bmp");
		m_animations[dir-1].parse(directory, s, sectname, encdefaults);
	}
}


