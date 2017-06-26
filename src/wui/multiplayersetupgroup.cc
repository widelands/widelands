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

#include "wui/multiplayersetupgroup.h"

#include <memory>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "ai/computer_player.h"
#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/playercolor.h"
#include "graphic/text_constants.h"
#include "logic/game.h"
#include "logic/game_settings.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/player.h"
#include "logic/widelands.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/mouse_constants.h"
#include "ui_basic/scrollbar.h"
#include "ui_basic/textarea.h"

#define AI_NAME_PREFIX "ai" AI_NAME_SEPARATOR

constexpr int kPadding = 4;

/// Holds the info and dropdown menu for a connected client
struct MultiPlayerClientGroup : public UI::Box {
	MultiPlayerClientGroup(UI::Panel* const parent,
	                       int32_t const w,
	                       int32_t const h,
	                       PlayerSlot id,
	                       GameSettingsProvider* const settings)
	   : UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h, kPadding),
	     slot_dropdown_(this, 0, 0, h, 200, h, _("Role"), UI::DropdownType::kPictorial),
	     // Name needs to be initialized after the dropdown, otherwise the layout function will
	     // crash.
	     name(this, 0, 0, w - h - UI::Scrollbar::kSize * 11 / 5, h),
	     s(settings),
	     id_(id),
	     slot_selection_locked_(false) {
		set_size(w, h);

		add(&slot_dropdown_);
		add(&name, UI::Box::Resizing::kAlign, UI::Align::kCenter);

		slot_dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		slot_dropdown_.selected.connect(
		   boost::bind(&MultiPlayerClientGroup::set_slot, boost::ref(*this)));

		update();
		layout();

		subscriber_ =
		   Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings& note) {
			   switch (note.action) {
			   case NoteGameSettings::Action::kMap:
				   /// In case the client gets kicked off its slot due to number of player slots in the
				   /// map
				   update();
				   break;
			   case NoteGameSettings::Action::kUser:
				   /// Player slot might have been closed, bumping the client to observer status. Also,
				   /// take note if another player changed their position.
				   if (id_ == note.usernum || note.usernum == UserSettings::none()) {
					   update();
				   }
				   break;
			   case NoteGameSettings::Action::kPlayer:
				   break;
			   }
			});
	}

	/// Update dropdown sizes
	void layout() override {
		UI::Box::layout();
		slot_dropdown_.set_height(g_gr->get_yres() * 3 / 4);
	}

	/// This will update the client's player slot with the value currently selected in the slot
	/// dropdown.
	void set_slot() {
		const GameSettings& settings = s->settings();
		if (id_ != settings.usernum) {
			return;
		}
		slot_selection_locked_ = true;
		if (slot_dropdown_.has_selection()) {
			const uint8_t new_slot = slot_dropdown_.get_selected();
			if (new_slot != settings.users.at(id_).position) {
				s->set_player_number(slot_dropdown_.get_selected());
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
				slot_dropdown_.add((boost::format(_("Player %u")) % cast_unsigned(slot + 1)).str(),
				                   slot, playercolor_image(slot, "images/players/genstats_player.png"),
				                   slot == user_setting.position);
			}
		}
		slot_dropdown_.add(_("Spectator"), UserSettings::none(),
		                   g_gr->images().get("images/wui/fieldaction/menu_tab_watch.png"),
		                   user_setting.position == UserSettings::none());
		slot_dropdown_.set_visible(true);
		slot_dropdown_.set_enabled(id_ == settings.usernum);
	}

	/// Take care of visibility and current values
	void update() {
		const GameSettings& settings = s->settings();
		const UserSettings& user_setting = settings.users.at(id_);

		if (user_setting.position == UserSettings::not_connected()) {
			set_visible(false);
			return;
		}

		name.set_text(user_setting.name);
		rebuild_slot_dropdown(settings);
	}

	UI::Dropdown<uintptr_t> slot_dropdown_;  /// Select the player slot.
	UI::Textarea name;                       /// Client nick name
	GameSettingsProvider* const s;
	uint8_t const id_;            /// User number
	bool slot_selection_locked_;  // Ensure that dropdowns will close on selection.
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;
};

/// Holds the dropdown menus for a player slot
struct MultiPlayerPlayerGroup : public UI::Box {
	MultiPlayerPlayerGroup(UI::Panel* const parent,
	                       int32_t const w,
	                       int32_t const h,
	                       PlayerSlot id,
	                       GameSettingsProvider* const settings,
	                       NetworkPlayerSettingsBackend* const npsb)
	   : UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h, kPadding / 2),
	     s(settings),
	     n(npsb),
	     id_(id),
	     player(this,
	            "player",
	            0,
	            0,
	            h,
	            h,
	            g_gr->images().get("images/ui_basic/but1.png"),
	            playercolor_image(id, "images/players/player_position_menu.png"),
	            (boost::format(_("Player %u")) % cast_unsigned(id_ + 1)).str(),
	            UI::Button::Style::kFlat),
	     type_dropdown_(this, 0, 0, 50, 200, h, _("Type"), UI::DropdownType::kPictorial),
	     tribes_dropdown_(this, 0, 0, 50, 200, h, _("Tribe"), UI::DropdownType::kPictorial),
	     init_dropdown_(
	        this, 0, 0, w - 4 * h - 3 * kPadding, 200, h, "", UI::DropdownType::kTextualNarrow),
	     team_dropdown_(this, 0, 0, h, 200, h, _("Team"), UI::DropdownType::kPictorial),
	     last_state_(PlayerSettings::State::kClosed),
	     type_selection_locked_(false),
	     tribe_selection_locked_(false),
	     init_selection_locked_(false),
	     team_selection_locked_(false) {
		set_size(w, h);

		player.set_disable_style(UI::ButtonDisableStyle::kFlat);
		player.set_enabled(false);

		type_dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		tribes_dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		init_dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		team_dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);

		type_dropdown_.selected.connect(
		   boost::bind(&MultiPlayerPlayerGroup::set_type, boost::ref(*this)));
		tribes_dropdown_.selected.connect(
		   boost::bind(&MultiPlayerPlayerGroup::set_tribe_or_shared_in, boost::ref(*this)));
		init_dropdown_.selected.connect(
		   boost::bind(&MultiPlayerPlayerGroup::set_init, boost::ref(*this)));
		team_dropdown_.selected.connect(
		   boost::bind(&MultiPlayerPlayerGroup::set_team, boost::ref(*this)));

		add_space(0);
		add(&player);
		add(&type_dropdown_);
		add(&tribes_dropdown_);
		add(&init_dropdown_);
		add(&team_dropdown_);
		add_space(0);

		subscriber_ =
		   Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings& note) {
			   switch (note.action) {
			   case NoteGameSettings::Action::kMap:
				   // We don't care about map updates, since we receive enough notifications for the
				   // slots.
				   break;
			   default:
				   if (s->settings().players.empty()) {
					   // No map/savegame yet
					   return;
				   }
				   if (id_ == note.position ||
				       s->settings().players[id_].state == PlayerSettings::State::kShared) {
					   update();
				   }
			   }
			});

		// Init dropdowns
		update();
		layout();
	}

	/// Update dropdown sizes
	void layout() override {
		const int max_height = g_gr->get_yres() * 3 / 4;
		type_dropdown_.set_height(max_height);
		tribes_dropdown_.set_height(max_height);
		init_dropdown_.set_height(max_height);
		team_dropdown_.set_height(max_height);
		UI::Box::layout();
	}

	/// This will update the game settings for the type with the value
	/// currently selected in the type dropdown.
	void set_type() {
		if (!s->can_change_player_state(id_)) {
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
				if (selected == AI_NAME_PREFIX "random") {
					n->set_player_ai(id_, "", true);
				} else {
					if (boost::starts_with(selected, AI_NAME_PREFIX)) {
						std::vector<std::string> parts;
						boost::split(parts, selected, boost::is_any_of(AI_NAME_SEPARATOR));
						assert(parts.size() == 2);
						n->set_player_ai(id_, parts[1], false);
					} else {
						throw wexception("Unknown player state: %s\n", selected.c_str());
					}
				}
			}
			n->set_player_state(id_, state);
		}
		type_selection_locked_ = false;
	}

	/// Rebuild the type dropdown from the server settings. This will keep the host and client UIs in
	/// sync.
	void rebuild_type_dropdown(const GameSettings& settings) {
		if (type_selection_locked_) {
			return;
		}
		type_dropdown_.clear();
		// AIs
		for (const auto* impl : ComputerPlayer::get_implementations()) {
			type_dropdown_.add(_(impl->descname),
			                   (boost::format(AI_NAME_PREFIX "%s") % impl->name).str(),
			                   g_gr->images().get(impl->icon_filename), false, _(impl->descname));
		}
		/** TRANSLATORS: This is the name of an AI used in the game setup screens */
		type_dropdown_.add(_("Random AI"), AI_NAME_PREFIX "random",
		                   g_gr->images().get("images/ai/ai_random.png"), false, _("Random AI"));

		// Slot state. Only add shared_in if there are viable slots
		if (settings.is_shared_usable(id_, settings.find_shared(id_))) {
			type_dropdown_.add(_("Shared in"), "shared_in",
			                   g_gr->images().get("images/ui_fsmenu/shared_in.png"), false,
			                   _("Shared in"));
		}

		// Do not close a player in savegames or scenarios
		if (!settings.uncloseable(id_)) {
			type_dropdown_.add(_("Closed"), "closed", g_gr->images().get("images/ui_basic/stop.png"),
			                   false, _("Closed"));
		}

		type_dropdown_.add(
		   _("Open"), "open", g_gr->images().get("images/ui_basic/continue.png"), false, _("Open"));

		type_dropdown_.set_enabled(s->can_change_player_state(id_));

		// Now select the entry according to server settings
		const PlayerSettings& player_setting = settings.players[id_];
		if (player_setting.state == PlayerSettings::State::kHuman) {
			type_dropdown_.set_image(g_gr->images().get("images/wui/stats/genstats_nrworkers.png"));
			type_dropdown_.set_tooltip((boost::format(_("%1%: %2%")) % _("Type") % _("Human")).str());
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
						type_dropdown_.select(AI_NAME_PREFIX "random");
					} else {
						const ComputerPlayer::Implementation* impl =
						   ComputerPlayer::get_implementation(player_setting.ai);
						type_dropdown_.select((boost::format(AI_NAME_PREFIX "%s") % impl->name).str());
					}
				}
			}
		}
	}

	/// Whether the client who is running the UI is allowed to change the tribe for this player slot.
	bool has_tribe_access() {
		return s->settings().players[id_].state == PlayerSettings::State::kShared ?
		          s->can_change_player_init(id_) :
		          s->can_change_player_tribe(id_);
	}

	/// This will update the game settings for the tribe or shared_in with the value
	/// currently selected in the tribes dropdown.
	void set_tribe_or_shared_in() {
		if (!has_tribe_access()) {
			return;
		}
		const PlayerSettings& player_settings = s->settings().players[id_];
		tribe_selection_locked_ = true;
		tribes_dropdown_.set_disable_style(player_settings.state == PlayerSettings::State::kShared ?
		                                      UI::ButtonDisableStyle::kPermpressed :
		                                      UI::ButtonDisableStyle::kFlat);
		if (tribes_dropdown_.has_selection()) {
			if (player_settings.state == PlayerSettings::State::kShared) {
				n->set_player_shared(
				   id_, boost::lexical_cast<unsigned int>(tribes_dropdown_.get_selected()));
			} else {
				n->set_player_tribe(id_, tribes_dropdown_.get_selected());
			}
		}
		tribe_selection_locked_ = false;
	}

	/// Rebuild the tribes dropdown from the server settings. This will keep the host and client UIs
	/// in sync.
	void rebuild_tribes_dropdown(const GameSettings& settings) {
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

					const Image* player_image =
					   playercolor_image(i, "images/players/player_position_menu.png");
					assert(player_image);
					const std::string player_name =
					   /** TRANSLATORS: This is an option in multiplayer setup for sharing
					      another player's starting position. */
					   (boost::format(_("Shared in Player %u")) % cast_unsigned(i + 1)).str();
					tribes_dropdown_.add(player_name,
					                     boost::lexical_cast<std::string>(cast_unsigned(i + 1)),
					                     player_image, (i + 1) == player_setting.shared_in, player_name);
				}
			}
			tribes_dropdown_.set_enabled(tribes_dropdown_.size() > 1);
		} else {
			{
				i18n::Textdomain td("tribes");
				for (const TribeBasicInfo& tribeinfo : Widelands::get_all_tribeinfos()) {
					tribes_dropdown_.add(_(tribeinfo.descname), tribeinfo.name,
					                     g_gr->images().get(tribeinfo.icon), false, tribeinfo.tooltip);
				}
			}
			tribes_dropdown_.add(pgettext("tribe", "Random"), "random",
			                     g_gr->images().get("images/ui_fsmenu/random.png"), false,
			                     _("The tribe will be selected at random"));
			if (player_setting.random_tribe) {
				tribes_dropdown_.select("random");
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
		if (!s->can_change_player_init(id_)) {
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
		} else if (settings.savegame) {
			/** Translators: This is a game type */
			init_dropdown_.set_label(_("Saved Game"));
		} else {
			init_dropdown_.set_label("");
			i18n::Textdomain td("tribes");  // for translated initialisation
			const TribeBasicInfo tribeinfo = Widelands::get_tribeinfo(player_setting.tribe);
			for (size_t i = 0; i < tribeinfo.initializations.size(); ++i) {
				const TribeBasicInfo::Initialization& addme = tribeinfo.initializations[i];
				init_dropdown_.add(_(addme.descname), i, nullptr,
				                   i == player_setting.initialization_index, _(addme.tooltip));
			}
		}

		init_dropdown_.set_visible(true);
		init_dropdown_.set_enabled(s->can_change_player_init(id_));
	}

	/// This will update the team settings with the value currently selected in the teams dropdown.
	void set_team() {
		team_selection_locked_ = true;
		if (team_dropdown_.has_selection()) {
			n->set_player_team(id_, team_dropdown_.get_selected());
		}
		team_selection_locked_ = false;
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
		team_dropdown_.add(_("No Team"), 0, g_gr->images().get("images/players/no_team.png"));
#ifndef NDEBUG
		const size_t no_of_team_colors = sizeof(kTeamColors) / sizeof(kTeamColors[0]);
#endif
		for (Widelands::TeamNumber t = 1; t <= settings.players.size() / 2; ++t) {
			assert(t < no_of_team_colors);
			team_dropdown_.add((boost::format(_("Team %d")) % cast_unsigned(t)).str(), t,
			                   playercolor_image(kTeamColors[t], "images/players/team.png"));
		}
		team_dropdown_.select(player_setting.team);
		team_dropdown_.set_visible(true);
		team_dropdown_.set_enabled(s->can_change_player_team(id_));
	}

	/// Refresh all user interfaces
	void update() {
		const GameSettings& settings = s->settings();
		if (id_ >= settings.players.size()) {
			set_visible(false);
			return;
		}

		const PlayerSettings& player_setting = settings.players[id_];
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
		} else {
			rebuild_tribes_dropdown(settings);
			rebuild_init_dropdown(settings);
			rebuild_team_dropdown(settings);
		}

		// Trigger update for the other players for shared_in mode when slots open and close
		if (last_state_ != player_setting.state) {
			last_state_ = player_setting.state;
			for (PlayerSlot slot = 0; slot < s->settings().players.size(); ++slot) {
				if (slot != id_) {
					n->set_player_state(slot, settings.players[slot].state);
				}
			}
		}
	}

	GameSettingsProvider* const s;
	NetworkPlayerSettingsBackend* const n;
	PlayerSlot const id_;

	UI::Button player;
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

MultiPlayerSetupGroup::MultiPlayerSetupGroup(UI::Panel* const parent,
                                             int32_t const x,
                                             int32_t const y,
                                             int32_t const w,
                                             int32_t const h,
                                             GameSettingsProvider* const settings,
                                             uint32_t buth)
   : UI::Box(parent, x, y, UI::Box::Horizontal, w, h, 8 * kPadding),
     s(settings),
     npsb(new NetworkPlayerSettingsBackend(s)),
     clientbox(this, 0, 0, UI::Box::Vertical),
     playerbox(this, 0, 0, UI::Box::Vertical, w * 9 / 15, h, kPadding),
     buth_(buth) {
	clientbox.set_size(w / 3, h);
	clientbox.set_scrolling(true);

	add(&clientbox, UI::Box::Resizing::kExpandBoth);
	add(&playerbox);

	// Playerbox
	playerbox.set_size(w * 9 / 15, h);
	playerbox.add_space(0);
	multi_player_player_groups.resize(kMaxPlayers);
	for (PlayerSlot i = 0; i < multi_player_player_groups.size(); ++i) {
		multi_player_player_groups.at(i) =
		   new MultiPlayerPlayerGroup(&playerbox, playerbox.get_w(), buth_, i, s, npsb.get());
		playerbox.add(multi_player_player_groups.at(i));
	}
	playerbox.add_space(0);

	subscriber_ =
	   Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings&) { update(); });
	set_size(w, h);
	update();
}

MultiPlayerSetupGroup::~MultiPlayerSetupGroup() {
}

/// Update which slots are available based on current settings.
void MultiPlayerSetupGroup::update() {
	const GameSettings& settings = s->settings();

	// Update / initialize client groups
	if (multi_player_client_groups.size() < settings.users.size()) {
		multi_player_client_groups.resize(settings.users.size());
	}
	for (uint32_t i = 0; i < settings.users.size(); ++i) {
		if (!multi_player_client_groups.at(i)) {
			multi_player_client_groups.at(i) =
			   new MultiPlayerClientGroup(&clientbox, clientbox.get_w(), buth_, i, s);
			clientbox.add(multi_player_client_groups.at(i), UI::Box::Resizing::kFullSize);
			multi_player_client_groups.at(i)->layout();
		}
		multi_player_client_groups.at(i)->set_visible(true);
	}

	// Keep track of which player slots are visible
	for (PlayerSlot i = 0; i < multi_player_player_groups.size(); ++i) {
		const bool should_be_visible = i < settings.players.size();
		if (should_be_visible != multi_player_player_groups.at(i)->is_visible()) {
			multi_player_player_groups.at(i)->set_visible(should_be_visible);
		}
	}
}

void MultiPlayerSetupGroup::draw(RenderTarget& dst) {
	for (MultiPlayerPlayerGroup* player_group : multi_player_player_groups) {
		if (player_group->is_visible()) {
			dst.brighten_rect(
			   Recti(playerbox.get_x(), playerbox.get_y() + player_group->get_y() - kPadding / 2,
			         playerbox.get_w() + kPadding, player_group->get_h() + kPadding),
			   -MOUSE_OVER_BRIGHT_FACTOR);
		}
	}
}
