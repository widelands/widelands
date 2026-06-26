/*
 * Copyright (C) 2026 by the Widelands Development Team
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

#ifndef WL_LOGIC_GAME_INTERFACE_PROVIDER_H
#define WL_LOGIC_GAME_INTERFACE_PROVIDER_H

#include <memory>

#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

namespace Widelands {
class Game;
}  // namespace Widelands
struct ChatProvider;

class IGameInterface {
public:
	virtual ~IGameInterface() = default;

	virtual void cleanup_for_load() = 0;
	virtual void postload() = 0;
	virtual void info_panel_fast_forward_message_queue() = 0;

	virtual void notify_player_starting_pos(Widelands::PlayerNumber player, Widelands::Coords coords) = 0;
	virtual void load_plugins() = 0;
	virtual void rebuild_main_menu() = 0;

	virtual void log_message(const std::string& message, const std::string& tooltip = std::string()) const = 0;
	inline void log_message(const char* message, const char* tt = nullptr) const {
		log_message(std::string(message), tt == nullptr ? std::string() : std::string(tt));
	}

	virtual Widelands::PlayerNumber player_number() const = 0;

	virtual void main_loop() = 0;
	virtual void end_main_loop() = 0;
};

class IGameInterfaceProvider {
public:
	virtual ~IGameInterfaceProvider() = default;
	virtual std::unique_ptr<IGameInterface> create(Widelands::Game& game, Widelands::PlayerNumber player_number, bool multiplayer, ChatProvider* chat_provider) = 0;
};

#endif  // end of include guard: WL_LOGIC_GAME_INTERFACE_PROVIDER_H
