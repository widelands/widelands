/*
 * Copyright (C) 2022 by the Widelands Development Team
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
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "wui/interactive_player.h"

constexpr int16_t kSpacing = 4;
constexpr int16_t kRowSize = 32;
constexpr int16_t kButtonWidth = 128;

DiplomacyConfirmWindow::DiplomacyConfirmWindow(InteractivePlayer& parent,
                                               const Widelands::Game::PendingDiplomacyAction& a)
   : UI::Window(
        &parent, UI::WindowStyle::kWui, "diplomacy_confirm", 0, 0, 300, 200, _("Diplomacy")),
     iplayer_(parent),
     action_(&a) {
	// The layout here is designed to mimic the ActionConfirm dialog.
	UI::Box* box = new UI::Box(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	UI::Box* button_box = new UI::Box(box, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);

	UI::Button* okbtn = new UI::Button(button_box, "ok", 0, 0, 80, 34, UI::ButtonStyle::kWuiMenu,
	                                   g_image_cache->get("images/wui/menu_okay.png"), _("Accept"));
	UI::Button* cancelbtn =
	   new UI::Button(button_box, "reject", 0, 0, 80, 34, UI::ButtonStyle::kWuiMenu,
	                  g_image_cache->get("images/wui/menu_abort.png"), _("Reject"));

	okbtn->sigclicked.connect([this]() { ok(); });
	cancelbtn->sigclicked.connect([this]() { die(); });

	button_box->add(
	   UI::g_fh->fontset()->is_rtl() ? okbtn : cancelbtn, UI::Box::Resizing::kFillSpace);
	button_box->add_space(2 * kSpacing);
	button_box->add(
	   UI::g_fh->fontset()->is_rtl() ? cancelbtn : okbtn, UI::Box::Resizing::kFillSpace);
	box->add_inf_space();
	box->add(new UI::MultilineTextarea(
	            box, 0, 0, 100, 50, UI::PanelStyle::kWui,
	            format(action_->action == Widelands::DiplomacyAction::kInvite ?
                         _("%s has invited you to join their team.") :
                         _("%s wants to join your team."),
	                   iplayer_.egbase().get_safe_player(a.sender)->get_name()),
	            UI::Align::kCenter, UI::MultilineTextarea::ScrollMode::kNoScrolling),
	         UI::Box::Resizing::kExpandBoth);
	box->add_space(kSpacing);
	box->add(button_box, UI::Box::Resizing::kFullSize);

	set_center_panel(box);
	center_to_parent();
	initialization_complete();
}

bool DiplomacyConfirmWindow::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
			ok();
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

void DiplomacyConfirmWindow::ok() {
	iplayer_.game().send_player_diplomacy(action_->other,
	                                      action_->action == Widelands::DiplomacyAction::kInvite ?
                                            Widelands::DiplomacyAction::kAcceptInvite :
                                            Widelands::DiplomacyAction::kAcceptJoin,
	                                      action_->sender);

	action_ = nullptr;
	die();
}

void DiplomacyConfirmWindow::die() {
	if (action_ != nullptr) {
		iplayer_.game().send_player_diplomacy(action_->other,
		                                      action_->action == Widelands::DiplomacyAction::kInvite ?
                                               Widelands::DiplomacyAction::kRefuseInvite :
                                               Widelands::DiplomacyAction::kRefuseJoin,
		                                      action_->sender);
	}

	UI::Window::die();
}

GameDiplomacyMenu::GameDiplomacyMenu(InteractivePlayer& parent,
                                     UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "diplomacy", &registry, 300, 200, _("Diplomacy")),
     iplayer_(parent),
     diplomacy_box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     hbox_(&diplomacy_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
     vbox_flag_(&hbox_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     vbox_name_(&hbox_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     vbox_team_(&hbox_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     vbox_status_(&hbox_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     vbox_action_(&hbox_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     diplomacy_info_(&diplomacy_box_,
                     0,
                     0,
                     100,
                     0,
                     UI::PanelStyle::kWui,
                     "",
                     UI::Align::kLeft,
                     UI::MultilineTextarea::ScrollMode::kNoScrolling) {

	const bool rtl = UI::g_fh->fontset()->is_rtl();
	const bool show_all_players = iplayer_.player().see_all() || iplayer_.omnipotent();
	iterate_players_existing_const(
	   p, iplayer_.egbase().map().get_nrplayers(), iplayer_.egbase(), player) {
		if (p != iplayer_.player_number() && !show_all_players &&
		    player->is_hidden_from_general_statistics()) {
			continue;
		}

		UI::Icon* icon_flag = new UI::Icon(
		   &vbox_flag_, UI::PanelStyle::kWui, 0, 0, kRowSize, kRowSize,
		   THREADSAFE_T(const Image*, const Image* (*)(const RGBColor&, const std::string&),
		                playercolor_image, player->get_playercolor(),
		                "images/players/genstats_player.png"));
		UI::Icon* icon_team =
		   new UI::Icon(&vbox_team_, UI::PanelStyle::kWui, 0, 0, kRowSize, kRowSize, nullptr);
		UI::Textarea* txt_name =
		   new UI::Textarea(&vbox_name_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel,
		                    player->get_name(), UI::mirror_alignment(UI::Align::kLeft, rtl));
		UI::Textarea* txt_status =
		   new UI::Textarea(&vbox_status_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, "",
		                    UI::mirror_alignment(UI::Align::kRight, rtl));

		icon_team->set_handle_mouse(true);
		icon_flag->set_handle_mouse(true);
		icon_flag->set_tooltip(format(_("Player %u"), static_cast<unsigned>(p)));
		txt_status->set_fixed_width(230);

		UI::Button* b1 = nullptr;
		UI::Button* b2 = nullptr;
		UI::Box* buttonsbox =
		   new UI::Box(&vbox_action_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
		if (p == iplayer_.player_number()) {
			b1 = new UI::Button(buttonsbox, "leave", 0, 0, kButtonWidth, kRowSize,
			                    UI::ButtonStyle::kWuiSecondary, _("Leave"),
			                    _("Leave your current team and become teamless"));
			b2 = new UI::Button(buttonsbox, "resign", 0, 0, kButtonWidth, kRowSize,
			                    UI::ButtonStyle::kWuiSecondary, _("Resign"),
			                    _("Give up and become a spectator"));
			b1->sigclicked.connect([this]() {
				iplayer_.game().send_player_diplomacy(
				   iplayer_.player_number(), Widelands::DiplomacyAction::kLeaveTeam, 0 /* ignored */);
			});
			b2->sigclicked.connect([this]() {
				iplayer_.game().send_player_diplomacy(
				   iplayer_.player_number(), Widelands::DiplomacyAction::kResign, 0 /* ignored */);
			});
		} else {
			b1 = new UI::Button(buttonsbox, "join", 0, 0, kButtonWidth, kRowSize,
			                    UI::ButtonStyle::kWuiSecondary, _("Join"),
			                    _("Request to join this player’s team"));
			b2 = new UI::Button(buttonsbox, "invite", 0, 0, kButtonWidth, kRowSize,
			                    UI::ButtonStyle::kWuiSecondary, _("Invite"),
			                    _("Invite this player to join your team"));
			b1->sigclicked.connect([this, p]() {
				iplayer_.game().send_player_diplomacy(
				   iplayer_.player_number(), Widelands::DiplomacyAction::kJoin, p);
			});
			b2->sigclicked.connect([this, p]() {
				iplayer_.game().send_player_diplomacy(
				   iplayer_.player_number(), Widelands::DiplomacyAction::kInvite, p);
			});
		}

		buttonsbox->add(b1, UI::Box::Resizing::kExpandBoth);
		buttonsbox->add_space(kSpacing);
		buttonsbox->add(b2, UI::Box::Resizing::kExpandBoth);

		vbox_name_.add_space(kSpacing);
		vbox_status_.add_space(kSpacing);
		vbox_name_.add(txt_name, UI::Box::Resizing::kFillSpace, UI::Align::kCenter);
		vbox_status_.add(txt_status, UI::Box::Resizing::kFillSpace, UI::Align::kCenter);
		vbox_team_.add(icon_team, UI::Box::Resizing::kExpandBoth);
		vbox_flag_.add(icon_flag, UI::Box::Resizing::kExpandBoth);
		vbox_action_.add(buttonsbox, UI::Box::Resizing::kExpandBoth);

		diplomacy_teams_[p] = icon_team;
		diplomacy_status_[p] = txt_status;
		diplomacy_buttons_[p] = {b1, b2};

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
	diplomacy_box_.add(&hbox_, UI::Box::Resizing::kExpandBoth);
	diplomacy_box_.add_space(kSpacing);
	diplomacy_box_.add(&diplomacy_info_, UI::Box::Resizing::kFullSize);

	update_diplomacy_details();
	set_center_panel(&diplomacy_box_);
	if (get_usedefaultpos()) {
		center_to_parent();
	}
	initialization_complete();
}

/** Recompute the data in the Diplomacy tab and update which buttons are enabled. */
void GameDiplomacyMenu::update_diplomacy_details() {
	vbox_action_.set_visible(iplayer_.game().diplomacy_allowed());

	for (auto& pair : diplomacy_teams_) {
		const unsigned t = iplayer_.egbase().player(pair.first).team_number();
		pair.second->set_icon(
		   THREADSAFE_T(const Image*, const Image* (*)(const RGBColor&, const std::string&),
		                playercolor_image, kTeamColors[t /* it's 1-based, 0 means No Team */],
		                t == 0 ? "images/players/no_team.png" : "images/players/team.png"));
		pair.second->set_tooltip(t == 0 ? _("No team") : format(_("Team %u"), t));
	}

	std::set<Widelands::PlayerNumber> players_with_result;
	for (auto& pair : diplomacy_status_) {
		const Widelands::PlayerEndStatus* p =
		   iplayer_.egbase().player_manager()->get_player_end_status(pair.first);
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

	const unsigned own_team = iplayer_.egbase().player(iplayer_.player_number()).team_number();
	const bool has_result = players_with_result.count(iplayer_.player_number()) > 0;
	for (auto& pair : diplomacy_buttons_) {
		if (has_result || players_with_result.count(pair.first) > 0) {
			// Ignore players who are no longer playing
			pair.second.first->set_enabled(false);
			pair.second.second->set_enabled(false);
		} else if (pair.first == iplayer_.player_number()) {
			// Self. The two buttons are "Leave team" and "Resign".
			pair.second.first->set_enabled(own_team > 0);
		} else {
			// Other player. The two buttons are "Request to join" and "Invite to join".
			const bool can_join =
			   (own_team != iplayer_.egbase().player(pair.first).team_number()) || own_team == 0;
			pair.second.first->set_enabled(can_join);
			pair.second.second->set_enabled(can_join);
		}
	}

	if (iplayer_.game().pending_diplomacy_actions().empty()) {
		diplomacy_info_.set_visible(false);
	} else {
		std::string text = "<rt><p>";
		text += g_style_manager->font_style(UI::FontStyle::kWuiInfoPanelHeading)
		           .as_font_tag(_("Pending Diplomacy Actions"));
		text += "</p>";
		for (const Widelands::Game::PendingDiplomacyAction& pda :
		     iplayer_.game().pending_diplomacy_actions()) {
			std::string descr;
			switch (pda.action) {
			case Widelands::DiplomacyAction::kJoin:
				descr = format(_("%1$s has requested to join the team of %2$s."),
				               iplayer_.egbase().player(pda.sender).get_name(),
				               iplayer_.egbase().player(pda.other).get_name());
				break;
			case Widelands::DiplomacyAction::kInvite:
				descr = format(_("%1$s has invited %2$s to join their team."),
				               iplayer_.egbase().player(pda.sender).get_name(),
				               iplayer_.egbase().player(pda.other).get_name());
				break;
			default:
				NEVER_HERE();
			}
			text += as_listitem(descr, UI::FontStyle::kWuiInfoPanelParagraph);
		}
		text += "</rt>";
		diplomacy_info_.set_text(text);
		diplomacy_info_.set_visible(true);
	}
}

void GameDiplomacyMenu::think() {
	update_diplomacy_details();
}

void GameDiplomacyMenu::draw(RenderTarget& rt) {
	UI::UniqueWindow::draw(rt);

	const auto it = diplomacy_buttons_.find(iplayer_.player_number());
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
			UI::UniqueWindow::Registry& r = dynamic_cast<InteractivePlayer&>(ib).diplomacy_;
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
