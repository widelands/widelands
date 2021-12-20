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

#include "ui_fsmenu/multiplayersetupgroup.h"

#include <memory>

#include "ai/computer_player.h"
#include "base/i18n.h"
#include "base/string.h"
#include "base/wexception.h"
#include "graphic/image_cache.h"
#include "graphic/playercolor.h"
#include "logic/game.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "ui_basic/button.h"
#include "ui_basic/color_chooser.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/mouse_constants.h"

constexpr int kPadding = 4;

/// Holds the info and dropdown menu for a connected client
struct MultiPlayerClientGroup : public UI::Box {
	MultiPlayerClientGroup(UI::Panel* const grand_parent,
	                       UI::Panel* const parent,
	                       int32_t const,
	                       int32_t const h,
	                       PlayerSlot id,
	                       GameSettingsProvider* const settings)
	   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, 0, 0, kPadding),
	     slot_dropdown_(this,
	                    format("dropdown_slot%d", static_cast<unsigned int>(id)),
	                    0,
	                    0,
	                    0,
	                    16,
	                    h,
	                    _("Role"),
	                    UI::DropdownType::kPictorial,
	                    UI::PanelStyle::kFsMenu,
	                    UI::ButtonStyle::kFsMenuSecondary),
	     // Name needs to be initialized after the dropdown, otherwise the layout function will
	     // crash.
	     name(this, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuLabel, 0, 0, 0, 0),
	     settings_(settings),
	     id_(id),
	     slot_selection_locked_(false),
	     menu_parent_(grand_parent) {
		add(&slot_dropdown_);
		add(&name, UI::Box::Resizing::kAlign, UI::Align::kCenter);

		slot_dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		slot_dropdown_.selected.connect([this]() { set_slot(); });

		update();
	}

	void force_new_dimensions(uint32_t standard_element_height) {
		slot_dropdown_.set_desired_size(standard_element_height, standard_element_height);
	}

	/// This will update the client's player slot with the value currently selected in the slot
	/// dropdown.
	void set_slot() {
		const GameSettings& settings = settings_->settings();
		if (id_ != settings.usernum) {
			return;
		}
		slot_selection_locked_ = true;
		if (slot_dropdown_.has_selection()) {
			const uint8_t new_slot = slot_dropdown_.get_selected();
			if (new_slot != settings.users.at(id_).position) {
				const std::string& slotname =
				   new_slot == UserSettings::none() ? "" : settings.players.at(new_slot).name;
				const std::string& username = settings.users.at(id_).name;

				if (new_slot != UserSettings::none() && settings.savegame && !slotname.empty() &&
				    slotname != username) {
					slot_dropdown_.toggle();
					UI::WLMessageBox m(
					   menu_parent_, UI::WindowStyle::kFsMenu, _("Divergent Player Name"),
					   format(
					      _("The player name of the selected slot (%1%) does not match your own (%2%). "
					        "Are you sure that you want to occupy this slot?"),
					      slotname, username),
					   UI::WLMessageBox::MBoxType::kOkCancel, UI::Align::kLeft);

					if (m.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
						settings_->set_player_number(new_slot);
					} else {
						slot_dropdown_.select(settings.users.at(id_).position);
					}
				} else {
					settings_->set_player_number(new_slot);
				}
			}
		}
		slot_selection_locked_ = false;
	}

	/// Rebuild the slot dropdown from the server settings. This will keep the host and client UIs in
	/// sync.
	void rebuild_slot_dropdown(const GameSettings& settings) {
		if (slot_selection_locked_) {
			return;
		}
		const UserSettings& user_setting = settings.users.at(id_);

		slot_dropdown_.clear();
		for (PlayerSlot slot = 0; slot < settings.players.size(); ++slot) {
			if (settings.players.at(slot).state == PlayerSettings::State::kHuman ||
			    settings.players.at(slot).state == PlayerSettings::State::kOpen) {
				slot_dropdown_.add(format(_("Player %u"), static_cast<unsigned int>(slot + 1)), slot,
				                   playercolor_image(settings.players[slot].color,
				                                     "images/players/genstats_player.png"),
				                   slot == user_setting.position);
			}
		}
		slot_dropdown_.add(_("Spectator"), UserSettings::none(),
		                   g_image_cache->get("images/wui/fieldaction/menu_tab_watch.png"),
		                   user_setting.position == UserSettings::none());
		slot_dropdown_.set_visible(true);
		slot_dropdown_.set_enabled(id_ == settings.usernum);
	}

	/// Take care of visibility and current values
	void update() {
		const GameSettings& settings = settings_->settings();
		const UserSettings& user_setting = settings.users.at(id_);
		set_visible(user_setting.position != UserSettings::not_connected());

		name.set_text(user_setting.name);
		rebuild_slot_dropdown(settings);

		initialization_complete();
	}

	UI::Dropdown<uintptr_t> slot_dropdown_;  /// Select the player slot.
	UI::Textarea name;                       /// Client nick name
	GameSettingsProvider* const settings_;
	uint8_t const id_;            /// User number
	bool slot_selection_locked_;  // Ensure that dropdowns will close on selection.
	UI::Panel* menu_parent_;
};

/// Holds the dropdown menus for a player slot
struct MultiPlayerPlayerGroup : public UI::Box {
	MultiPlayerPlayerGroup(UI::Panel* const parent,
	                       int32_t const,
	                       int32_t const h,
	                       PlayerSlot id,
	                       GameSettingsProvider* const settings,
	                       NetworkPlayerSettingsBackend* const npsb)
	   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
	     settings_(settings),
	     n(npsb),
	     id_(id),
	     player_(this,
	             "player",
	             0,
	             0,
	             h,
	             h,
	             UI::ButtonStyle::kFsMenuSecondary,
	             playercolor_image(settings_->settings().players[id].color,
	                               "images/players/player_position_menu.png"),
	             format(_("Player %u"), static_cast<unsigned int>(id_ + 1))),
	     type_dropdown_(this,
	                    format("dropdown_type%d", static_cast<unsigned int>(id)),
	                    0,
	                    0,
	                    h,
	                    16,
	                    h,
	                    _("Type"),
	                    UI::DropdownType::kPictorial,
	                    UI::PanelStyle::kFsMenu,
	                    UI::ButtonStyle::kFsMenuSecondary),
	     tribes_dropdown_(this,
	                      format("dropdown_tribes%d", static_cast<unsigned int>(id)),
	                      0,
	                      0,
	                      h,
	                      16,
	                      h,
	                      _("Tribe"),
	                      UI::DropdownType::kPictorial,
	                      UI::PanelStyle::kFsMenu,
	                      UI::ButtonStyle::kFsMenuSecondary),
	     init_dropdown_(this,
	                    format("dropdown_init%d", static_cast<unsigned int>(id)),
	                    0,
	                    0,
	                    h,
	                    16,
	                    h,
	                    "",
	                    UI::DropdownType::kTextualNarrow,
	                    UI::PanelStyle::kFsMenu,
	                    UI::ButtonStyle::kFsMenuSecondary),
	     team_dropdown_(this,
	                    format("dropdown_team%d", static_cast<unsigned int>(id)),
	                    0,
	                    0,
	                    h,
	                    16,
	                    h,
	                    _("Team"),
	                    UI::DropdownType::kPictorial,
	                    UI::PanelStyle::kFsMenu,
	                    UI::ButtonStyle::kFsMenuSecondary),
	     last_state_(PlayerSettings::State::kClosed),
	     type_selection_locked_(false),
	     tribe_selection_locked_(false),
	     init_selection_locked_(false),
	     team_selection_locked_(false) {

		type_dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		tribes_dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		init_dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		team_dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		player_.set_disable_style(UI::ButtonDisableStyle::kFlat);

		type_dropdown_.selected.connect([this]() { set_type(); });
		tribes_dropdown_.selected.connect([this]() { set_tribe_or_shared_in(); });
		init_dropdown_.selected.connect([this]() { set_init(); });
		team_dropdown_.selected.connect([this]() { set_team(); });
		player_.sigclicked.connect([this]() { set_color(); });

		add(&player_);
		add(&type_dropdown_);
		add(&tribes_dropdown_);
		add(&init_dropdown_, UI::Box::Resizing::kExpandBoth);
		add(&team_dropdown_);

		// Init dropdowns
		update();
	}

	/// This will update the game settings for the type with the value
	/// currently selected in the type dropdown.
	void set_type() {
		if (!settings_->can_change_player_state(id_)) {
			return;
		}
		type_selection_locked_ = true;
		if (type_dropdown_.has_selection()) {
			const std::string& selected = type_dropdown_.get_selected();
			PlayerSettings::State state = PlayerSettings::State::kComputer;
			if (selected == "closed") {
				state = PlayerSettings::State::kClosed;
			} else if (selected == "open") {
				state = PlayerSettings::State::kOpen;
			} else if (selected == "shared_in") {
				state = PlayerSettings::State::kShared;
			} else {
				if (selected == kRandomAiName) {
					n->set_player_ai(id_, "", true);
				} else {
					if (starts_with(selected, kAiNamePrefix)) {
						std::vector<std::string> parts;
						split(parts, selected, {kAiNameSeparator});
						assert(parts.size() == 2);
						n->set_player_ai(id_, parts[1], false);
					} else {
						throw wexception("Unknown player state: %s\n", selected.c_str());
					}
				}
			}
			n->set_player_state(id_, state);

			const GameSettings& settings = settings_->settings();
			const std::string& tribe = settings.players[id_].tribe;
			if (state == PlayerSettings::State::kComputer &&
			    (tribe.empty() || !settings.get_tribeinfo(tribe).suited_for_ai)) {
				for (const Widelands::TribeBasicInfo& t : settings.tribes) {
					if (t.suited_for_ai) {
						n->set_player_tribe(id_, t.name);
						break;
					}
				}
			}
		}
		type_selection_locked_ = false;
		update();
	}

	/// Rebuild the type dropdown from the server settings. This will keep the host and client UIs in
	/// sync.
	void rebuild_type_dropdown(const GameSettings& settings) {
		if (type_selection_locked_) {
			return;
		}
		const PlayerSettings& player_setting = settings.players[id_];
		type_dropdown_.clear();
		// AIs
		bool can_change_hidden_tribe = !tribes_dropdown_.is_visible() && !settings.savegame;
		if (settings.allows_ais(id_) &&
		    (player_setting.tribe.empty() ||
		     settings.get_tribeinfo(player_setting.tribe).suited_for_ai || can_change_hidden_tribe)) {
			for (const auto* impl : AI::ComputerPlayer::get_implementations()) {
				type_dropdown_.add(_(impl->descname), format("%s%s", kAiNamePrefix, impl->name),
				                   g_image_cache->get(impl->icon_filename), false, _(impl->descname));
			}
			/** TRANSLATORS: This is the name of an AI used in the game setup screens */
			type_dropdown_.add(_("Random AI"), kRandomAiName,
			                   g_image_cache->get("images/ai/ai_random.png"), false, _("Random AI"));
		}

		// Slot state. Only add shared_in if there are viable slots
		if (settings.is_shared_usable(id_, settings.find_shared(id_))) {
			type_dropdown_.add(_("Shared in"), "shared_in",
			                   g_image_cache->get("images/ui_fsmenu/shared_in.png"), false,
			                   _("Shared in"));
		}

		// Do not close a player in savegames or scenarios, but add an visual entry for the client
		if (!settings.uncloseable(id_) || !settings_->can_change_player_state(id_)) {
			type_dropdown_.add(_("Closed"), "closed", g_image_cache->get("images/ui_basic/stop.png"),
			                   false, _("Closed"));
		}

		type_dropdown_.add(
		   _("Open"), "open", g_image_cache->get("images/ui_basic/continue.png"), false, _("Open"));

		type_dropdown_.set_enabled(settings_->can_change_player_state(id_));

		// Now select the entry according to server settings
		if (player_setting.state == PlayerSettings::State::kHuman) {
			type_dropdown_.set_image(g_image_cache->get("images/wui/stats/genstats_nrworkers.png"));
			type_dropdown_.set_tooltip(format(_("%1%: %2%"), _("Type"), _("Human")));
		} else if (player_setting.state == PlayerSettings::State::kClosed) {
			type_dropdown_.select("closed");
		} else if (player_setting.state == PlayerSettings::State::kOpen) {
			type_dropdown_.select("open");
		} else if (player_setting.state == PlayerSettings::State::kShared) {
			type_dropdown_.select("shared_in");
		} else {
			if (player_setting.state == PlayerSettings::State::kComputer) {
				if (player_setting.ai.empty()) {
					type_dropdown_.set_errored(_("No AI"));
				} else {
					if (player_setting.random_ai) {
						type_dropdown_.select(kRandomAiName);
					} else {
						const AI::ComputerPlayer::Implementation* impl =
						   AI::ComputerPlayer::get_implementation(player_setting.ai);
						type_dropdown_.select(format("%s%s", kAiNamePrefix, impl->name));
					}
				}
			}
		}
	}

	/// Whether the client who is running the UI is allowed to change the tribe for this player slot.
	bool has_tribe_access() const {
		return settings_->settings().players[id_].state == PlayerSettings::State::kShared ?
                settings_->can_change_player_init(id_) :
                settings_->can_change_player_tribe(id_);
	}

	/// This will update the game settings for the tribe or shared_in with the value
	/// currently selected in the tribes dropdown.
	void set_tribe_or_shared_in() {
		if (!has_tribe_access()) {
			return;
		}
		const PlayerSettings& player_settings = settings_->settings().players[id_];
		tribe_selection_locked_ = true;
		tribes_dropdown_.set_disable_style(player_settings.state == PlayerSettings::State::kShared ?
                                            UI::ButtonDisableStyle::kPermpressed :
                                            UI::ButtonDisableStyle::kFlat);
		if (tribes_dropdown_.has_selection()) {
			if (player_settings.state == PlayerSettings::State::kShared) {
				n->set_player_shared(id_, stoul(tribes_dropdown_.get_selected()));
			} else {
				n->set_player_tribe(id_, tribes_dropdown_.get_selected());
			}
		}
		tribe_selection_locked_ = false;
	}

	/// Rebuild the tribes dropdown from the server settings. This will keep the host and client UIs
	/// in sync.
	void rebuild_tribes_dropdown(const GameSettings& settings) {
		assert(!settings.tribes.empty());
		if (tribe_selection_locked_) {
			return;
		}
		const PlayerSettings& player_setting = settings.players[id_];
		tribes_dropdown_.clear();
		if (player_setting.state == PlayerSettings::State::kShared) {
			for (size_t i = 0; i < settings.players.size(); ++i) {
				if (i != id_) {
					// Do not add players that are also shared_in or closed.
					const PlayerSettings& other_setting = settings.players[i];
					if (!PlayerSettings::can_be_shared(other_setting.state)) {
						continue;
					}

					const Image* player_image = playercolor_image(
					   settings.players[i].color, "images/players/player_position_menu.png");
					assert(player_image);
					const std::string player_name =
					   /** TRANSLATORS: This is an option in multiplayer setup for sharing
					      another player's starting position. */
					   format(_("Shared in Player %u"), static_cast<unsigned int>(i + 1));
					tribes_dropdown_.add(player_name, as_string(static_cast<unsigned int>(i + 1)),
					                     player_image, (i + 1) == player_setting.shared_in, player_name);
				}
			}
			tribes_dropdown_.set_enabled(tribes_dropdown_.size() > 1);
		} else {
			for (const Widelands::TribeBasicInfo& tribeinfo : settings.tribes) {
				if (player_setting.state != PlayerSettings::State::kComputer ||
				    tribeinfo.suited_for_ai) {
					tribes_dropdown_.add(tribeinfo.descname, tribeinfo.name,
					                     g_image_cache->get(tribeinfo.icon), false, tribeinfo.tooltip);
				}
			}

			tribes_dropdown_.add(pgettext("tribe", "Random"), kRandom,
			                     g_image_cache->get("images/ui_fsmenu/random.png"), false,
			                     _("The tribe will be selected at random"));
			if (player_setting.random_tribe) {
				tribes_dropdown_.select(kRandom);
			} else {
				tribes_dropdown_.select(player_setting.tribe);
			}
		}
		const bool has_access = has_tribe_access();
		if (tribes_dropdown_.is_enabled() != has_access) {
			tribes_dropdown_.set_enabled(has_access && tribes_dropdown_.size() > 1);
		}
		if (player_setting.state == PlayerSettings::State::kClosed ||
		    player_setting.state == PlayerSettings::State::kOpen) {
			return;
		}
		if (!tribes_dropdown_.is_visible()) {
			tribes_dropdown_.set_visible(true);
		}
	}

	/// This will update the game settings for the initialization with the value
	/// currently selected in the initialization dropdown.
	void set_init() {
		if (!settings_->can_change_player_init(id_)) {
			return;
		}
		init_selection_locked_ = true;
		if (init_dropdown_.has_selection()) {
			n->set_player_init(id_, init_dropdown_.get_selected());
		}
		init_selection_locked_ = false;
	}

	/// Rebuild the init dropdown from the server settings. This will keep the host and client UIs in
	/// sync.
	void rebuild_init_dropdown(const GameSettings& settings) {
		if (init_selection_locked_) {
			return;
		}

		init_dropdown_.clear();
		const PlayerSettings& player_setting = settings.players[id_];
		if (settings.scenario) {
			init_dropdown_.set_label(_("Scenario"));
			init_dropdown_.set_tooltip(_("Start type is set via the scenario"));
		} else if (settings.savegame) {
			/** Translators: This is a game type */
			init_dropdown_.set_label(_("Saved Game"));
		} else {
			init_dropdown_.set_label("");
			const Widelands::TribeBasicInfo tribeinfo = settings.get_tribeinfo(player_setting.tribe);
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
					init_dropdown_.add(_(addme.descname), i, nullptr,
					                   i == player_setting.initialization_index, _(addme.tooltip));
				}
			}
		}

		init_dropdown_.set_visible(true);
		init_dropdown_.set_enabled(settings_->can_change_player_init(id_));
	}

	/// This will update the team settings with the value currently selected in the teams dropdown.
	void set_team() {
		team_selection_locked_ = true;
		if (team_dropdown_.has_selection()) {
			n->set_player_team(id_, team_dropdown_.get_selected());
		}
		team_selection_locked_ = false;
	}

	void set_color() {
		Panel* p = this;
		while (p->get_parent()) {
			p = p->get_parent();
		}
		UI::ColorChooser c(p, UI::WindowStyle::kFsMenu, settings_->settings().players[id_].color,
		                   &kPlayerColors[id_]);
		if (c.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
			n->set_player_color(id_, c.get_color());
		}
	}

	/// Rebuild the team dropdown from the server settings. This will keep the host and client UIs in
	/// sync.
	void rebuild_team_dropdown(const GameSettings& settings) {
		if (team_selection_locked_) {
			return;
		}
		const PlayerSettings& player_setting = settings.players[id_];
		if (player_setting.state == PlayerSettings::State::kShared) {
			team_dropdown_.set_visible(false);
			team_dropdown_.set_enabled(false);
			return;
		}

		team_dropdown_.clear();
		team_dropdown_.add(_("No Team"), 0, g_image_cache->get("images/players/no_team.png"));
#ifndef NDEBUG
		const size_t no_of_team_colors = sizeof(kTeamColors) / sizeof(kTeamColors[0]);
#endif
		for (Widelands::TeamNumber t = 1; t <= settings.players.size() / 2; ++t) {
			assert(t < no_of_team_colors);
			team_dropdown_.add(format(_("Team %d"), static_cast<unsigned int>(t)), t,
			                   playercolor_image(kTeamColors[t], "images/players/team.png"));
		}
		team_dropdown_.select(player_setting.team);
		team_dropdown_.set_visible(true);
		team_dropdown_.set_enabled(settings_->can_change_player_team(id_));
	}

	/// Refresh all user interfaces
	void update() {
		const GameSettings& settings = settings_->settings();
		if (id_ >= settings.players.size()) {
			set_visible(false);
			return;
		}

		const PlayerSettings& player_setting = settings.players[id_];
		player_.set_tooltip(player_setting.name);
		player_.set_pic(
		   playercolor_image(player_setting.color, "images/players/player_position_menu.png"));
		player_.set_enabled(settings_->can_change_player_color(id_));

		rebuild_type_dropdown(settings);
		set_visible(true);

		if (player_setting.state == PlayerSettings::State::kClosed ||
		    player_setting.state == PlayerSettings::State::kOpen) {
			team_dropdown_.set_visible(false);
			team_dropdown_.set_enabled(false);
			tribes_dropdown_.set_visible(false);
			tribes_dropdown_.set_enabled(false);
			init_dropdown_.set_visible(false);
			init_dropdown_.set_enabled(false);
		} else {  // kHuman, kShared, kComputer
			rebuild_tribes_dropdown(settings);
			rebuild_init_dropdown(settings);
			rebuild_team_dropdown(settings);
		}

		// Trigger update for the other players for shared_in mode when slots open and close
		if (last_state_ != player_setting.state) {
			last_state_ = player_setting.state;
			for (PlayerSlot slot = 0; slot < settings_->settings().players.size(); ++slot) {
				if (slot != id_) {
					n->set_player_state(slot, settings.players[slot].state);
				}
			}
		}

		initialization_complete();
	}

	void force_new_dimensions(uint32_t height) {
		player_.set_desired_size(height, height);
		type_dropdown_.set_desired_size(height, height);
		tribes_dropdown_.set_desired_size(height, height);
		team_dropdown_.set_desired_size(height, height);
		init_dropdown_.set_desired_size(height, height);
	}

	GameSettingsProvider* const settings_;
	NetworkPlayerSettingsBackend* const n;
	PlayerSlot const id_;

	UI::Button player_;
	UI::Dropdown<std::string>
	   type_dropdown_;  /// Select who owns the slot (human, AI, open, closed, shared-in).
	UI::Dropdown<std::string> tribes_dropdown_;  /// Select the tribe or shared_in player.
	UI::Dropdown<uintptr_t>
	   init_dropdown_;  /// Select the initialization (Headquarters, Fortified Village etc.)
	UI::Dropdown<uintptr_t> team_dropdown_;  /// Select the team number
	PlayerSettings::State
	   last_state_;  /// The dropdowns for the other slots need updating if this changes
	/// Lock rebuilding dropdowns so that they can close on selection
	bool type_selection_locked_;
	bool tribe_selection_locked_;
	bool init_selection_locked_;
	bool team_selection_locked_;

	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;
};

MultiPlayerSetupGroup::MultiPlayerSetupGroup(UI::Panel* const launchgame,
                                             UI::Panel* const parent,
                                             int32_t const x,
                                             int32_t const y,
                                             int32_t const,
                                             int32_t const,
                                             GameSettingsProvider* const settings,
                                             uint32_t buth)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, x, y, UI::Box::Horizontal),
     settings_(settings),
     npsb_(new NetworkPlayerSettingsBackend(settings_)),
     launchgame_(launchgame),
     clientbox_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     playerbox_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     scrollable_playerbox_(&playerbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     clients_(&clientbox_,
              UI::PanelStyle::kFsMenu,
              UI::FontStyle::kFsGameSetupHeadings,
              0,
              0,
              0,
              0,
              _("Clients"),
              UI::Align::kCenter),
     players_(&playerbox_,
              UI::PanelStyle::kFsMenu,
              UI::FontStyle::kFsGameSetupHeadings,
              0,
              0,
              0,
              0,
              _("Players"),
              UI::Align::kCenter),
     buth_(buth) {
	clientbox_.add(&clients_, Resizing::kAlign, UI::Align::kCenter);
	clientbox_.add_space(3 * kPadding);
	clientbox_.set_scrolling(true);

	add(&clientbox_);
	add_space(8 * kPadding);
	add(&playerbox_, Resizing::kExpandBoth);
	playerbox_.add(&players_, Resizing::kAlign, UI::Align::kCenter);
	scrollable_playerbox_.set_scrolling(true);
	playerbox_.add_space(kPadding);

	playerbox_.add(&scrollable_playerbox_, Resizing::kExpandBoth);

	subscriber_ = Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings& n) {
		if (n.action == NoteGameSettings::Action::kMap) {
			reset();
		}
		update_players();
		update_clients();
	});
}

MultiPlayerSetupGroup::~MultiPlayerSetupGroup() = default;

/// Update which slots are available based on current settings.
void MultiPlayerSetupGroup::reset() {
	for (auto& p : multi_player_player_groups_) {
		p->die();
	}
	multi_player_player_groups_.clear();
	for (auto& c : multi_player_client_groups_) {
		c->die();
	}
	multi_player_client_groups_.clear();
}
void MultiPlayerSetupGroup::update_players() {
	const GameSettings& settings = settings_->settings();
	const size_t number_of_players = settings.players.size();

	for (PlayerSlot i = multi_player_player_groups_.size(); i < number_of_players; ++i) {
		multi_player_player_groups_.push_back(new MultiPlayerPlayerGroup(
		   &scrollable_playerbox_, playerbox_.get_w() - UI::Scrollbar::kSize, buth_, i, settings_,
		   npsb_.get()));
		scrollable_playerbox_.add(multi_player_player_groups_.at(i), Resizing::kFullSize);
	}
	for (auto& p : multi_player_player_groups_) {
		p->update();
	}
}
void MultiPlayerSetupGroup::update_clients() {
	const GameSettings& settings = settings_->settings();
	const size_t number_of_users = settings.users.size();

	if (number_of_users > multi_player_client_groups_.size()) {
		for (uint32_t i = multi_player_client_groups_.size(); i < number_of_users; ++i) {
			multi_player_client_groups_.push_back(new MultiPlayerClientGroup(
			   launchgame_, &clientbox_, clientbox_.get_w(), buth_, i, settings_));
			clientbox_.add(multi_player_client_groups_.at(i), Resizing::kFullSize);
		}
	}
	for (auto& c : multi_player_client_groups_) {
		c->update();
	}
}

void MultiPlayerSetupGroup::draw(RenderTarget& dst) {
	const int32_t total_box_height = scrollable_playerbox_.get_y() + scrollable_playerbox_.get_h();

	for (MultiPlayerPlayerGroup* current_player : multi_player_player_groups_) {
		if (current_player->get_y() < 0 && current_player->get_y() > -current_player->get_h()) {
			dst.brighten_rect(
			   Recti(playerbox_.get_x(), scrollable_playerbox_.get_y(), scrollable_playerbox_.get_w(),
			         current_player->get_h() + current_player->get_y()),
			   -MOUSE_OVER_BRIGHT_FACTOR);
		} else if (current_player->get_y() >= 0) {
			auto rect_height =
			   std::min(total_box_height - (scrollable_playerbox_.get_y() + current_player->get_y()),
			            current_player->get_h());
			dst.brighten_rect(
			   Recti(playerbox_.get_x(), scrollable_playerbox_.get_y() + current_player->get_y(),
			         scrollable_playerbox_.get_w(), rect_height < 0 ? 0 : rect_height),
			   -MOUSE_OVER_BRIGHT_FACTOR);
		}
	}
}

void MultiPlayerSetupGroup::force_new_dimensions(uint32_t max_width,
                                                 uint32_t max_height,
                                                 uint32_t standard_element_height) {
	buth_ = standard_element_height;
	clientbox_.set_min_desired_breadth(max_width / 3);
	clientbox_.set_max_size(max_width / 3, max_height);
	playerbox_.set_max_size(max_width / 2, max_height);
	scrollable_playerbox_.set_max_size(max_width / 2, max_height - players_.get_h() - 4 * kPadding);

	for (auto& multiPlayerClientGroup : multi_player_client_groups_) {
		multiPlayerClientGroup->force_new_dimensions(standard_element_height);
	}

	for (auto& multiPlayerPlayerGroup : multi_player_player_groups_) {
		multiPlayerPlayerGroup->force_new_dimensions(standard_element_height);
	}
}
