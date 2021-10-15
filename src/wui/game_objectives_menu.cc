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

#include "wui/game_objectives_menu.h"

#include "base/time_string.h"
#include "graphic/font_handler.h"
#include "logic/game_data_error.h"
#include "logic/objective.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "wui/interactive_player.h"

constexpr int16_t kSpacing = 4;
constexpr int16_t kRowSize = 32;
constexpr int16_t kButtonWidth = 128;
constexpr int16_t kStatusWidth = 300;

GameObjectivesMenu::GameObjectivesMenu(InteractivePlayer& parent, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      "objectives",
                      &registry,
                      300, 200,
                      _("Objectives & Diplomacy")),
     iplayer_(parent),
     tabs_(this, UI::TabPanelStyle::kWuiLight),
     objective_box_(&tabs_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     diplomacy_box_(&tabs_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     objective_list_(&objective_box_, 0, 0, 550, 180, UI::PanelStyle::kWui),
     objective_text_(&objective_box_,
                   0,
                   0,
                   100,
                   150,
                   UI::PanelStyle::kWui,
                   "",
                   UI::Align::kLeft,
                   UI::MultilineTextarea::ScrollMode::kScrollNormalForced) {

	objective_box_.add(&objective_list_, UI::Box::Resizing::kExpandBoth);
	objective_box_.add(&objective_text_, UI::Box::Resizing::kExpandBoth);

	UI::Box* hbox = new UI::Box(&diplomacy_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	UI::Box* vbox_flag = new UI::Box(hbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	UI::Box* vbox_name = new UI::Box(hbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	UI::Box* vbox_team = new UI::Box(hbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	UI::Box* vbox_status = new UI::Box(hbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	UI::Box* vbox_action = new UI::Box(hbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);

	const bool rtl = UI::g_fh->fontset()->is_rtl();
	const bool show_all_players = iplayer_.player().see_all() || iplayer_.omnipotent();
	iterate_players_existing_const(p, iplayer_.egbase().player_manager()->get_number_of_players(), iplayer_.egbase(), player) {
		if (p != iplayer_.player_number() && !show_all_players && player->is_hidden_from_general_statistics()) {
			continue;
		}

		UI::Icon* icon_flag = new UI::Icon(vbox_flag, UI::PanelStyle::kWui, 0, 0, kRowSize, kRowSize,
				playercolor_image(player->get_playercolor(), "images/players/genstats_player.png"));
		UI::Icon* icon_team = new UI::Icon(vbox_team, UI::PanelStyle::kWui, 0, 0, kRowSize, kRowSize, nullptr);
		UI::Textarea* txt_name = new UI::Textarea(vbox_name, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, player->get_name(),
				UI::mirror_alignment(UI::Align::kLeft, rtl));
		UI::Textarea* txt_status = new UI::Textarea(vbox_status, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, "", UI::mirror_alignment(UI::Align::kRight, rtl));

		icon_team->set_handle_mouse(true);
		icon_flag->set_handle_mouse(true);
		icon_flag->set_tooltip((boost::format(_("Player %u")) % static_cast<unsigned>(p)).str());
		txt_status->set_fixed_width(kStatusWidth);

		UI::Button* b1 = nullptr;
		UI::Button* b2 = nullptr;
		UI::Box* buttonsbox = new UI::Box(vbox_action, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
		if (p == iplayer_.player_number()) {
			b1 = new UI::Button(buttonsbox, "leave", 0, 0, kButtonWidth, kRowSize, UI::ButtonStyle::kWuiSecondary,
					_("Leave"), _("Leave your current team and become teamless"));
			b2 = new UI::Button(buttonsbox, "resign", 0, 0, kButtonWidth, kRowSize, UI::ButtonStyle::kWuiSecondary,
					_("Resign"), _("Give up and become a spectator"));
			b1->sigclicked.connect([this]() {
				// NOCOM leave
			});
			b2->sigclicked.connect([this]() {
				// NOCOM resign
			});
		} else {
			b1 = new UI::Button(buttonsbox, "join", 0, 0, kButtonWidth, kRowSize, UI::ButtonStyle::kWuiSecondary,
					_("Join"), _("Request to join this player’s team"));
			b2 = new UI::Button(buttonsbox, "invite", 0, 0, kButtonWidth, kRowSize, UI::ButtonStyle::kWuiSecondary,
					_("Invite"), _("Invite this player to join your team"));
			b1->sigclicked.connect([this]() {
				// NOCOM join
			});
			b2->sigclicked.connect([this]() {
				// NOCOM invite
			});
		}

		buttonsbox->add(b1, UI::Box::Resizing::kExpandBoth);
		buttonsbox->add_space(kSpacing);
		buttonsbox->add(b2, UI::Box::Resizing::kExpandBoth);

		vbox_name->add_space(kSpacing);
		vbox_status->add_space(kSpacing);
		vbox_name->add(txt_name, UI::Box::Resizing::kFillSpace, UI::Align::kCenter);
		vbox_status->add(txt_status, UI::Box::Resizing::kFillSpace, UI::Align::kCenter);
		vbox_team->add(icon_team, UI::Box::Resizing::kExpandBoth);
		vbox_flag->add(icon_flag, UI::Box::Resizing::kExpandBoth);
		vbox_action->add(buttonsbox, UI::Box::Resizing::kExpandBoth);

		diplomacy_teams_[p] = icon_team;
		diplomacy_status_[p] = txt_status;
		diplomacy_buttons_[p] = {b1, b2};

		for (UI::Box* b : { vbox_flag, vbox_name, vbox_team, vbox_status, vbox_action }) {
			b->add_space(kSpacing);
			b->add_inf_space();
		}
	}

	hbox->add(rtl ? vbox_action : vbox_flag, UI::Box::Resizing::kExpandBoth);
	hbox->add_space(kSpacing);
	hbox->add(rtl ? vbox_team : vbox_name, UI::Box::Resizing::kExpandBoth);
	hbox->add_space(kSpacing);
	hbox->add(vbox_status, UI::Box::Resizing::kExpandBoth);
	hbox->add_space(kSpacing);
	hbox->add(rtl ? vbox_name : vbox_team, UI::Box::Resizing::kExpandBoth);
	hbox->add_space(kSpacing);
	hbox->add(rtl ? vbox_flag : vbox_action, UI::Box::Resizing::kExpandBoth);
	diplomacy_box_.add(hbox, UI::Box::Resizing::kExpandBoth);

	tabs_.add("objectives", _("Objectives"), &objective_box_);
	tabs_.add("diplomacy", _("Diplomacy"), &diplomacy_box_);

	objective_list_.selected.connect([this](uint32_t a) { selected(a); });
	if (get_usedefaultpos()) {
		center_to_parent();
	}

	set_center_panel(&tabs_);
	update_diplomacy_details();
	initialization_complete();
}

void GameObjectivesMenu::update_diplomacy_details() {
	for (auto& pair : diplomacy_teams_) {
		const unsigned t = iplayer_.egbase().player(pair.first).team_number();
		pair.second->set_icon(playercolor_image(kTeamColors[t /* it's 1-based, 0 means No Team */],
				t == 0 ? "images/players/no_team.png" : "images/players/team.png"));
		pair.second->set_tooltip(t == 0 ? _("No team") : (boost::format(_("Team %u")) % t).str());
	}

	std::set<Widelands::PlayerNumber> players_with_result;
	for (auto& pair : diplomacy_status_) {
		std::string str;
		for (const auto& s : iplayer_.egbase().player_manager()->get_players_end_status()) {
			if (s.player == pair.first) {
				players_with_result.insert(s.player);
				switch (s.result) {
				case Widelands::PlayerEndResult::kWon:
					str = (boost::format(_("Won at %s")) % gametimestring(s.time.get())).str();
					break;
				case Widelands::PlayerEndResult::kLost:
					str = (boost::format(_("Lost at %s")) % gametimestring(s.time.get())).str();
					break;
				case Widelands::PlayerEndResult::kResigned:
					str = (boost::format(_("Resigned at %s")) % gametimestring(s.time.get())).str();
					break;
				default: break;
				}
				break;
			}
		}
		pair.second->set_text(str);
	}

	const unsigned own_team = iplayer_.egbase().player(iplayer_.player_number()).team_number();
	for (auto& pair : diplomacy_buttons_) {
		if (players_with_result.count(pair.first)) {
			// Ignore players who are no longer playing
			pair.second.first->set_enabled(false);
			pair.second.second->set_enabled(false);
		} else if (pair.first == iplayer_.player_number()) {
			// Self. The two buttons are "Leave team" and "Resign".
			pair.second.first->set_enabled(own_team > 0);
		} else {
			// Other player. The two buttons are "Request to join" and "Invite to join".
			const bool can_join = (own_team != iplayer_.egbase().player(pair.first).team_number()) || own_team == 0;
			pair.second.first->set_enabled(can_join);
			pair.second.second->set_enabled(can_join);
		}
	}
}

void GameObjectivesMenu::think() {
	//  Adjust the list according to the game state.
	for (const auto& pair : iplayer_.game().map().objectives()) {
		const Widelands::Objective& obj = *(pair.second);
		bool should_show = obj.visible() && !obj.done();
		uint32_t const list_size = objective_list_.size();
		for (uint32_t j = 0;; ++j) {
			if (j == list_size) {  //  the objective is not in our list
				if (should_show) {
					objective_list_.add(obj.descname(), obj);
				}
				break;
			}
			if (&objective_list_[j] == &obj) {  //  the objective is in our list
				if (!should_show) {
					objective_list_.remove(j);
				} else if (objective_list_[j].descname() != obj.descname() || objective_list_[j].descr() != obj.descr()) {
					// Update
					objective_list_.remove(j);
					objective_list_.add(obj.descname(), obj);
				}
				break;
			}
		}
	}
	objective_list_.sort();
	if (!objective_list_.empty() && !objective_list_.has_selection()) {
		objective_list_.select(0);
	}

	update_diplomacy_details();
}

/**
 * An entry in the objectives menu has been selected
 */
void GameObjectivesMenu::selected(uint32_t const t) {
	objective_text_.set_text(t == ListType::no_selection_index() ? "" : objective_list_[t].descr());
}

constexpr uint16_t kCurrentPacketVersion = 2;
UI::Window& GameObjectivesMenu::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		// TODO(Nordfriese): Savegame compatibility v1.1
		if (packet_version >= 1 && packet_version <= kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r = dynamic_cast<InteractivePlayer&>(ib).objectives_;
			r.create();
			assert(r.window);
			GameObjectivesMenu& m = dynamic_cast<GameObjectivesMenu&>(*r.window);
			m.think();  // Fills the list
			m.tabs_.activate(packet_version < 2 ? 0 : fr.unsigned_8());
			m.objective_list_.select(fr.unsigned_32());
			return m;
		} else {
			throw Widelands::UnhandledVersionError(
			   "Objectives Menu", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError("objectives menu: %s", e.what());
	}
}
void GameObjectivesMenu::save(FileWrite& fw, Widelands::MapObjectSaver&) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_8(tabs_.active());
	fw.unsigned_32(objective_list_.selection_index());
}
