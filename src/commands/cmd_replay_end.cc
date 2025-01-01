/*
 * Copyright (C) 2004-2025 by the Widelands Development Team
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

#include "commands/cmd_replay_end.h"

#include "logic/game_controller.h"
#include "ui_basic/messagebox.h"
#include "wui/interactive_base.h"

namespace Widelands {

void CmdReplayEnd::execute(Widelands::Game& game) {
	game.game_controller()->set_desired_speed(0);

	// Need to pull this out into a variable to make the includes script happy
	InteractiveBase* i = game.get_ibase();
	assert(i != nullptr);
	UI::WLMessageBox mmb(i, UI::WindowStyle::kWui, _("End of Replay"),
	                     _("The end of the replay has been reached and the game has "
	                       "been paused. You may unpause the game and continue watching "
	                       "if you want to."),
	                     UI::WLMessageBox::MBoxType::kOk);
	mmb.run<UI::Panel::Returncodes>();
}

}  // namespace Widelands
