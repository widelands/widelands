/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#include "map_io/map_extradata_packet.h"

#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "profile/profile.h"

namespace Widelands {

constexpr int32_t kCurrentPacketVersion = 1;

void MapExtradataPacket::read(FileSystem& fs, bool const skip) {
	if (skip)
		return;

	Profile prof;
	try {prof.read("extra_data", nullptr, fs);} catch (...) {return;}

	try {
		int32_t const packet_version =
			prof.get_safe_section("global").get_safe_int("packet_version");
		if (packet_version == kCurrentPacketVersion) {
			// Read all pics.
			if (fs.file_exists("pics") && fs.is_directory("pics")) {
				FilenameSet pictures = fs.list_directory("pics");
				for
					(FilenameSet::iterator pname = pictures.begin();
					 pname != pictures.end();
					 ++pname)
				{
					if (fs.is_directory(pname->c_str())) // Might be a dir, maybe CVS
						continue;

					const std::string hash = std::string("map:") + FileSystem::fs_filename(pname->c_str());
					const Image* image = nullptr;
					if (!g_gr->images().has(hash)) {
						image = g_gr->images().insert(hash, load_image(*pname, &fs));
					} else {
						image = g_gr->images().get(hash);
					}
					assert(image);
				}
			}
		} else {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
		throw GameDataError("extradata: %s", e.what());
	}
}


void MapExtradataPacket::write
	(FileSystem & fs, EditorGameBase & egbase)
{
	Profile prof;
	prof.create_section("global").set_int
		("packet_version", kCurrentPacketVersion);

	// Copy all files from pics/ from the old map to the new.
	FileSystem* map_fs = egbase.map().filesystem();
	if (map_fs && map_fs->file_exists("pics") && map_fs->is_directory("pics")) {
		fs.ensure_directory_exists("pics");
		for (const std::string& picture : map_fs->list_directory("pics")) {
			size_t length;
			void* input_data = map_fs->load(picture, length);
			fs.write(picture, input_data, length);
			free(input_data);
		}
	}
	prof.write("extra_data", false, fs);
}

}
