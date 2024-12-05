/*
 * Copyright (C) 2022-2024 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/game_diplomacy_menu.h"

#include "base/time_string.h"
#include "graphic/font_handler.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "wui/actionconfirm.h"
#include "wui/interactive_player.h"

constexpr int16_t kSpacing = 4;
constexpr int16_t kRowSize = 32;
constexpr int16_t kButtonWidth = 128;

GameDiplomacyMenu::GameDiplomacyMenu(InteractiveGameBase& parent,
                                     UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "diplomacy", &registry, 300, 200, _("Diplomacy")),
     igbase_(parent),
     iplayer_(dynamic_cast<InteractivePlayer*>(&igbase_)),
     diplomacy_box_(this, UI::PanelStyle::kWui, "main_box", 0, 0, UI::Box::Vertical),
     hbox_(&diplomacy_box_, UI::PanelStyle::kWui, "hbox", 0, 0, UI::Box::Horizontal),
     vbox_flag_(&hbox_, UI::PanelStyle::kWui, "flag_vbox", 0, 0, UI::Box::Vertical),
     vbox_name_(&hbox_, UI::PanelStyle::kWui, "name_vbox", 0, 0, UI::Box::Vertical),
     vbox_team_(&hbox_, UI::PanelStyle::kWui, "team_vbox", 0, 0, UI::Box::Vertical),
     vbox_status_(&hbox_, UI::PanelStyle::kWui, "status_vbox", 0, 0, UI::Box::Vertical),
     vbox_action_(&hbox_, UI::PanelStyle::kWui, "action_vbox", 0, 0, UI::Box::Vertical),
     actions_hbox_(
        &diplomacy_box_, UI::PanelStyle::kWui, "actions_hbox", 0, 0, UI::Box::Horizontal),
     actions_vbox_descr_(
        &actions_hbox_, UI::PanelStyle::kWui, "actions_description_vbox", 0, 0, UI::Box::Vertical),
     actions_vbox_yes_(
        &actions_hbox_, UI::PanelStyle::kWui, "actions_yes_vbox", 0, 0, UI::Box::Vertical),
     actions_vbox_no_(
        &actions_hbox_, UI::PanelStyle::kWui, "actions_no_vbox", 0, 0, UI::Box::Vertical) {

	const bool rtl = UI::g_fh->fontset()->is_rtl();
	const bool show_all_players =
	   iplayer_ == nullptr || iplayer_->player().see_all() || iplayer_->omnipotent();
	iterate_players_existing_const(
	   p, igbase_.egbase().map().get_nrplayers(), igbase_.egbase(), player) {
		if (iplayer_ != nullptr && p != iplayer_->player_number() && !show_all_players &&
		    player->is_hidden_from_general_statistics()) {
			continue;
		}

		UI::Icon* icon_flag = new UI::Icon(
		   &vbox_flag_, UI::PanelStyle::kWui, format("flag_icon_%u", p), 0, 0, kRowSize, kRowSize,
		   THREADSAFE_T(const Image*, const Image* (*)(const RGBColor&, const std::string&),
		                playercolor_image, player->get_playercolor(),
		                "images/players/genstats_player.png"));
		UI::Icon* icon_team =
		   new UI::Icon(&vbox_team_, UI::PanelStyle::kWui, format("team_icon_%u", p), 0, 0, kRowSize,
		                kRowSize, nullptr);
		UI::Textarea* txt_name = new UI::Textarea(
		   &vbox_name_, UI::PanelStyle::kWui, format("name_%u", p), UI::FontStyle::kWuiLabel,
		   player->get_name(), UI::mirror_alignment(UI::Align::kLeft, rtl));
		UI::Textarea* txt_status = new UI::Textarea(&vbox_status_, UI::PanelStyle::kWui,
		                                            format("status_%u", p), UI::FontStyle::kWuiLabel,
		                                            "", UI::mirror_alignment(UI::Align::kRight, rtl));

		icon_team->set_handle_mouse(true);
		icon_flag->set_handle_mouse(true);
		icon_flag->set_tooltip(format(_("Player %u"), static_cast<unsigned>(p)));
		txt_status->set_fixed_width(230);

		if (iplayer_ != nullptr) {
			UI::Button* b1 = nullptr;
			UI::Button* b2 = nullptr;
			UI::Box* buttonsbox = new UI::Box(&vbox_action_, UI::PanelStyle::kWui,
			                                  format("buttons_box_%u", p), 0, 0, UI::Box::Horizontal);
			if (p == iplayer_->player_number()) {
				b1 = new UI::Button(buttonsbox, "leave", 0, 0, kButtonWidth, kRowSize,
				                    UI::ButtonStyle::kWuiSecondary, _("Leave"),
				                    _("Leave your current team and become teamless"));
				b2 = new UI::Button(buttonsbox, "resign", 0, 0, kButtonWidth, kRowSize,
				                    UI::ButtonStyle::kWuiSecondary, _("Resign"),
				                    _("Give up and become a spectator"));
				b1->sigclicked.connect([this]() {
					iplayer_->game().send_player_diplomacy(iplayer_->player_number(),
					                                       Widelands::DiplomacyAction::kLeaveTeam,
					                                       0 /* ignored */);
				});
				b2->sigclicked.connect([this]() {
					if ((SDL_GetModState() & KMOD_CTRL) != 0) {
						iplayer_->game().send_player_diplomacy(iplayer_->player_number(),
						                                       Widelands::DiplomacyAction::kResign,
						                                       0 /* ignored */);
					} else {
						show_resign_confirm(*iplayer_);
					}
				});
			} else {
				b1 = new UI::Button(buttonsbox, "join", 0, 0, kButtonWidth, kRowSize,
				                    UI::ButtonStyle::kWuiSecondary, _("Join"),
				                    _("Request to join this player’s team"));
				b2 = new UI::Button(buttonsbox, "invite", 0, 0, kButtonWidth, kRowSize,
				                    UI::ButtonStyle::kWuiSecondary, _("Invite"),
				                    _("Invite this player to join your team"));
				b1->sigclicked.connect([this, p]() {
					iplayer_->game().send_player_diplomacy(
					   iplayer_->player_number(), Widelands::DiplomacyAction::kJoin, p);
				});
				b2->sigclicked.connect([this, p]() {
					iplayer_->game().send_player_diplomacy(
					   iplayer_->player_number(), Widelands::DiplomacyAction::kInvite, p);
				});
			}

			buttonsbox->add(b1, UI::Box::Resizing::kExpandBoth);
			buttonsbox->add_space(kSpacing);
			buttonsbox->add(b2, UI::Box::Resizing::kExpandBoth);

			vbox_action_.add(buttonsbox, UI::Box::Resizing::kExpandBoth);
			diplomacy_buttons_[p] = {b1, b2};
		}

		vbox_name_.add_space(kSpacing);
		vbox_status_.add_space(kSpacing);
		vbox_name_.add(
		   txt_name, UI::Box::Resizing::kFillSpace, UI::mirror_alignment(UI::Align::kLeft, rtl));
		vbox_status_.add(
		   txt_status, UI::Box::Resizing::kFillSpace, UI::mirror_alignment(UI::Align::kLeft, rtl));
		vbox_team_.add(icon_team, UI::Box::Resizing::kExpandBoth);
		vbox_flag_.add(icon_flag, UI::Box::Resizing::kExpandBoth);

		diplomacy_teams_[p] = icon_team;
		diplomacy_status_[p] = txt_status;

		for (UI::Box* b : {&vbox_flag_, &vbox_name_, &vbox_team_, &vbox_status_, &vbox_action_}) {
			b->add_space(kSpacing);
			b->add_inf_space();
		}
	}

	hbox_.add(rtl ? &vbox_action_ : &vbox_flag_, UI::Box::Resizing::kExpandBoth);
	hbox_.add_space(kSpacing);
	hbox_.add(rtl ? &vbox_team_ : &vbox_name_, UI::Box::Resizing::kExpandBoth);
	hbox_.add_space(kSpacing);
	hbox_.add(&vbox_status_, UI::Box::Resizing::kExpandBoth);
	hbox_.add_space(kSpacing);
	hbox_.add(rtl ? &vbox_name_ : &vbox_team_, UI::Box::Resizing::kExpandBoth);
	hbox_.add_space(kSpacing);
	hbox_.add(rtl ? &vbox_flag_ : &vbox_action_, UI::Box::Resizing::kExpandBoth);

	actions_hbox_.add(&actions_vbox_descr_, UI::Box::Resizing::kExpandBoth);
	actions_hbox_.add_space(kSpacing);
	actions_hbox_.add(&actions_vbox_yes_, UI::Box::Resizing::kFullSize);
	actions_hbox_.add_space(kSpacing);
	actions_hbox_.add(&actions_vbox_no_, UI::Box::Resizing::kFullSize);

	diplomacy_box_.add(&hbox_, UI::Box::Resizing::kExpandBoth);
	diplomacy_box_.add_space(kSpacing);
	diplomacy_box_.add(&actions_hbox_, UI::Box::Resizing::kExpandBoth);

	update_diplomacy_details();
	set_center_panel(&diplomacy_box_);
	if (get_usedefaultpos()) {
		center_to_parent();
	}
	initialization_complete();
}

/** Recompute the data in the Diplomacy tab and update which buttons are enabled. */
void GameDiplomacyMenu::update_diplomacy_details() {
	vbox_action_.set_visible(igbase_.game().diplomacy_allowed() && iplayer_ != nullptr);

	for (auto& pair : diplomacy_teams_) {
		const unsigned t = igbase_.egbase().player(pair.first).team_number();
		pair.second->set_icon(
		   THREADSAFE_T(const Image*, const Image* (*)(const RGBColor&, const std::string&),
		                playercolor_image, kTeamColors[t /* it's 1-based, 0 means No Team */],
		                t == 0 ? "images/players/no_team.png" : "images/players/team.png"));
		pair.second->set_tooltip(t == 0 ? _("No team") : format(_("Team %u"), t));
	}

	std::set<Widelands::PlayerNumber> players_with_result;
	for (auto& pair : diplomacy_status_) {
		const Widelands::PlayerEndStatus* p =
		   igbase_.egbase().player_manager()->get_player_end_status(pair.first);
		if (p == nullptr) {
			continue;
		}

		players_with_result.insert(p->player);
		std::string str;
		switch (p->result) {
		case Widelands::PlayerEndResult::kWon:
			str = format(_("Won at %s"), gametimestring(p->time.get()));
			break;
		case Widelands::PlayerEndResult::kLost:
		case Widelands::PlayerEndResult::kEliminated:
			str = format(_("Lost at %s"), gametimestring(p->time.get()));
			break;
		case Widelands::PlayerEndResult::kResigned:
			str = format(_("Resigned at %s"), gametimestring(p->time.get()));
			break;
		default:
			break;
		}
		pair.second->set_text(str);
	}

	const unsigned own_team =
	   iplayer_ == nullptr ? 0 : iplayer_->egbase().player(iplayer_->player_number()).team_number();
	const bool has_result =
	   iplayer_ != nullptr && players_with_result.count(iplayer_->player_number()) > 0;
	for (auto& pair : diplomacy_buttons_) {
		if (has_result || players_with_result.count(pair.first) > 0) {
			// Ignore players who are no longer playing
			pair.second.first->set_enabled(false);
			pair.second.second->set_enabled(false);
		} else if (iplayer_ != nullptr && pair.first == iplayer_->player_number()) {
			// Self. The two buttons are "Leave team" and "Resign".
			pair.second.first->set_enabled(own_team > 0);
		} else {
			// Other player. The two buttons are "Request to join" and "Invite to join".
			const bool can_join =
			   (own_team != igbase_.egbase().player(pair.first).team_number()) || own_team == 0;
			pair.second.first->set_enabled(can_join);
			pair.second.second->set_enabled(can_join);
		}
	}

	/* Recreate the list of pending actions if needed. */
	std::list<Widelands::Game::PendingDiplomacyAction> new_list =
	   igbase_.game().pending_diplomacy_actions();
	if (cached_diplomacy_actions_ == new_list) {
		return;
	}
	cached_diplomacy_actions_ = new_list;

	for (UI::Box* box : {&actions_vbox_descr_, &actions_vbox_yes_, &actions_vbox_no_}) {
		while (box->get_first_child() != nullptr) {
			delete box->get_first_child();
		}
		box->clear();
	}
	unsigned index = 0;
	for (const Widelands::Game::PendingDiplomacyAction& pda : cached_diplomacy_actions_) {
		if (index > 0) {
			actions_vbox_descr_.add_space(kSpacing);
			actions_vbox_no_.add_space(kSpacing);
			actions_vbox_yes_.add_space(kSpacing);
		}

		std::string descr;
		std::string approve_string;
		std::string deny_string;
		switch (pda.action) {
		case Widelands::DiplomacyAction::kJoin:
			descr = format(_("%1$s has requested to join the team of %2$s."),
			               igbase_.egbase().player(pda.sender).get_name(),
			               igbase_.egbase().player(pda.other).get_name());
			approve_string = _("Approve");
			deny_string = _("Reject");
			break;
		case Widelands::DiplomacyAction::kInvite:
			descr = format(_("%1$s has invited %2$s to join their team."),
			               igbase_.egbase().player(pda.sender).get_name(),
			               igbase_.egbase().player(pda.other).get_name());
			approve_string = _("Accept");
			deny_string = _("Refuse");
			break;
		default:
			NEVER_HERE();
		}

		actions_vbox_descr_.add_inf_space();
		actions_vbox_descr_.add(new UI::Textarea(&actions_vbox_descr_, UI::PanelStyle::kWui,
		                                         format("description_%u", index),
		                                         UI::FontStyle::kWuiInfoPanelParagraph, descr),
		                        UI::Box::Resizing::kFullSize);
		actions_vbox_descr_.add_inf_space();
		if (iplayer_ != nullptr && iplayer_->player_number() == pda.sender) {
			UI::Button* b =
			   new UI::Button(&actions_vbox_no_, format("retract_%u", index), 0, 0, kRowSize, kRowSize,
			                  UI::ButtonStyle::kWuiSecondary,
			                  g_image_cache->get("images/wui/menu_abort.png"), _("Retract"));
			b->sigclicked.connect([this, pda]() {
				iplayer_->game().send_player_diplomacy(
				   pda.sender,
				   pda.action == Widelands::DiplomacyAction::kInvite ?
				      Widelands::DiplomacyAction::kRetractInvite :
				      Widelands::DiplomacyAction::kRetractJoin,
				   pda.other);
			});
			actions_vbox_no_.add(b);
			actions_vbox_yes_.add_space(kRowSize);
		} else if (iplayer_ != nullptr &&
		           iplayer_->player().may_approve_request(pda.action, pda.sender, pda.other)) {
			UI::Button* b1 =
			   new UI::Button(&actions_vbox_yes_, format("approve_%u", index), 0, 0, kRowSize,
			                  kRowSize, UI::ButtonStyle::kWuiSecondary,
			                  g_image_cache->get("images/wui/menu_okay.png"), approve_string);
			UI::Button* b2 =
			   new UI::Button(&actions_vbox_no_, format("reject_%u", index), 0, 0, kRowSize, kRowSize,
			                  UI::ButtonStyle::kWuiSecondary,
			                  g_image_cache->get("images/wui/menu_abort.png"), deny_string);
			b1->sigclicked.connect([this, pda]() {
				iplayer_->game().send_player_diplomacy(
				   pda.other,
				   pda.action == Widelands::DiplomacyAction::kInvite ?
				      Widelands::DiplomacyAction::kAcceptInvite :
				      Widelands::DiplomacyAction::kAcceptJoin,
				   pda.sender);
			});
			b2->sigclicked.connect([this, pda]() {
				iplayer_->game().send_player_diplomacy(
				   pda.other,
				   pda.action == Widelands::DiplomacyAction::kInvite ?
				      Widelands::DiplomacyAction::kRefuseInvite :
				      Widelands::DiplomacyAction::kRefuseJoin,
				   pda.sender);
			});
			actions_vbox_yes_.add(b1);
			actions_vbox_no_.add(b2);
		} else {
			actions_vbox_yes_.add_space(kRowSize);
			actions_vbox_no_.add_space(kRowSize);
		}

		++index;
	}
	initialization_complete();
}

void GameDiplomacyMenu::think() {
	update_diplomacy_details();
}

void GameDiplomacyMenu::draw(RenderTarget& rt) {
	UI::UniqueWindow::draw(rt);
	if (iplayer_ == nullptr) {
		return;
	}

	const auto it = diplomacy_buttons_.find(iplayer_->player_number());
	if (it != diplomacy_buttons_.end()) {
		UI::Panel* panel = it->second.first;
		const int h = panel->get_h();
		int y = 0;
		for (; panel != this; panel = panel->get_parent()) {
			y += panel->get_y();
		}
		rt.brighten_rect(Recti(0, y - kSpacing / 2, get_inner_w(), h + kSpacing), 32);
	}
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& GameDiplomacyMenu::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r = dynamic_cast<InteractiveGameBase&>(ib).diplomacy_;
			r.create();
			assert(r.window);
			GameDiplomacyMenu& m = dynamic_cast<GameDiplomacyMenu&>(*r.window);
			return m;
		}
		throw Widelands::UnhandledVersionError(
		   "Diplomacy Menu", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("diplomacy menu: %s", e.what());
	}
}
void GameDiplomacyMenu::save(FileWrite& fw, Widelands::MapObjectSaver& /* mos */) const {
	fw.unsigned_16(kCurrentPacketVersion);
	// Nothing to save currently.
}
