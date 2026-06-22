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

#ifndef WL_UI_WUI_INTERACTIVE_PROVIDER_H
#define WL_UI_WUI_INTERACTIVE_PROVIDER_H

#include <memory>

#include "logic/game_interface_provider.h"

class UserInterfaceProvider : public IGameInterfaceProvider {
public:
	std::unique_ptr<IGameInterface> create(Widelands::Game& game, Widelands::PlayerNumber player_number, bool multiplayer, ChatProvider* chat_provider) override;
};

#endif  // end of include guard: WL_UI_WUI_INTERACTIVE_PROVIDER_H
