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

#include "ai/computer_player.h"

#include "ai/defaultai.h"

namespace AI {

ComputerPlayer::ComputerPlayer(Widelands::Game& g, Widelands::PlayerNumber const pid)
   : game_(g), player_number_(pid) {
}

struct EmptyAI : ComputerPlayer {
	EmptyAI(Widelands::Game& g, const Widelands::PlayerNumber pid) : ComputerPlayer(g, pid) {
	}

	void think() override {
	}

	struct EmptyAIImpl : Implementation {
		EmptyAIImpl()
		   : Implementation(
		        "empty",
		        /** TRANSLATORS: This is the name of an AI used in the game setup screens */
		        _("No AI"),
		        "images/ai/ai_empty.png",
		        Implementation::Type::kEmpty) {
		}
		ComputerPlayer* instantiate(Widelands::Game& g,
		                            Widelands::PlayerNumber const pid) const override {
			return new EmptyAI(g, pid);
		}
	};

	static EmptyAIImpl implementation;
};

EmptyAI::EmptyAIImpl EmptyAI::implementation;

const ComputerPlayer::ImplementationVector& ComputerPlayer::get_implementations() {
	static std::vector<ComputerPlayer::Implementation const*> impls;

	if (impls.empty()) {
		impls.push_back(&DefaultAI::normal_impl);
		impls.push_back(&DefaultAI::weak_impl);
		impls.push_back(&DefaultAI::very_weak_impl);
		impls.push_back(&EmptyAI::implementation);
	}

	return impls;
}

const ComputerPlayer::Implementation* ComputerPlayer::get_implementation(const std::string& name) {
	const ImplementationVector& vec = get_implementations();

	for (const ComputerPlayer::Implementation* implementation : vec) {
		if (implementation->name == name) {
			return implementation;
		}
	}
	return vec[0];
}
}  // namespace AI
