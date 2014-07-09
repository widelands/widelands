/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "graphic/pam_io.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SDL.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "graphic/image_io.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"

using namespace boost::algorithm;
using namespace boost;

namespace {

std::map<std::string, std::string> read_header(FileRead* fr) {
	std::map<std::string, std::string> header;

	std::string first_line = fr->ReadLine();
	trim(first_line);
	if (first_line != "P7") {
		throw ImageLoadingError(
		   "<stream>", (format("Invalid format %s. Can only read P7.") % first_line).str());
	}

	for (;;) {
		std::string line = fr->ReadLine();
		trim(line);
		if (line == "ENDHDR") {
			break;
		}
		std::vector<std::string> parts = split_string(line, " ");
		if (parts.size() != 2) {
			throw ImageLoadingError("<stream>", (format("Invalid line in header '%s'") % line).str());
		}
		for (std::string& part : parts) {
			trim(part);
		}
		header[parts[0]] = parts[1];
	}
	return header;
}

}  // namespace

void save_pam(Surface* surface, FileWrite* fw) {
	const auto write_string = [&fw] (const std::string& s) {
		fw->Data(s.data(), s.size());
	};
	write_string("P7\n");
	write_string((format("WIDTH %i\n") % surface->width()).str());
	write_string((format("HEIGHT %i\n") % surface->height()).str());
	write_string("DEPTH 4\n");
	write_string("MAXVAL 255\n");
	write_string("TUPLTYPE RGB_ALPHA\n");
	write_string("ENDHDR\n");

	surface->lock(Surface::Lock_Normal);

	uint8_t r, g, b, a;
	for (int y = 0; y < surface->height(); ++y) {
		for (int x = 0; x < surface->width(); ++x) {
			SDL_GetRGBA(surface->get_pixel(x, y), &surface->format(), &r, &g, &b, &a);

			fw->Unsigned8(r);
			fw->Unsigned8(g);
			fw->Unsigned8(b);
			fw->Unsigned8(a);
		}
	}

	surface->unlock(Surface::Unlock_NoChange);
}

std::unique_ptr<Surface> load_pam(FileRead* fr) {
	auto header = read_header(fr);
	const auto save_header_entry = [&header](const std::string & key) {
		if (header.count(key) == 0) {
			throw ImageLoadingError("<stream>", (format("Missing header key '%s'") % key).str());
		}
		return header[key];
	};

	const auto check_header_entry = [&header, &save_header_entry](
	   const std::string& key, const std::string& wanted_value) {
		const std::string seen_value = save_header_entry(key);
		if (seen_value != wanted_value) {
			throw ImageLoadingError(
			   "<stream>",
			   (format("Invalid %s '%s'. Needs to be %s.") % key % seen_value % wanted_value).str());
		}
	};

	check_header_entry("TUPLTYPE", "RGB_ALPHA");
	check_header_entry("DEPTH", "4");
	check_header_entry("MAXVAL", "255");

	int width = lexical_cast<int>(save_header_entry("WIDTH"));
	int height = lexical_cast<int>(save_header_entry("HEIGHT"));

	std::unique_ptr<Surface> surface(Surface::create(width, height));
	surface->lock(Surface::Lock_Discard);

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			uint8_t r = fr->Unsigned8();
			uint8_t g = fr->Unsigned8();
			uint8_t b = fr->Unsigned8();
			uint8_t a = fr->Unsigned8();

			surface->set_pixel(x, y, SDL_MapRGBA(&surface->format(), r, g, b, a));
		}
	}

	surface->unlock(Surface::Unlock_Update);

	return surface;
}
