/*
 * Copyright (C) 2002-2004, 2006, 2009-2011 by the Widelands Development Team
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

#include "animation_gfx.h"

#include <cassert>

#include <SDL.h>

#include "animation.h"
#include "image.h"
#include "image_cache.h"
#include "image_transformations.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "wexception.h"

static const uint32_t nextensions = 2;
static const char extensions[nextensions][5] = {".png", ".jpg"};
AnimationGfx::AnimationGfx(const AnimationData& data, ImageCache* image_cache)
	: m_hotspot(data.hotspot), m_hasplrclrs(data.hasplrclrs) {
	//  In the filename template, the last sequence of '?' characters (if any)
	//  is replaced with a number, for example the template "idle_??" is
	//  replaced with "idle_00". Then the code looks if there is a file with
	//  that name + any extension in the list above. Assuming that it finds a
	//  file, it increments the number so that the filename is "idle_01" and
	//  looks for a file with that name + extension, and so on until it can not
	//  find any file. Then it is assumed that there are no more frames in the
	//  animation.

	//  Allocate a buffer for the filename. It must be large enough to hold the
	//  image name template, the "_pc" (3 characters) part for playercolor
	//  masks, the extension (4 characters including the dot) and the null
	//  terminator. Copy the image name template into the buffer.
	char filename[256];
	std::string::size_type const picnametempl_size = data.picnametempl.size();
	if (sizeof(filename) < picnametempl_size + 3 + 4 + 1)
		throw wexception
			("buffer too small (%lu) for image name template of size %lu\n",
			 static_cast<long unsigned>(sizeof(filename)), static_cast<long unsigned>(picnametempl_size));
	strcpy(filename, data.picnametempl.c_str());

	//  Find out where in the image name template the number is. Search
	//  backwards from the end.
	char * const after_basename = filename + picnametempl_size;
	char * last_digit = after_basename;
	while (filename <= last_digit and *last_digit != '?')
		--last_digit;
	char * before_first_digit = last_digit;
	while (filename <= before_first_digit and *before_first_digit == '?') {
		*before_first_digit = '0';
		--before_first_digit;
	}
	unsigned int width = 0, height = 0;

	for (;;) {
		// Load the base image
		for (size_t extnr = 0;;) {
			strcpy(after_basename, extensions[extnr]);
			if (g_fs->FileExists(filename)) { //  Is the frame actually there?
				try {
					const Image* image = image_cache->get(filename);
					if (width == 0) { //  This is the first frame.
						width  = image->width();
						height = image->height();
					} else if (width != image->width() or height != image->height())
						throw wexception
							("wrong size: (%u, %u), should be (%u, %u) like the "
							 "first frame",
							 image->width(), image->height(), width, height);
					//  Get a new AnimFrame.
					m_frames.push_back(image);
				} catch (const std::exception & e) {
					throw wexception
						("could not load animation frame %s: %s\n",
						 filename, e.what());
				}
				//  Successfully loaded the frame.
				break;  //  No need to look for files with other extensions.
			} else if (++extnr == nextensions) //  Tried all extensions.
				goto end;  //  This frame does not exist. No more frames in anim.
		}

		if (m_hasplrclrs) {
			//TODO Do not load playercolor mask as opengl texture or use it as
			//     opengl texture.
			strcpy(after_basename, "_pc");
			for (size_t extnr = 0;;) {
				strcpy(after_basename + 3, extensions[extnr]);
				if (g_fs->FileExists(filename)) {
					try {
						const Image* image = image_cache->get(filename);
						if (width != image->width() or height != image->height())
							throw wexception
								("playercolor mask has wrong size: (%u, %u), should "
								 "be (%u, %u) like the animation frame",
								 image->width(), image->height(), width, height);
						m_pcmasks.push_back(image);
						break;
					} catch (const std::exception & e) {
						throw wexception
							("error while reading \"%s\": %s", filename, e.what());
					}
				} else if (++extnr == nextensions) {
					after_basename[3] = '\0'; //  cut off the extension
					throw wexception("\"%s\" is missing", filename);
				}
			}
		}

		//  Increment the number in the filename.
		for (char * digit_to_increment = last_digit;;) {
			if (digit_to_increment == before_first_digit)
				goto end; //  The number wrapped around to all zeros.
			assert('0' <= *digit_to_increment);
			assert(*digit_to_increment <= '9');
			if (*digit_to_increment == '9') {
				*digit_to_increment = '0';
				--digit_to_increment;
			} else {
				++*digit_to_increment;
				break;
			}
		}
	}
end:
	if (m_frames.empty())
		throw wexception
			("animation %s has no frames", data.picnametempl.c_str());

	if (m_pcmasks.size() and m_pcmasks.size() < m_frames.size())
		throw wexception
			("animation has %" PRIuS " frames but playercolor mask has only %" PRIuS " frames",
			 m_frames.size(), m_pcmasks.size());
}

const Image& AnimationGfx::get_frame(size_t i, const RGBColor& playercolor) {
	assert(i < nr_frames());

	const Image& original = get_frame(i);
	if (!m_hasplrclrs)
		return original;

	assert(m_frames.size() == m_pcmasks.size());

	return *ImageTransformations::player_colored(playercolor, &original, m_pcmasks[i]);
}

const Image& AnimationGfx::get_frame(size_t i) const {
	assert(i < nr_frames());
	return *m_frames[i];
}


