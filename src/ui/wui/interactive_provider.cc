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

#include "ui/wui/interactive_provider.h"

#include <memory>

#include "ui/wui/interactive_player.h"
#include "ui/wui/interactive_spectator.h"

std::unique_ptr<IGameInterface>
UserInterfaceProvider::create(Widelands::Game& game,
                              const Widelands::PlayerNumber player_number,
                              const bool multiplayer,
                              ChatProvider* chat_provider) {
	if (player_number != 0) {
		return std::make_unique<InteractivePlayer>(
		   game, get_config_section(), player_number, multiplayer, chat_provider);
	}
	return std::make_unique<InteractiveSpectator>(
	   game, get_config_section(), multiplayer, chat_provider);
}
