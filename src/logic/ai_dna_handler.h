/*
 * Copyright (C) 2007-2017 by the Widelands Development Team
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

#ifndef WL_LOGIC_AI_DUMP_H
#define WL_LOGIC_AI_DUMP_H

#define AI_SUFFIX "wai"

/**
 * Saves newly generated AI DNA to a text files
 *
 * Allows to use file with AI DNA as a basis for DNA mutation and initialization
 * playercommands.
 */

#include <cstring>
#include <string>

#include <stdint.h>

#include "base/time_string.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/player.h"

//#define REPLAY_DIR "ai_dna"
#define SUFFIX ".wai"

// struct Md5Checksum;

// class StreamRead;
// class StreamWrite;

// namespace Widelands {
// class Player;
//}

class AiDnaHandler {
public:
	AiDnaHandler();

	static std::string get_base_dir() {
		return "ai";
	}

	void fetch_dna(std::vector<int16_t>&,
	               std::vector<int8_t>&,
	               std::vector<int8_t>&,
	               std::vector<uint32_t>&,
	               uint8_t);
	void dump_output(Widelands::Player::AiPersistentState* pd, uint8_t);

private:
	FileSystem::Type fs_type_;
};

#endif  // end of include guard: WL_LOGIC_AI_DUMP_H
