/*
 * Copyright (C) 2017 by the Widelands Development Team
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

#ifndef WL_LOGIC_AI_DNA_HANDLER_H
#define WL_LOGIC_AI_DNA_HANDLER_H

#include <cstring>
#include <string>

#include <stdint.h>

#include "base/time_string.h"
#include "logic/constants.h"
#include "logic/game.h"
#include "logic/player.h"

namespace Widelands {

/**
 * This handles reading and saving AI DNA to files
 * - reading when initializing new AI
 * - saving AI into files (with timestamp and player number) after mutation
 */
class AiDnaHandler {
public:
	AiDnaHandler();

	static std::string get_ai_dir() {
		return "ai";
	}

	static std::string get_ai_suffix() {
		return "wai";
	}

	void fetch_dna(std::vector<int16_t>&,
	               std::vector<int8_t>&,
	               std::vector<int8_t>&,
	               std::vector<uint32_t>&,
	               uint8_t);
	void dump_output(Widelands::Player::AiPersistentState* pd, uint8_t);
};
}
#endif  // end of include guard: WL_LOGIC_AI_DNA_HANDLER_H
