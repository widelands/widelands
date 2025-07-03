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

#include "commands/cmd_replay_sync_read.h"

#include "build_info.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/replay.h"
#include "ui_basic/messagebox.h"
#include "wui/interactive_base.h"

namespace Widelands {

const Game* CmdReplaySyncRead::reported_desync_for_(nullptr);

void CmdReplaySyncRead::execute(Game& game) {
	if (reported_desync_for_ == &game) {
		// We already know there was a desync
		return;
	}

	const crypto::MD5Checksum myhash = game.get_sync_hash();

	if (hash_ != myhash) {
		reported_desync_for_ = &game;
		log_err_time(game.get_gametime(),
		             "REPLAY: Lost synchronization at time %u\n"
		             "I have:     %s\n"
		             "Replay has: %s\n",
		             duetime().get(), myhash.str().c_str(), hash_.str().c_str());

		// In case syncstream logging is on, save it for analysis
		game.save_syncstream(true);

		// There has to be a better way to do this.
		game.game_controller()->set_desired_speed(0);

		UI::WLMessageBox m(
		   game.get_ibase(), UI::WindowStyle::kWui, _("Desync"),
		   format(_("The replay has desynced and the game was paused.\n"
		            "You are probably watching a replay created with another version of "
		            "Widelands, which is not supported.\n\n"
		            "If you are certain that the replay was created with the same version "
		            "of Widelands, %1$s, please report this problem as a bug.\n"
		            "You will find related messages in the standard output (stdout.txt on "
		            "Windows). Please add this information to your report."),
		          build_ver_details()),
		   UI::WLMessageBox::MBoxType::kOk);
		m.run<UI::Panel::Returncodes>();
	}
}

}  // namespace Widelands
