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

#include <string>

#include <boost/format.hpp>

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
#include "logic/player.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/icon.h"
#include "ui_basic/scrollbar.h"
#include "ui_basic/textarea.h"

struct MultiPlayerClientGroup : public UI::Box {
	MultiPlayerClientGroup(UI::Panel* const parent,
	                       uint8_t id,
	                       int32_t const /* x */,
	                       int32_t const /* y */,
	                       int32_t const w,
	                       int32_t const h,
	                       GameSettingsProvider* const settings)
	   : UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h),
	     type_icon(nullptr),
	     type(nullptr),
	     s(settings),
	     id_(id),
	     save_(-2) {
		set_size(w, h);
		name = new UI::Textarea(this, 0, 0, w - h - UI::Scrollbar::kSize * 11 / 5, h);
		add(name, UI::Align::kCenter);
		// Either Button if changeable OR text if not
		if (id == settings->settings().usernum) {  // Our Client
			type = new UI::Button(
			   this, "client_type", 0, 0, h, h, g_gr->images().get("images/ui_basic/but1.png"), "");
			type->sigclicked.connect(
			   boost::bind(&MultiPlayerClientGroup::toggle_type, boost::ref(*this)));
			add(type);
		} else {  // just a shown client
			type_icon = new UI::Icon(
			   this, 0, 0, h, h, g_gr->images().get("images/wui/fieldaction/menu_tab_watch.png"));
			add(type_icon, UI::Align::kCenter);
		}
	}

	/// Switch human players and spectator
	void toggle_type() {
		UserSettings us = s->settings().users.at(id_);
		int16_t p = us.position;
		if (p == UserSettings::none())
			p = -1;

		for (++p; p < static_cast<int16_t>(s->settings().players.size()); ++p) {
			if (s->settings().players.at(p).state == PlayerSettings::stateHuman ||
			    s->settings().players.at(p).state == PlayerSettings::stateOpen) {
				s->set_player_number(p);
				return;
			}
		}
		s->set_player_number(UserSettings::none());
	}

	/// Care about visibility and current values
	void refresh() {
		UserSettings us = s->settings().users.at(id_);
		if (us.position == UserSettings::not_connected()) {
			name->set_text((boost::format("<%s>") % _("free")).str());
			if (type)
				type->set_visible(false);
			else
				type_icon->set_visible(false);
		} else {
			name->set_text(us.name);
			if (save_ != us.position) {
				const Image* position_image;
				std::string temp_tooltip;
				if (us.position < UserSettings::highest_playernum()) {
					position_image = playercolor_image(
					   us.position, g_gr->images().get("images/players/genstats_player.png"),
					   g_gr->images().get("images/players/genstats_player_pc.png"));
					temp_tooltip =
					   (boost::format(_("Player %u")) % static_cast<unsigned int>(us.position + 1))
					      .str();
				} else {
					position_image = g_gr->images().get("images/wui/fieldaction/menu_tab_watch.png");
					temp_tooltip = _("Spectator");
				}

				// Either Button if changeable OR text if not
				if (id_ == s->settings().usernum) {
					type->set_pic(position_image);
					type->set_tooltip(temp_tooltip);
					type->set_visible(true);
				} else {
					type_icon->set_icon(position_image);
					type_icon->set_tooltip(temp_tooltip);
					type_icon->set_visible(true);
				}
				save_ = us.position;
			}
		}
	}

	UI::Textarea* name;
	UI::Icon* type_icon;
	UI::Button* type;
	GameSettingsProvider* const s;
	uint8_t const id_;
	int16_t save_;  // saved position to check rewrite need.
};

struct MultiPlayerPlayerGroup : public UI::Box {
	MultiPlayerPlayerGroup(UI::Panel* const parent,
	                       uint8_t id,
	                       int32_t const /* x */,
	                       int32_t const /* y */,
	                       int32_t const w,
	                       int32_t const h,
	                       GameSettingsProvider* const settings,
	                       NetworkPlayerSettingsBackend* const npsb,
	                       std::map<std::string, const Image*>& tp,
	                       std::map<std::string, std::string>& tn)
	   : UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h),
	     player(nullptr),
	     type(nullptr),
	     tribe(nullptr),
	     init(nullptr),
	     s(settings),
	     n(npsb),
	     id_(id),
	     tribepics_(tp),
	     tribenames_(tn) {
		set_size(w, h);
		const Image* player_image =
		   playercolor_image(id, g_gr->images().get("images/players/player_position_menu.png"),
		                     g_gr->images().get("images/players/player_position_menu_pc.png"));
		assert(player_image);
		player = new UI::Icon(this, 0, 0, h, h, player_image);
		add(player, UI::Align::kCenter);
		type = new UI::Button(
		   this, "player_type", 0, 0, h, h, g_gr->images().get("images/ui_basic/but1.png"), "");
		type->sigclicked.connect(
		   boost::bind(&MultiPlayerPlayerGroup::toggle_type, boost::ref(*this)));
		add(type, UI::Align::kCenter);
		tribe = new UI::Button(
		   this, "player_tribe", 0, 0, h, h, g_gr->images().get("images/ui_basic/but1.png"), "");
		tribe->sigclicked.connect(
		   boost::bind(&MultiPlayerPlayerGroup::toggle_tribe, boost::ref(*this)));
		add(tribe, UI::Align::kCenter);
		init = new UI::Button(this, "player_init", 0, 0, w - 4 * h, h,
		                      g_gr->images().get("images/ui_basic/but1.png"), "");
		init->sigclicked.connect(
		   boost::bind(&MultiPlayerPlayerGroup::toggle_init, boost::ref(*this)));
		add(init, UI::Align::kCenter);
		team = new UI::Button(
		   this, "player_team", 0, 0, h, h, g_gr->images().get("images/ui_basic/but1.png"), "");
		team->sigclicked.connect(
		   boost::bind(&MultiPlayerPlayerGroup::toggle_team, boost::ref(*this)));
		add(team, UI::Align::kCenter);
	}

	/// Toggle through the types
	void toggle_type() {
		n->toggle_type(id_);
	}

	/// Toggle through the tribes + handle shared in players
	void toggle_tribe() {
		n->toggle_tribe(id_);
	}

	/// Toggle through the initializations
	void toggle_init() {
		n->toggle_init(id_);
	}

	/// Toggle through the teams
	void toggle_team() {
		n->toggle_team(id_);
	}

	/// Refresh all user interfaces
	void refresh() {
		const GameSettings& settings = s->settings();

		if (id_ >= settings.players.size()) {
			set_visible(false);
			return;
		}

		n->refresh(id_);

		set_visible(true);

		const PlayerSettings& player_setting = settings.players[id_];
		bool typeaccess = s->can_change_player_state(id_);
		bool tribeaccess = s->can_change_player_tribe(id_);
		bool const initaccess = s->can_change_player_init(id_);
		bool teamaccess = s->can_change_player_team(id_);

		type->set_enabled(typeaccess);
		if (player_setting.state == PlayerSettings::stateClosed) {
			type->set_tooltip(_("Closed"));
			type->set_pic(g_gr->images().get("images/ui_basic/stop.png"));
			team->set_visible(false);
			team->set_enabled(false);
			tribe->set_visible(false);
			tribe->set_enabled(false);
			tribe->set_style(UI::Button::Style::kRaised);
			init->set_visible(false);
			init->set_enabled(false);
			return;
		} else if (player_setting.state == PlayerSettings::stateOpen) {
			type->set_tooltip(_("Open"));
			type->set_pic(g_gr->images().get("images/ui_basic/continue.png"));
			team->set_visible(false);
			team->set_enabled(false);
			tribe->set_visible(false);
			tribe->set_enabled(false);
			tribe->set_style(UI::Button::Style::kRaised);
			init->set_visible(false);
			init->set_enabled(false);
			return;
		} else if (player_setting.state == PlayerSettings::stateShared) {
			type->set_tooltip(_("Shared in"));
			type->set_pic(g_gr->images().get("images/ui_fsmenu/shared_in.png"));
			const Image* player_image =
			   playercolor_image(player_setting.shared_in - 1,
			                     g_gr->images().get("images/players/player_position_menu.png"),
			                     g_gr->images().get("images/players/player_position_menu_pc.png"));
			assert(player_image);
			tribe->set_pic(player_image);
			tribe->set_tooltip(
			   (boost::format(_("Player %u")) % static_cast<unsigned int>(player_setting.shared_in))
			      .str());

			team->set_visible(false);
			team->set_enabled(false);
			// Flat ~= icon
			tribe->set_style(initaccess ? UI::Button::Style::kRaised : UI::Button::Style::kFlat);
			tribe->set_enabled(true);
		} else {
			std::string title;
			std::string pic = "images/";
			if (player_setting.state == PlayerSettings::stateComputer) {
				if (player_setting.ai.empty()) {
					title = _("Computer");
					pic += "novalue.png";
				} else {
					if (player_setting.random_ai) {
						/** TRANSLATORS: This is the name of an AI used in the game setup screens */
						title = _("Random AI");
						pic += "ai/ai_random.png";
					} else {
						const ComputerPlayer::Implementation* impl =
						   ComputerPlayer::get_implementation(player_setting.ai);
						title = _(impl->descname);
						pic = impl->icon_filename;
					}
				}
			} else {  // PlayerSettings::stateHuman
				title = _("Human");
				pic += "wui/stats/genstats_nrworkers.png";
			}
			type->set_tooltip(title.c_str());
			type->set_pic(g_gr->images().get(pic));
			if (player_setting.random_tribe) {
				std::string random = pgettext("tribe", "Random");
				if (!tribenames_["random"].size())
					tribepics_[random] = g_gr->images().get("images/ui_fsmenu/random.png");
				tribe->set_tooltip(random.c_str());
				tribe->set_pic(tribepics_[random]);
			} else {
				if (!tribenames_[player_setting.tribe].size()) {
					// get tribes name and picture
					i18n::Textdomain td("tribes");
					for (const TribeBasicInfo& tribeinfo : settings.tribes) {
						tribenames_[tribeinfo.name] = _(tribeinfo.descname);
						tribepics_[tribeinfo.name] = g_gr->images().get(tribeinfo.icon);
					}
				}
				tribe->set_tooltip(tribenames_[player_setting.tribe].c_str());
				tribe->set_pic(tribepics_[player_setting.tribe]);
			}
			tribe->set_style(UI::Button::Style::kRaised);

			if (player_setting.team) {
				team->set_title(std::to_string(static_cast<unsigned int>(player_setting.team)));
			} else {
				team->set_title("--");
			}
			team->set_visible(true);
			team->set_enabled(teamaccess);
			tribe->set_enabled(tribeaccess);
		}
		init->set_enabled(initaccess);
		tribe->set_visible(true);
		init->set_visible(true);

		if (settings.scenario)
			init->set_title(_("Scenario"));
		else if (settings.savegame)
			/** Translators: This is a game type */
			init->set_title(_("Saved Game"));
		else {
			i18n::Textdomain td("tribes");  // for translated initialisation
			for (const TribeBasicInfo& tribeinfo : settings.tribes) {
				if (tribeinfo.name == player_setting.tribe) {
					init->set_title(
					   _(tribeinfo.initializations.at(player_setting.initialization_index).descname));
					init->set_tooltip(
					   _(tribeinfo.initializations.at(player_setting.initialization_index).tooltip));
					break;
				}
			}
		}
	}

	UI::Icon* player;
	UI::Button* type;
	UI::Button* tribe;
	UI::Button* init;
	UI::Button* team;
	GameSettingsProvider* const s;
	NetworkPlayerSettingsBackend* const n;
	uint8_t const id_;
	std::map<std::string, const Image*>& tribepics_;
	std::map<std::string, std::string>& tribenames_;
};

MultiPlayerSetupGroup::MultiPlayerSetupGroup(UI::Panel* const parent,
                                             int32_t const x,
                                             int32_t const y,
                                             int32_t const w,
                                             int32_t const h,
                                             GameSettingsProvider* const settings,
                                             uint32_t /* butw */,
                                             uint32_t buth)
   : UI::Panel(parent, x, y, w, h),
     s(settings),
     npsb(new NetworkPlayerSettingsBackend(s)),
     clientbox(this, 0, buth, UI::Box::Vertical, w / 3, h - buth),
     playerbox(this, w * 6 / 15, buth, UI::Box::Vertical, w * 9 / 15, h - buth),
     buth_(buth) {
	int small_font = UI_FONT_SIZE_SMALL * 3 / 4;

	// Clientbox and labels
	labels.push_back(new UI::Textarea(
	   this, UI::Scrollbar::kSize * 6 / 5, buth / 3, w / 3 - buth - UI::Scrollbar::kSize * 2, buth));
	labels.back()->set_text(_("Client name"));
	labels.back()->set_fontsize(small_font);

	labels.push_back(new UI::Textarea(
	   this, w / 3 - buth - UI::Scrollbar::kSize * 6 / 5, buth / 3, buth * 2, buth));
	labels.back()->set_text(_("Role"));
	labels.back()->set_fontsize(small_font);

	clientbox.set_size(w / 3, h - buth);
	clientbox.set_scrolling(true);

	// Playerbox and labels
	labels.push_back(new UI::Textarea(this, w * 6 / 15, buth / 3, buth, buth));
	labels.back()->set_text(_("Start"));
	labels.back()->set_fontsize(small_font);

	labels.push_back(new UI::Textarea(this, w * 6 / 15 + buth, buth / 3 - 10, buth, buth));
	labels.back()->set_text(_("Type"));
	labels.back()->set_fontsize(small_font);

	labels.push_back(new UI::Textarea(this, w * 6 / 15 + buth * 2, buth / 3, buth, buth));
	labels.back()->set_text(_("Tribe"));
	labels.back()->set_fontsize(small_font);

	labels.push_back(new UI::Textarea(
		this, w * 6 / 15 + buth * 3, buth / 3, w * 9 / 15 - 4 * buth, buth, UI::Align::kCenter));
	labels.back()->set_text(_("Initialization"));
	labels.back()->set_fontsize(small_font);

	labels.push_back(new UI::Textarea(this, w - buth, buth / 3, buth, buth, UI::Align::kRight));
	labels.back()->set_text(_("Team"));
	labels.back()->set_fontsize(small_font);

	playerbox.set_size(w * 9 / 15, h - buth);
	multi_player_player_groups.resize(kMaxPlayers);
	for (uint8_t i = 0; i < multi_player_player_groups.size(); ++i) {
		multi_player_player_groups.at(i) = new MultiPlayerPlayerGroup(
		   &playerbox, i, 0, 0, playerbox.get_w(), buth, s, npsb.get(), tribepics_, tribenames_);
		playerbox.add(multi_player_player_groups.at(i), UI::Align::kCenter);
	}
	refresh();
}

MultiPlayerSetupGroup::~MultiPlayerSetupGroup() {
}

/**
 * Update display and enabled buttons based on current settings.
 */
void MultiPlayerSetupGroup::refresh() {
	const GameSettings& settings = s->settings();

	// Update / initialize client groups
	if (multi_player_client_groups.size() < settings.users.size()) {
		multi_player_client_groups.resize(settings.users.size());
	}
	for (uint32_t i = 0; i < settings.users.size(); ++i) {
		if (!multi_player_client_groups.at(i)) {
			multi_player_client_groups.at(i) =
			   new MultiPlayerClientGroup(&clientbox, i, 0, 0, clientbox.get_w(), buth_, s);
			clientbox.add(&*multi_player_client_groups.at(i), UI::Align::kCenter);
		}
		multi_player_client_groups.at(i)->refresh();
	}

	// Update player groups
	for (uint32_t i = 0; i < kMaxPlayers; ++i) {
		multi_player_player_groups.at(i)->refresh();
	}
}
