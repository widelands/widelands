/*
 * Copyright (C) 2010-2021 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_MULTIPLAYERSETUPGROUP_H
#define WL_UI_FSMENU_MULTIPLAYERSETUPGROUP_H

#include <memory>

#include "network/network_player_settings_backend.h"
#include "ui_basic/box.h"
#include "ui_basic/panel.h"
#include "ui_basic/textarea.h"

struct GameSettingsProvider;
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
	~MultiPlayerSetupGroup() override;

	void
	force_new_dimensions(uint32_t max_width, uint32_t max_height, uint32_t standard_element_height);
	void update_players();

private:
	void reset();
	void draw(RenderTarget& dst) override;

	GameSettingsProvider* const settings_;
	std::unique_ptr<NetworkPlayerSettingsBackend> npsb;
	std::vector<MultiPlayerClientGroup*> multi_player_client_groups;  // not owned
	std::vector<MultiPlayerPlayerGroup*> multi_player_player_groups;  // not owned
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	UI::Box clientbox, playerbox, scrollable_playerbox;
	UI::Textarea clients_, players_;
	int32_t buth_;

	std::map<std::string, const Image*> tribepics_;
	std::map<std::string, std::string> tribenames_;
	void update_clients();
};

#endif  // end of include guard: WL_UI_FSMENU_MULTIPLAYERSETUPGROUP_H
