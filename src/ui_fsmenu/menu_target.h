/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_MENU_TARGET_H
#define WL_UI_FSMENU_MENU_TARGET_H

#include "ui_basic/panel.h"

namespace FsMenu {
enum class MenuTarget {
	kBack = static_cast<int>(UI::Panel::Returncodes::kBack),
	kOk = static_cast<int>(UI::Panel::Returncodes::kOk),

	// Options
	kApplyOptions,

	// Main menu
	kTutorial,
	kContinueLastsave,
	kReplay,
	kOptions,
	kAbout,
	kExit,

	// Single player
	kNewGame,
	kRandomGame,
	kCampaign,
	kLoadGame,

	// Multiplayer
	kMetaserver,
	kOnlineGameSettings,
	kLan,

	// Editor
	kEditorNew,
	kEditorRandom,
	kEditorContinue,
	kEditorLoad,

	// NOCOM used only by LaunchMPG map/save selection applet. Get rid.
	kNormalGame,
	kScenarioGame,
};
}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_MENU_TARGET_H
