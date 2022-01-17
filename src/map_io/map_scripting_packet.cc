/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "map_io/map_scripting_packet.h"

#include <csignal>

#include "base/log.h"
#include "base/macros.h"
#include "base/string.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "scripting/logic.h"

namespace Widelands {

namespace {
constexpr uint32_t kCurrentPacketVersion = 5;

// Write all .lua files that exist in the given 'path' in 'map_fs' to the 'target_fs'.
void write_lua_dir(FileSystem& target_fs, FileSystem* map_fs, const std::string& path) {
	assert(map_fs);
	target_fs.ensure_directory_exists(path);
	for (const std::string& script : map_fs->filter_directory(
	        path, [](const std::string& fn) { return ends_with(fn, ".lua"); })) {
		size_t length;
		void* input_data = map_fs->load(script, length);
		target_fs.write(script, input_data, length);
		free(input_data);
	}
}

// Write a selection of .lua files and all .png files that exist in the given 'path' in 'map_fs' to
// the 'target_fs'.
void write_tribes_dir(FileSystem& target_fs, FileSystem* map_fs, const std::string& path) {
	assert(map_fs);
	target_fs.ensure_directory_exists(path);
	for (const std::string& file : map_fs->list_directory(path)) {
		if (map_fs->is_directory(file)) {
			// Write subdirectories
			write_tribes_dir(target_fs, map_fs, file);
		} else {
			// Write file
			const std::string filename(FileSystem::fs_filename(file.c_str()));
			// TODO(GunChleoc): Savegame compatibility, forbid "helptexts.lua" after v1.0
			if (filename == "init.lua" || filename == "register.lua" || filename == "helptexts.lua" ||
			    ends_with(filename, ".png")) {
				size_t length;
				void* input_data = map_fs->load(file, length);
				target_fs.write(file, input_data, length);
				free(input_data);
			} else {
				log_warn("File name '%s' is not allowed in scenario tribes – expecting "
				         "'init.lua', 'register.lua', 'helptexts.lua' or a *.png file\n",
				         file.c_str());
			}
		}
	}
}
}  // namespace

[[noreturn]] static void abort_handler(int) {
	throw wexception(_("The engine received a SIGABRT signal which was most likely triggered by a "
	                   "corrupted savegame. No solution for this bug has been implemented yet. We "
	                   "are sorry, but this savegame seems to be broken beyond repair."));
}

/*
 * ========================================================================
 *            PUBLIC IMPLEMENTATION
 * ========================================================================
 */
void MapScriptingPacket::read(FileSystem& fs, EditorGameBase& egbase, bool, MapObjectLoader& mol) {
	// Always try to load the global State: even in a normal game, some lua
	// coroutines could run. But make sure that this is really a game, other
	// wise this makes no sense.
	FileRead fr;
	if (egbase.is_game() && fr.try_open(fs, "scripting/globals.dump")) {
		try {
			const uint32_t packet_version = fr.unsigned_32();
			if (packet_version >= 4 && packet_version <= kCurrentPacketVersion) {
				upcast(LuaGameInterface, lgi, &egbase.lua());
				signal(SIGABRT, &abort_handler);
				if (packet_version >= 5) {
					// TODO(Nordfriese): Savegame compatibility)
					lgi->read_textdomain_stack(fr);
				}
				lgi->read_global_env(fr, mol, fr.unsigned_32());
				signal(SIGABRT, SIG_DFL);
			} else {
				throw UnhandledVersionError(
				   "MapScriptingPacket", packet_version, kCurrentPacketVersion);
			}
		} catch (const WException& e) {
			throw GameDataError("scripting: %s", e.what());
		}
		fr.close();
	}
}

void MapScriptingPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver& mos) {
	// Write any scenario scripting files in the map's basic scripting dir
	FileSystem* map_fs = egbase.map().filesystem();
	if (map_fs) {
		write_lua_dir(fs, map_fs, "scripting");
		// Write any custom scenario tribe entities
		if (map_fs->file_exists("scripting/tribes")) {
			write_tribes_dir(fs, map_fs, "scripting/tribes");
		}
	}

	// Dump the global environment if this is a game and not in the editor
	if (upcast(Game, g, &egbase)) {
		upcast(LuaGameInterface, lgi, &g->lua());
		assert(lgi);
		FileWrite fw;

		fw.unsigned_32(kCurrentPacketVersion);
		lgi->write_textdomain_stack(fw);

		const FileWrite::Pos pos = fw.get_pos();
		fw.unsigned_32(0);  // N bytes written, follows below

		uint32_t nwritten = little_32(lgi->write_global_env(fw, mos));
		fw.data(&nwritten, 4, pos);
		fs.ensure_directory_exists("scripting");
		fw.write(fs, "scripting/globals.dump");
	}
}
}  // namespace Widelands
