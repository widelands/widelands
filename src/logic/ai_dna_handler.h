/*
 * Copyright (C) 2017-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_AI_DNA_HANDLER_H
#define WL_LOGIC_AI_DNA_HANDLER_H

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

	void fetch_dna(std::vector<int16_t>&,
	               std::vector<int8_t>&,
	               std::vector<int8_t>&,
	               std::vector<uint32_t>&,
	               uint8_t);
	void dump_output(Widelands::Player::AiPersistentState* pd, uint8_t);
};
}  // namespace Widelands
#endif  // end of include guard: WL_LOGIC_AI_DNA_HANDLER_H
