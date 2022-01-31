/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#ifndef WL_AI_COMPUTER_PLAYER_H
#define WL_AI_COMPUTER_PLAYER_H

#include <cassert>

#include "base/macros.h"
#include "base/string.h"
#include "logic/widelands.h"

// We need to use a string prefix in the game setup screens to identify the AIs, so we make sure
// that the AI names don't contain the separator that's used to parse the strings there.
constexpr char kAiNameSeparator = '|';
static const std::string kAiNamePrefix =  // comment to fix codecheck
   std::string("ai") + as_string(kAiNameSeparator);
static const std::string kRandom = "random";
static const std::string kRandomAiName = kAiNamePrefix + kRandom;

namespace Widelands {
class Game;
}  // namespace Widelands

namespace AI {

/**
 * The generic interface to AI instances, or "computer players".
 *
 * Instances of actual AI implementation can be created via the
 * \ref Implementation interface.
 */
struct ComputerPlayer {
	ComputerPlayer(Widelands::Game&, const Widelands::PlayerNumber);
	virtual ~ComputerPlayer() = default;

	virtual void think() = 0;

	Widelands::Game& game() const {
		return game_;
	}
	Widelands::PlayerNumber player_number() {
		return player_number_;
	}

	/**
	 * Interface to a concrete implementation, used to instantiate AIs.
	 *
	 * \see get_implementations()
	 */
	struct Implementation {
		enum class Type { kEmpty, kDefault };

		std::string name;
		std::string descname;
		std::string icon_filename;
		Type type;
		explicit Implementation(const std::string& init_name,
		                        const std::string& init_descname,
		                        const std::string& init_icon_filename,
		                        Type init_type)
		   : name(init_name),
		     descname(init_descname),
		     icon_filename(init_icon_filename),
		     type(init_type) {
			assert(!contains(name, as_string(kAiNameSeparator)));
		}

		virtual ~Implementation() {
		}
		virtual ComputerPlayer* instantiate(Widelands::Game&, Widelands::PlayerNumber) const = 0;
	};
	using ImplementationVector = std::vector<ComputerPlayer::Implementation const*>;

	/**
	 * Get a list of available AI implementations.
	 */
	static const ImplementationVector& get_implementations();

	/**
	 * Get the best matching implementation for this name.
	 */
	static const Implementation* get_implementation(const std::string& name);

private:
	Widelands::Game& game_;
	Widelands::PlayerNumber const player_number_;

	DISALLOW_COPY_AND_ASSIGN(ComputerPlayer);
};
}  // namespace AI
#endif  // end of include guard: WL_AI_COMPUTER_PLAYER_H
