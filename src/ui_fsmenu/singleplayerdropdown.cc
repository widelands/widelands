/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "ui_fsmenu/singleplayerdropdown.h"

#include <memory>

#include "ai/computer_player.h"
#include "base/i18n.h"
#include "base/string.h"
#include "graphic/image_cache.h"
#include "graphic/playercolor.h"
#include "map_io/map_loader.h"
#include "ui_fsmenu/launch_game.h"

namespace FsMenu {

constexpr const char* const kClosed = "closed";
constexpr const char* const kHuman_player = "human_player";

SinglePlayerTribeDropdown::SinglePlayerTribeDropdown(UI::Panel* parent,
                                                     LaunchGame& lg,
                                                     const std::string& name,
                                                     int32_t x,
                                                     int32_t y,
                                                     uint32_t w,
                                                     int button_dimension,
                                                     GameSettingsProvider* const settings,
                                                     PlayerSlot id)
   : SinglePlayerDropdown<std::string>(parent,
                                       lg,
                                       name,
                                       x,
                                       y,
                                       w,
                                       16,
                                       button_dimension,
                                       _("Tribe"),
                                       UI::DropdownType::kPictorial,
                                       UI::PanelStyle::kFsMenu,
                                       UI::ButtonStyle::kFsMenuSecondary,
                                       settings,
                                       id) {
}

void SinglePlayerTribeDropdown::rebuild() {

	if (selection_locked_) {
		return;
	}
	const GameSettings& settings = settings_->settings();
	const PlayerSettings& player_setting = settings.players[id_];
	dropdown_.clear();
	if (player_setting.state == PlayerSettings::State::kShared) {
		for (size_t i = 0; i < settings.players.size(); ++i) {
			if (i != id_) {
				// Do not add players that are also shared_in or closed.
				const PlayerSettings& other_setting = settings.players[i];
				if (!PlayerSettings::can_be_shared(other_setting.state)) {
					continue;
				}

				const Image* player_image =
				   playercolor_image(i, "images/players/player_position_menu.png");
				assert(player_image);
				const std::string player_name =
				   /** TRANSLATORS: This is an option in multiplayer setup for sharing
				         another player's starting position. */
				   format(_("Shared in Player %u"), static_cast<unsigned int>(i + 1));
				dropdown_.add(player_name, as_string(static_cast<unsigned int>(i + 1)), player_image,
				              (i + 1) == player_setting.shared_in, player_name);
			}
		}
		dropdown_.set_enabled(dropdown_.size() > 1);
	} else {
		for (const Widelands::TribeBasicInfo& tribeinfo : settings.tribes) {
			if (player_setting.state != PlayerSettings::State::kComputer || tribeinfo.suited_for_ai) {
				dropdown_.add(tribeinfo.descname, tribeinfo.name, g_image_cache->get(tribeinfo.icon),
				              false, tribeinfo.tooltip);
			}
		}
		dropdown_.add(pgettext("tribe", "Random"), kRandom,
		              g_image_cache->get("images/ui_fsmenu/random.png"), false,
		              _("The tribe will be selected at random"));
		if (player_setting.random_tribe) {
			dropdown_.select(kRandom);
		} else {
			dropdown_.select(player_setting.tribe);
		}
	}
	const bool has_access = true;  // has_tribe_access();
	if (dropdown_.is_enabled() != has_access) {
		dropdown_.set_enabled(has_access && dropdown_.size() > 1);
	}
	if (player_setting.state == PlayerSettings::State::kClosed ||
	    player_setting.state == PlayerSettings::State::kOpen) {
		return;
	}
	if (!dropdown_.is_visible()) {
		dropdown_.set_visible(true);
	}
}

void SinglePlayerTribeDropdown::selection_action() {
	const PlayerSettings& player_settings = settings_->settings().players[id_];
	dropdown_.set_disable_style(player_settings.state == PlayerSettings::State::kShared ?
                                  UI::ButtonDisableStyle::kPermpressed :
                                  UI::ButtonDisableStyle::kFlat);
	if (dropdown_.has_selection()) {
		if (player_settings.state == PlayerSettings::State::kShared) {
			settings_->set_player_shared(id_, stoul(dropdown_.get_selected()));
		} else {
			const std::string& selected = dropdown_.get_selected();
			settings_->set_player_tribe(id_, selected, selected == kRandom);
		}
		Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kPlayer));
	}
}

SinglePlayerPlayerTypeDropdown::SinglePlayerPlayerTypeDropdown(UI::Panel* parent,
                                                               LaunchGame& lg,
                                                               const std::string& name,
                                                               int32_t x,
                                                               int32_t y,
                                                               uint32_t w,
                                                               int button_dimension,
                                                               GameSettingsProvider* const settings,
                                                               PlayerSlot id)
   : SinglePlayerDropdown<std::string>(parent,
                                       lg,
                                       name,
                                       x,
                                       y,
                                       w,
                                       16,
                                       button_dimension,
                                       _("Type"),
                                       UI::DropdownType::kPictorial,
                                       UI::PanelStyle::kFsMenu,
                                       UI::ButtonStyle::kFsMenuSecondary,
                                       settings,
                                       id) {
}
void SinglePlayerPlayerTypeDropdown::rebuild() {
	if (selection_locked_) {
		return;
	}
	fill();
	dropdown_.set_enabled(settings_->can_change_player_state(id_));
	dropdown_.set_list_visibility(false);
	select_entry();
}
void SinglePlayerPlayerTypeDropdown::fill() {
	const GameSettings& settings = settings_->settings();
	dropdown_.clear();
	// AIs
	if (settings.get_tribeinfo(settings.players[id_].tribe).suited_for_ai) {
		for (const auto* impl : AI::ComputerPlayer::get_implementations()) {
			dropdown_.add(_(impl->descname), format("%s%s", kAiNamePrefix, impl->name),
			              g_image_cache->get(impl->icon_filename), false, _(impl->descname));
		}
		/** TRANSLATORS: This is the name of an AI used in the game setup screens */
		dropdown_.add(_("Random AI"), kRandomAiName, g_image_cache->get("images/ai/ai_random.png"),
		              false, _("Random AI"));
	}
	dropdown_.add(
	   /** TRANSLATORS: This is the "name" of the single player */
	   _("You"), kHuman_player, g_image_cache->get("images/wui/stats/genstats_nrworkers.png"));

	// Do not close a player in savegames or scenarios
	if (!settings.uncloseable(id_)) {
		dropdown_.add(
		   _("Closed"), kClosed, g_image_cache->get("images/ui_basic/stop.png"), false, _("Closed"));
	}
}

void SinglePlayerPlayerTypeDropdown::select_entry() {
	const GameSettings& settings = settings_->settings();
	const PlayerSettings& player_setting = settings.players[id_];
	if (player_setting.state == PlayerSettings::State::kHuman) {
		dropdown_.set_image(g_image_cache->get("images/wui/stats/genstats_nrworkers.png"));
		dropdown_.set_tooltip(format(_("%1%: %2%"), _("Type"), _("Human")));
		dropdown_.set_enabled(false);
	} else if (player_setting.state == PlayerSettings::State::kClosed) {
		dropdown_.select(kClosed);
	} else {
		if (player_setting.state == PlayerSettings::State::kComputer) {
			if (player_setting.random_ai) {
				dropdown_.select(kRandomAiName);
			} else if (player_setting.ai.empty()) {
				dropdown_.set_errored(_("No AI"));
			} else {
				const AI::ComputerPlayer::Implementation* impl =
				   AI::ComputerPlayer::get_implementation(player_setting.ai);
				dropdown_.select(format("%s%s", kAiNamePrefix, impl->name));
			}
		}
	}
}

void SinglePlayerPlayerTypeDropdown::selection_action() {
	if (!settings_->can_change_player_state(id_)) {
		return;
	}
	if (dropdown_.has_selection()) {
		const std::string& selected = dropdown_.get_selected();
		PlayerSettings::State state = PlayerSettings::State::kComputer;
		if (selected == kClosed) {
			state = PlayerSettings::State::kClosed;
		} else if (selected == kHuman_player) {
			settings_->set_player_number(id_);
			state = PlayerSettings::State::kHuman;
			dropdown_.set_enabled(false);
		} else {
			if (selected == kRandomAiName) {
				settings_->set_player_ai(id_, "", true);
			} else {
				if (starts_with(selected, kAiNamePrefix)) {
					std::vector<std::string> parts;
					split(parts, selected, {kAiNameSeparator});
					assert(parts.size() == 2);
					settings_->set_player_ai(id_, parts[1], false);
				} else {
					throw wexception("Unknown player state: %s\n", selected.c_str());
				}
			}
		}

		settings_->set_player_state(id_, state);
		Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kPlayer));
	}
}

SinglePlayerStartTypeDropdown::SinglePlayerStartTypeDropdown(UI::Panel* parent,
                                                             LaunchGame& lg,
                                                             const std::string& name,
                                                             int32_t x,
                                                             int32_t y,
                                                             uint32_t w,
                                                             int button_dimension,
                                                             GameSettingsProvider* const settings,
                                                             PlayerSlot id)
   : SinglePlayerDropdown<uintptr_t>(parent,
                                     lg,
                                     name,
                                     x,
                                     y,
                                     w,
                                     16,
                                     button_dimension,
                                     "",
                                     UI::DropdownType::kTextualNarrow,
                                     UI::PanelStyle::kFsMenu,
                                     UI::ButtonStyle::kFsMenuSecondary,
                                     settings,
                                     id) {
}

void SinglePlayerStartTypeDropdown::rebuild() {

	if (selection_locked_) {
		return;
	}
	const GameSettings& settings = settings_->settings();
	dropdown_.clear();

	if (settings.scenario) {
		dropdown_.set_label(_("Scenario"));
		dropdown_.set_tooltip(_("Start type is set via the scenario"));
	} else if (settings.savegame) {
		/** Translators: This is a game type */
		dropdown_.set_label(_("Saved Game"));
	} else {
		dropdown_.set_label("");
		fill();
	}

	dropdown_.set_visible(true);
	dropdown_.set_enabled(settings_->can_change_player_init(id_));
}

void SinglePlayerStartTypeDropdown::fill() {
	const GameSettings& settings = settings_->settings();
	const PlayerSettings& player_setting = settings.players[id_];
	i18n::Textdomain td("tribes");  // for translated initialisation
	Widelands::AllTribes all_tribes = Widelands::get_all_tribeinfos(nullptr);
	const Widelands::TribeBasicInfo tribeinfo =
	   Widelands::get_tribeinfo(player_setting.tribe, all_tribes);
	std::set<std::string> tags;
	if (!settings.mapfilename.empty()) {
		Widelands::Map map;
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(settings.mapfilename);
		if (ml) {
			ml->preload_map(true, nullptr);
			tags = map.get_tags();
		}
	}

	for (size_t i = 0; i < tribeinfo.initializations.size(); ++i) {
		const Widelands::TribeBasicInfo::Initialization& addme = tribeinfo.initializations[i];
		bool matches_tags = true;
		for (const std::string& tag : addme.required_map_tags) {
			if (!tags.count(tag)) {
				matches_tags = false;
				break;
			}
		}
		if (matches_tags &&
		    !addme.incompatible_win_conditions.count(settings_->get_win_condition_script())) {
			dropdown_.add(_(addme.descname), i, nullptr, i == player_setting.initialization_index,
			              _(addme.tooltip));
		}
	}
}

void SinglePlayerStartTypeDropdown::selection_action() {
	if (!settings_->can_change_player_init(id_)) {
		return;
	}
	if (dropdown_.has_selection()) {
		settings_->set_player_init(id_, dropdown_.get_selected());
		launch_game_.update_custom_starting_positions();
	}
}
SinglePlayerTeamDropdown::SinglePlayerTeamDropdown(UI::Panel* parent,
                                                   LaunchGame& lg,
                                                   const std::string& name,
                                                   int32_t x,
                                                   int32_t y,
                                                   uint32_t w,
                                                   int button_dimension,
                                                   GameSettingsProvider* const settings,
                                                   PlayerSlot id)
   : SinglePlayerDropdown<uintptr_t>(parent,
                                     lg,
                                     name,
                                     x,
                                     y,
                                     w,
                                     16,
                                     button_dimension,
                                     _("Team"),
                                     UI::DropdownType::kPictorial,
                                     UI::PanelStyle::kFsMenu,
                                     UI::ButtonStyle::kFsMenuSecondary,
                                     settings,
                                     id) {
}

void SinglePlayerTeamDropdown::rebuild() {
	if (selection_locked_) {
		return;
	}
	const GameSettings& settings = settings_->settings();
	const PlayerSettings& player_setting = settings.players[id_];
	if (player_setting.state == PlayerSettings::State::kShared) {
		dropdown_.set_visible(false);
		dropdown_.set_enabled(false);
		return;
	}

	dropdown_.clear();
	dropdown_.add(_("No Team"), 0, g_image_cache->get("images/players/no_team.png"));
#ifndef NDEBUG
	const size_t no_of_team_colors = sizeof(kTeamColors) / sizeof(kTeamColors[0]);
#endif
	for (Widelands::TeamNumber t = 1; t <= settings.players.size() / 2; ++t) {
		assert(t < no_of_team_colors);
		dropdown_.add(format(_("Team %d"), static_cast<unsigned int>(t)), t,
		              playercolor_image(kTeamColors[t], "images/players/team.png"));
	}
	dropdown_.select(player_setting.team);
	dropdown_.set_visible(true);
	dropdown_.set_enabled(settings_->can_change_player_team(id_));
}

void SinglePlayerTeamDropdown::selection_action() {
	if (dropdown_.has_selection()) {
		settings_->set_player_team(id_, dropdown_.get_selected());
	}
}
}  // namespace FsMenu
