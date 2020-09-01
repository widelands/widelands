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

#include "wui/playerdescrgroup.h"

#include <memory>

#include "ai/computer_player.h"
#include "base/i18n.h"
#include "base/wexception.h"
#include "logic/game_settings.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/textarea.h"

struct PlayerDescriptionGroupImpl {
	GameSettingsProvider* settings;
	uint32_t plnum;

	UI::Textarea* plr_name;
	UI::Checkbox* btnEnablePlayer;
	UI::Button* btnPlayerTeam;
	UI::Button* btnPlayerType;
	UI::Button* btnPlayerTribe;
	UI::Dropdown<uint8_t>* btnPlayerInit;
};

PlayerDescriptionGroup::PlayerDescriptionGroup(UI::Panel* const parent,
                                               int32_t const x,
                                               int32_t const y,
                                               int32_t const w,
                                               int32_t const h,
                                               GameSettingsProvider* const settings,
                                               uint32_t const plnum)
   : UI::Panel(parent, x, y, w, h), d(new PlayerDescriptionGroupImpl) {
	d->settings = settings;
	d->plnum = plnum;

	int32_t xplrname = 0;
	int32_t xplayertype = w * 35 / 125;
	int32_t xplayerteam = w * 35 / 125;
	int32_t xplayertribe = w * 80 / 125;
	int32_t xplayerinit = w * 55 / 125;
	d->plr_name = new UI::Textarea(this, xplrname, 0, xplayertype - xplrname, h);
	d->btnEnablePlayer = new UI::Checkbox(this, Vector2i(xplayertype - 23, 0), "");
	d->btnEnablePlayer->changedto.connect([this](bool b) { enable_player(b); });
	d->btnPlayerType =
	   new UI::Button(this, "player_type", xplayertype, 0, xplayertribe - xplayertype - 2, h / 2,
	                  UI::ButtonStyle::kFsMenuSecondary, "");
	d->btnPlayerType->sigclicked.connect([this]() { toggle_playertype(); });
	d->btnPlayerTeam =
	   new UI::Button(this, "player_team", xplayerteam, h / 2, xplayerinit - xplayerteam - 2, h / 2,
	                  UI::ButtonStyle::kFsMenuSecondary, "");
	d->btnPlayerTeam->sigclicked.connect([this]() { toggle_playerteam(); });
	d->btnPlayerTribe = new UI::Button(this, "player_tribe", xplayertribe, 0, w - xplayertribe,
	                                   h / 2, UI::ButtonStyle::kFsMenuSecondary, "");
	d->btnPlayerTribe->sigclicked.connect([this]() { toggle_playertribe(); });
	d->btnPlayerInit = new UI::Dropdown<uint8_t>(
	   this, "player_initialization", xplayerinit, h / 2, w - xplayerinit, 6, h / 2, "",
	   UI::DropdownType::kTextual, UI::PanelStyle::kFsMenu, UI::ButtonStyle::kFsMenuSecondary);
	d->btnPlayerInit->selected.connect(
	   [this]() { d->settings->set_player_init(d->plnum, d->btnPlayerInit->get_selected()); });

	update();
}

PlayerDescriptionGroup::~PlayerDescriptionGroup() {
}

/**
 * Update display and enabled buttons based on current settings.
 */
void PlayerDescriptionGroup::update() {
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size()) {
		set_visible(false);
		return;
	}

	set_visible(true);

	const PlayerSettings& player = settings.players[d->plnum];
	bool stateaccess = d->settings->can_change_player_state(d->plnum);
	bool tribeaccess = d->settings->can_change_player_tribe(d->plnum);
	bool const initaccess = d->settings->can_change_player_init(d->plnum);
	bool teamaccess = d->settings->can_change_player_team(d->plnum);

	d->btnEnablePlayer->set_enabled(stateaccess);

	if (player.state == PlayerSettings::State::kClosed) {
		d->btnEnablePlayer->set_state(false);
		d->btnPlayerTeam->set_visible(false);
		d->btnPlayerTeam->set_enabled(false);
		d->btnPlayerType->set_visible(false);
		d->btnPlayerType->set_enabled(false);
		d->btnPlayerTribe->set_visible(false);
		d->btnPlayerTribe->set_enabled(false);
		d->btnPlayerInit->set_visible(false);
		d->btnPlayerInit->set_enabled(false);
		d->plr_name->set_text(std::string());
	} else {
		d->btnEnablePlayer->set_state(true);
		d->btnPlayerType->set_visible(true);
		d->btnPlayerType->set_enabled(stateaccess);

		if (player.state == PlayerSettings::State::kOpen) {
			d->btnPlayerType->set_title(_("Open"));
			d->btnPlayerTeam->set_visible(false);
			d->btnPlayerTeam->set_visible(false);
			d->btnPlayerTribe->set_visible(false);
			d->btnPlayerInit->set_visible(false);
			d->btnPlayerTribe->set_enabled(false);
			d->btnPlayerInit->set_enabled(false);
			d->plr_name->set_text(std::string());
		} else {
			std::string title;

			if (player.state == PlayerSettings::State::kComputer) {
				if (player.ai.empty()) {
					title = _("Computer");
				} else {
					if (player.random_ai) {
						title += _("Random AI");
					} else {
						const ComputerPlayer::Implementation* impl =
						   ComputerPlayer::get_implementation(player.ai);
						title = _(impl->descname);
					}
				}
			} else {  // PlayerSettings::State::stateHuman
				title = _("Human");
			}
			d->btnPlayerType->set_title(title);

			Widelands::TribeBasicInfo info = settings.get_tribeinfo(player.tribe);
			if (!tribenames_[player.tribe].size()) {
				// Tribe's localized name
				tribenames_[player.tribe] = info.descname;
			}
			if (player.random_tribe) {
				d->btnPlayerTribe->set_title(pgettext("tribe", "Random"));
				d->btnPlayerTribe->set_tooltip(_("The tribe will be set at random."));
			} else {
				d->btnPlayerTribe->set_title(tribenames_[player.tribe]);
				d->btnPlayerTribe->set_tooltip(info.tooltip);
			}

			d->plr_name->set_text(player.name);

			if (player.team) {
				d->btnPlayerTeam->set_title(std::to_string(static_cast<unsigned int>(player.team)));
			} else {
				d->btnPlayerTeam->set_title("--");
			}

			d->btnPlayerTeam->set_visible(true);
			d->btnPlayerTribe->set_visible(true);
			d->btnPlayerInit->set_visible(true);
			d->btnPlayerTeam->set_enabled(teamaccess);
			d->btnPlayerTribe->set_enabled(tribeaccess);
			d->btnPlayerInit->set_enabled(initaccess);
		}
	}
	update_playerinit();
}

/**
 * The checkbox to open/close a player position has been pressed.
 */
void PlayerDescriptionGroup::enable_player(bool on) {
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size()) {
		return;
	}

	if (on) {
		if (settings.players[d->plnum].state == PlayerSettings::State::kClosed) {
			d->settings->set_player_state(d->plnum, PlayerSettings::State::kComputer);
		}
	} else {
		if (settings.players[d->plnum].state != PlayerSettings::State::kClosed) {
			d->settings->set_player_state(d->plnum, PlayerSettings::State::kClosed);
		}
	}
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kPlayer, d->plnum));
}

void PlayerDescriptionGroup::toggle_playertype() {
	d->settings->next_player_state(d->plnum);
	update();
}

/**
 * Cycle through available tribes for the player.
 */
void PlayerDescriptionGroup::toggle_playertribe() {
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size()) {
		return;
	}

	const PlayerSettings& player = settings.players.at(d->plnum);
	const std::string& currenttribe = player.tribe;
	std::string nexttribe = settings.tribes.at(0).name;
	bool random_tribe = false;
	uint32_t num_tribes = settings.tribes.size();

	if (player.random_tribe) {
		nexttribe = settings.tribes.at(0).name;
	} else if (player.tribe == settings.tribes.at(num_tribes - 1).name) {
		nexttribe = "Random";
		random_tribe = true;
	} else {
		for (uint32_t i = 0; i < num_tribes - 1; ++i) {
			if (settings.tribes[i].name == currenttribe) {
				nexttribe = settings.tribes.at(i + 1).name;
				break;
			}
		}
	}

	d->settings->set_player_tribe(d->plnum, nexttribe, random_tribe);
	update();
}

/**
 * Cycle through team numbers.
 *
 * \note Since each "useful" team needs at least two players, the maximum
 * number of teams is the number of players divided by two. Lonely players
 * can just have team number set to 0.
 */
void PlayerDescriptionGroup::toggle_playerteam() {
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size()) {
		return;
	}

	Widelands::TeamNumber currentteam = settings.players[d->plnum].team;
	Widelands::TeamNumber maxteam = settings.players.size() / 2;
	Widelands::TeamNumber newteam;

	if (currentteam >= maxteam) {
		newteam = 0;
	} else {
		newteam = currentteam + 1;
	}

	d->settings->set_player_team(d->plnum, newteam);
	update();
}

void PlayerDescriptionGroup::update_playerinit() {
	const size_t selection =
	   d->btnPlayerInit->has_selection() ? d->btnPlayerInit->get_selected() : -1;
	d->btnPlayerInit->clear();

	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size()) {
		return;
	}

	if (settings.scenario) {
		d->btnPlayerInit->add(
		   _("Scenario"), -1, nullptr, true, _("Start type is set via the scenario"));
		return;
	}

	const PlayerSettings& player = settings.players[d->plnum];
	std::set<std::string> tags;
	if (!settings.mapfilename.empty()) {
		Widelands::Map map;
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(settings.mapfilename);
		if (ml) {
			ml->preload_map(true, nullptr);
			tags = map.get_tags();
		}
	}

	for (const Widelands::TribeBasicInfo& tribeinfo : settings.tribes) {
		if (tribeinfo.name == player.tribe) {
			const size_t nr_inits = tribeinfo.initializations.size();
			for (size_t i = 0; i < nr_inits; ++i) {
				bool matches_tags = true;
				for (const std::string& tag : tribeinfo.initializations[i].required_map_tags) {
					if (!tags.count(tag)) {
						matches_tags = false;
						break;
					}
				}
				if (matches_tags) {
					d->btnPlayerInit->add(tribeinfo.initializations[i].descname, i, nullptr,
					                      i == selection, tribeinfo.initializations[i].tooltip);
				}
			}
		}
	}
	if (!d->btnPlayerInit->has_selection()) {
		d->btnPlayerInit->select(0);
	}
}
