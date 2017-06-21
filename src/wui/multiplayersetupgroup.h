/*
 * Copyright (C) 2010-2017 by the Widelands Development Team
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

#ifndef WL_WUI_MULTIPLAYERSETUPGROUP_H
#define WL_WUI_MULTIPLAYERSETUPGROUP_H

#include <map>
#include <memory>
#include <string>

#include "graphic/font_handler1.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "network/network_player_settings_backend.h"
#include "ui_basic/box.h"
#include "ui_basic/panel.h"
#include "ui_basic/textarea.h"

struct GameSettingsProvider;
struct MultiPlayerSetupGroupOptions;
struct MultiPlayerClientGroup;
struct MultiPlayerPlayerGroup;

/**
 * struct MultiPlayerSetupGroup
 *
 * A MultiPlayerSetupGroup holds information and provides the UI to set the
 * clients, computers and closed players.
 *
 */
struct MultiPlayerSetupGroup : public UI::Box {
	MultiPlayerSetupGroup(UI::Panel* parent,
	                      int32_t x,
	                      int32_t y,
	                      int32_t w,
	                      int32_t h,
	                      GameSettingsProvider* settings,
	                      uint32_t buth);
	~MultiPlayerSetupGroup();

	void refresh();

private:
	GameSettingsProvider* const s;
	std::unique_ptr<NetworkPlayerSettingsBackend> npsb;
	std::vector<MultiPlayerClientGroup*> multi_player_client_groups;  // not owned
	std::vector<MultiPlayerPlayerGroup*> multi_player_player_groups;  // not owned
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	UI::Box clientbox, playerbox;

	uint32_t buth_;

	std::map<std::string, const Image*> tribepics_;
	std::map<std::string, std::string> tribenames_;
};

#endif  // end of include guard: WL_WUI_MULTIPLAYERSETUPGROUP_H
