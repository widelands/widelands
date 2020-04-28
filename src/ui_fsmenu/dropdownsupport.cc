#include "dropdownsupport.h"

#include "ai/computer_player.h"
#include "base/i18n.h"
#include "graphic/playercolor.h"
#include "logic/game_settings.h"
#include "map_io/map_loader.h"

#define AI_NAME_PREFIX "ai" AI_NAME_SEPARATOR

TribeDropdownSupport::TribeDropdownSupport(UI::Panel* parent,
                                           const std::string& name,
                                           int32_t x,
                                           int32_t y,
                                           uint32_t w,
                                           uint32_t max_list_items,
                                           int button_dimension,
                                           const std::string& label,
                                           GameSettingsProvider* const settings,
                                           PlayerSlot id)
   : DropDownSupport<std::string>(parent,
                                  name,
                                  x,
                                  y,
                                  w,
                                  max_list_items,
                                  button_dimension,
                                  label,
                                  UI::DropdownType::kPictorial,
                                  UI::PanelStyle::kFsMenu,
                                  UI::ButtonStyle::kFsMenuSecondary,
                                  settings,
                                  id) {
}

/// Rebuild the tribes dropdown from the server settings. This will keep the host and client UIs
/// in sync.

void TribeDropdownSupport::rebuild() {
	const GameSettings& settings = settings_->settings();
	//	if (tribe_selection_locked_) {
	//		return;
	//	}
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
				   (boost::format(_("Shared in Player %u")) % static_cast<unsigned int>(i + 1)).str();
				dropdown_.add(player_name,
				              boost::lexical_cast<std::string>(static_cast<unsigned int>(i + 1)),
				              player_image, (i + 1) == player_setting.shared_in, player_name);
			}
		}
		dropdown_.set_enabled(dropdown_.size() > 1);
	} else {
		{
			i18n::Textdomain td("tribes");
			for (const Widelands::TribeBasicInfo& tribeinfo : Widelands::get_all_tribeinfos()) {
				dropdown_.add(_(tribeinfo.descname), tribeinfo.name, g_gr->images().get(tribeinfo.icon),
				              false, tribeinfo.tooltip);
			}
		}
		dropdown_.add(pgettext("tribe", "Random"), "random",
		              g_gr->images().get("images/ui_fsmenu/random.png"), false,
		              _("The tribe will be selected at random"));
		if (player_setting.random_tribe) {
			dropdown_.select("random");
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

TypeDropdownSupport::TypeDropdownSupport(UI::Panel* parent,
                                         const std::string& name,
                                         int32_t x,
                                         int32_t y,
                                         uint32_t w,
                                         uint32_t max_list_items,
                                         int button_dimension,
                                         const std::string& label,
                                         GameSettingsProvider* const settings,
                                         PlayerSlot id)
   : DropDownSupport<std::string>(parent,
                                  name,
                                  x,
                                  y,
                                  w,
                                  max_list_items,
                                  button_dimension,
                                  label,
                                  UI::DropdownType::kPictorial,
                                  UI::PanelStyle::kFsMenu,
                                  UI::ButtonStyle::kFsMenuSecondary,
                                  settings,
                                  id) {
}
void TypeDropdownSupport::rebuild() {
	/// Rebuild the type dropdown from the server settings. This will keep the host and client UIs in
	/// sync.

	//	if (type_selection_locked_) {
	//		return;
	//	}
	fill();
	dropdown_.set_enabled(settings_->can_change_player_state(id_));
	select_entry();
}
void TypeDropdownSupport::fill() {
	const GameSettings& settings = settings_->settings();
	dropdown_.clear();
	// AIs
	for (const auto* impl : ComputerPlayer::get_implementations()) {
		dropdown_.add(_(impl->descname), (boost::format(AI_NAME_PREFIX "%s") % impl->name).str(),
		              g_gr->images().get(impl->icon_filename), false, _(impl->descname));
	}
	/** TRANSLATORS: This is the name of an AI used in the game setup screens */
	dropdown_.add(_("Random AI"), AI_NAME_PREFIX "random",
	              g_gr->images().get("images/ai/ai_random.png"), false, _("Random AI"));

	// Slot state. Only add shared_in if there are viable slots
	if (settings.is_shared_usable(id_, settings.find_shared(id_))) {
		dropdown_.add(_("Shared in"), "shared_in",
		              g_gr->images().get("images/ui_fsmenu/shared_in.png"), false, _("Shared in"));
	}

	// Do not close a player in savegames or scenarios
	if (!settings.uncloseable(id_)) {
		dropdown_.add(
		   _("Closed"), "closed", g_gr->images().get("images/ui_basic/stop.png"), false, _("Closed"));
	}

	dropdown_.add(
	   _("Open"), "open", g_gr->images().get("images/ui_basic/continue.png"), false, _("Open"));
}

void TypeDropdownSupport::select_entry() {
	const GameSettings& settings = settings_->settings();
	// Now select the entry according to server settings
	const PlayerSettings& player_setting = settings.players[id_];
	if (player_setting.state == PlayerSettings::State::kHuman) {
		dropdown_.set_image(g_gr->images().get("images/wui/stats/genstats_nrworkers.png"));
		dropdown_.set_tooltip((boost::format(_("%1%: %2%")) % _("Type") % _("Human")).str());
	} else if (player_setting.state == PlayerSettings::State::kClosed) {
		dropdown_.select("closed");
	} else if (player_setting.state == PlayerSettings::State::kOpen) {
		dropdown_.select("open");
	} else if (player_setting.state == PlayerSettings::State::kShared) {
		dropdown_.select("shared_in");
	} else {
		if (player_setting.state == PlayerSettings::State::kComputer) {
			if (player_setting.ai.empty()) {
				dropdown_.set_errored(_("No AI"));
			} else {
				if (player_setting.random_ai) {
					dropdown_.select(AI_NAME_PREFIX "random");
				} else {
					const ComputerPlayer::Implementation* impl =
					   ComputerPlayer::get_implementation(player_setting.ai);
					dropdown_.select((boost::format(AI_NAME_PREFIX "%s") % impl->name).str());
				}
			}
		}
	}
}

InitDropdownSupport::InitDropdownSupport(UI::Panel* parent,
                                         const std::string& name,
                                         int32_t x,
                                         int32_t y,
                                         uint32_t w,
                                         uint32_t max_list_items,
                                         int button_dimension,
                                         const std::string& label,
                                         GameSettingsProvider* const settings,
                                         PlayerSlot id)
   : DropDownSupport<uintptr_t>(parent,
                                name,
                                x,
                                y,
                                w,
                                max_list_items,
                                button_dimension,
                                label,
                                UI::DropdownType::kTextualNarrow,
                                UI::PanelStyle::kFsMenu,
                                UI::ButtonStyle::kFsMenuSecondary,
                                settings,
                                id) {
}

/// Rebuild the init dropdown from the server settings. This will keep the host and client UIs in
/// sync.
void InitDropdownSupport::rebuild() {
	const GameSettings& settings = settings_->settings();
	//	if (init_selection_locked_) {
	//		return;
	//	}

	dropdown_.clear();
	const PlayerSettings& player_setting = settings.players[id_];
	if (settings.scenario) {
		dropdown_.set_label(_("Scenario"));
		dropdown_.set_tooltip(_("Start type is set via the scenario"));
	} else if (settings.savegame) {
		/** Translators: This is a game type */
		dropdown_.set_label(_("Saved Game"));
	} else {
		dropdown_.set_label("");
		i18n::Textdomain td("tribes");  // for translated initialisation
		const Widelands::TribeBasicInfo tribeinfo = Widelands::get_tribeinfo(player_setting.tribe);
		std::set<std::string> tags;
		if (!settings.mapfilename.empty()) {
			Widelands::Map map;
			std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(settings.mapfilename);
			if (ml) {
				ml->preload_map(true);
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
			if (matches_tags) {
				dropdown_.add(_(addme.descname), i, nullptr, i == player_setting.initialization_index,
				              _(addme.tooltip));
			}
		}
	}

	dropdown_.set_visible(true);
	dropdown_.set_enabled(settings_->can_change_player_init(id_));
}
TeamDropdown::TeamDropdown(UI::Panel* parent,
                           const std::string& name,
                           int32_t x,
                           int32_t y,
                           uint32_t w,
                           uint32_t max_list_items,
                           int button_dimension,
                           const std::string& label,
                           GameSettingsProvider* const settings,
                           PlayerSlot id)
   : DropDownSupport<uintptr_t>(parent,
                                name,
                                x,
                                y,
                                w,
                                max_list_items,
                                button_dimension,
                                label,
                                UI::DropdownType::kTextualNarrow,
                                UI::PanelStyle::kFsMenu,
                                UI::ButtonStyle::kFsMenuSecondary,
                                settings,
                                id) {
}
/// Rebuild the team dropdown from the server settings. This will keep the host and client UIs in
/// sync.

void TeamDropdown::rebuild() {
	const GameSettings& settings = settings_->settings();
	//	if (team_selection_locked_) {
	//		return;
	//	}
	const PlayerSettings& player_setting = settings.players[id_];
	if (player_setting.state == PlayerSettings::State::kShared) {
		dropdown_.set_visible(false);
		dropdown_.set_enabled(false);
		return;
	}

	dropdown_.clear();
	dropdown_.add(_("No Team"), 0, g_gr->images().get("images/players/no_team.png"));
#ifndef NDEBUG
	const size_t no_of_team_colors = sizeof(kTeamColors) / sizeof(kTeamColors[0]);
#endif
	for (Widelands::TeamNumber t = 1; t <= settings.players.size() / 2; ++t) {
		assert(t < no_of_team_colors);
		dropdown_.add((boost::format(_("Team %d")) % static_cast<unsigned int>(t)).str(), t,
		              playercolor_image(kTeamColors[t], "images/players/team.png"));
	}
	dropdown_.select(player_setting.team);
	dropdown_.set_visible(true);
	dropdown_.set_enabled(settings_->can_change_player_team(id_));
}
