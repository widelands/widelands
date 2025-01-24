/*
 * Copyright (C) 2022-2025 by the Widelands Development Team
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
#include "graphic/text_layout.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "ui_basic/multilinetextarea.h"
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
        &actions_hbox_, UI::PanelStyle::kWui, "actions_no_vbox", 0, 0, UI::Box::Vertical),
     trades_tabs_(&diplomacy_box_, UI::TabPanelStyle::kWuiLight, "trades"),
     trades_box_offers_(
        &trades_tabs_, UI::PanelStyle::kWui, "trades_offers", 0, 0, UI::Box::Vertical),
     trades_box_proposed_(
        &trades_tabs_, UI::PanelStyle::kWui, "trades_proposed", 0, 0, UI::Box::Vertical),
     trades_box_active_(
        &trades_tabs_, UI::PanelStyle::kWui, "trades_active", 0, 0, UI::Box::Vertical) {

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

	trades_tabs_.add(
	   "active", "", &trades_box_offers_, _("Trade offers you have received from other players"));
	trades_tabs_.add(
	   "active", "", &trades_box_proposed_, _("Trades you have proposed to other players"));
	trades_tabs_.add("active", "", &trades_box_active_, _("Your active trade agreements"));

	diplomacy_box_.add(&hbox_, UI::Box::Resizing::kExpandBoth);
	diplomacy_box_.add_space(kSpacing);
	diplomacy_box_.add(&actions_hbox_, UI::Box::Resizing::kExpandBoth);

	if (iplayer_ != nullptr) {
		diplomacy_box_.add_space(kSpacing);
		diplomacy_box_.add(&trades_tabs_, UI::Box::Resizing::kExpandBoth);
	} else {
		trades_tabs_.set_visible(false);
	}

	update(true);
	set_center_panel(&diplomacy_box_);
	if (get_usedefaultpos()) {
		center_to_parent();
	}
	initialization_complete();
}

void GameDiplomacyMenu::update(bool always) {
	update_diplomacy_details();

	if (iplayer_ != nullptr) {
		update_trades_offers(always);
		update_trades_proposed(always);
		update_trades_active(always);
	}
}

void GameDiplomacyMenu::think() {
	update(false);
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
		box->delete_all_children();
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
}

void GameDiplomacyMenu::update_trades_offers(bool always) {
	const std::vector<Widelands::TradeID> trades =
	   iplayer_->game().find_trade_offers(iplayer_->player_number());

	if (!always && trades == cached_trades_offers_) {
		return;
	}

	trades_tabs_.tabs()[0]->set_title(format(_("Trade Offers (%u)"), trades.size()));

	cached_trades_offers_ = trades;
	trades_box_offers_.delete_all_children();

	if (trades.empty()) {
		UI::MultilineTextarea* txt = new UI::MultilineTextarea(
		   &trades_box_offers_, "text", 0, 0, 0, 0, UI::PanelStyle::kWui, std::string(),
		   UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
		   UI::MultilineTextarea::ScrollMode::kNoScrolling);
		txt->set_style(UI::FontStyle::kWuiInfoPanelParagraph);
		txt->set_text(_("Nobody has offered you any trades at the moment."));
		trades_box_offers_.add(txt, UI::Box::Resizing::kFullSize);
		return;
	}

	MutexLock m(MutexLock::ID::kObjects);
	for (Widelands::TradeID trade_id : trades) {
		const Widelands::TradeInstance& trade = iplayer_->game().get_trade(trade_id);
		const Widelands::Market* other_market = trade.initiator.get(iplayer_->egbase());
		if (other_market == nullptr) {
			continue;
		}

		UI::Box* box = new UI::Box(&trades_box_offers_, UI::PanelStyle::kWui,
		                           format("offer_%u", trade_id), 0, 0, UI::Box::Horizontal);
		UI::Box* buttons =
		   new UI::Box(box, UI::PanelStyle::kWui, "buttons", 0, 0, UI::Box::Horizontal);

		UI::Dropdown<Widelands::Serial>* select_market = new UI::Dropdown<Widelands::Serial>(
		   buttons, "select", 0, 0, 2 * kButtonWidth + kSpacing, 8, kRowSize, std::string(),
		   UI::DropdownType::kTextual, UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary);
		select_market->set_tooltip(_("Market to accept the trade at"));

		std::multimap<uint32_t, const Widelands::Market*> markets =
		   iplayer_->player().get_markets(other_market->get_position());
		for (auto it = markets.begin(); it != markets.end(); ++it) {
			const bool first = it == markets.begin();
			select_market->add(first ? format_l(_("%s (closest)"), it->second->get_market_name()) :
			                           it->second->get_market_name(),
			                   it->second->serial(), it->second->descr().icon(), first,
			                   format_l(ngettext("%1$s (distance: %2$u field)",
			                                     "%1$s (distance: %2$u fields)", it->first),
			                            it->second->get_market_name(), it->first));
		}

		UI::Button* yes = new UI::Button(
		   buttons, "yes", 0, 0, kRowSize, kRowSize, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/wui/menu_okay.png"), _("Accept this trade offer"));
		UI::Button* no = new UI::Button(
		   buttons, "no", 0, 0, kRowSize, kRowSize, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/wui/menu_abort.png"), _("Reject this trade offer"));

		if (select_market->empty()) {
			yes->set_enabled(false);
			yes->set_tooltip(_("Build a market to accept trade offers."));
		} else {
			yes->sigclicked.connect([this, trade_id, select_market]() {
				iplayer_->game().send_player_trade_action(iplayer_->player_number(), trade_id,
				                                          Widelands::TradeAction::kAccept,
				                                          select_market->get_selected());
			});
		}
		no->sigclicked.connect([this, trade_id]() {
			iplayer_->game().send_player_trade_action(
			   iplayer_->player_number(), trade_id, Widelands::TradeAction::kReject, 0);
		});

		buttons->add(select_market, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		buttons->add_space(kSpacing);
		buttons->add(no, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		buttons->add_space(kSpacing);
		buttons->add(yes, UI::Box::Resizing::kAlign, UI::Align::kCenter);

		std::string infotext("<rt><p>");
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelHeading,
		                        format_l(_("Trade offer from %s"), other_market->owner().get_name()));

		infotext += "</p><p>";
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelParagraph,
		   format_l(ngettext("%d batch", "%d batches", trade.num_batches), trade.num_batches));

		infotext += "</p>";
		infotext += as_vspace(kSpacing);
		infotext += "<p>";
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelHeading, _("You send:"));
		for (const auto& pair : trade.items_to_receive) {
			infotext += as_listitem(
			   format_l(_("%1$i× %2$s"), pair.second,
			            iplayer_->egbase().descriptions().get_ware_descr(pair.first)->descname()),
			   UI::FontStyle::kWuiInfoPanelParagraph);
		}

		infotext += "</p>";
		infotext += as_vspace(kSpacing);
		infotext += "<p>";
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelHeading, _("You receive:"));
		for (const auto& pair : trade.items_to_send) {
			infotext += as_listitem(
			   format_l(_("%1$i× %2$s"), pair.second,
			            iplayer_->egbase().descriptions().get_ware_descr(pair.first)->descname()),
			   UI::FontStyle::kWuiInfoPanelParagraph);
		}
		infotext += "</p></rt>";

		box->add(new UI::MultilineTextarea(
		            box, "description", 0, 0, 0, 0, UI::PanelStyle::kWui, infotext,
		            UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
		            UI::MultilineTextarea::ScrollMode::kNoScrolling),
		         UI::Box::Resizing::kExpandBoth);
		box->add_space(kSpacing);
		box->add(buttons, UI::Box::Resizing::kAlign, UI::Align::kTop);
		trades_box_offers_.add(box, UI::Box::Resizing::kExpandBoth);
		trades_box_offers_.add_space(kSpacing);
	}
}

void GameDiplomacyMenu::update_trades_proposed(bool always) {
	const std::vector<Widelands::TradeID> trades =
	   iplayer_->game().find_trade_proposals(iplayer_->player_number());
	if (!always && trades == cached_trades_proposed_) {
		return;
	}

	trades_tabs_.tabs()[1]->set_title(format(_("Proposed Trades (%u)"), trades.size()));

	cached_trades_proposed_ = trades;
	trades_box_proposed_.delete_all_children();

	if (trades.empty()) {
		UI::MultilineTextarea* txt = new UI::MultilineTextarea(
		   &trades_box_proposed_, "text", 0, 0, 0, 0, UI::PanelStyle::kWui, std::string(),
		   UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
		   UI::MultilineTextarea::ScrollMode::kNoScrolling);
		txt->set_style(UI::FontStyle::kWuiInfoPanelParagraph);
		txt->set_text(_("There are no pending trade proposals at the moment. You can propose a new "
		                "trade from a market."));
		trades_box_proposed_.add(txt, UI::Box::Resizing::kFullSize);
		return;
	}

	MutexLock m(MutexLock::ID::kObjects);
	for (Widelands::TradeID trade_id : trades) {
		const Widelands::TradeInstance& trade = iplayer_->game().get_trade(trade_id);
		const Widelands::Player& other_player = iplayer_->egbase().player(trade.receiving_player);
		const Widelands::Market* market = trade.initiator.get(iplayer_->egbase());
		if (market == nullptr) {
			continue;
		}

		UI::Box* box = new UI::Box(&trades_box_proposed_, UI::PanelStyle::kWui,
		                           format("proposal_%u", trade_id), 0, 0, UI::Box::Horizontal);
		UI::Box* buttons =
		   new UI::Box(box, UI::PanelStyle::kWui, "buttons", 0, 0, UI::Box::Horizontal);

		UI::Button* go_to = new UI::Button(
		   buttons, "go_to", 0, 0, kRowSize, kRowSize, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/wui/menus/goto.png"), _("Center view on this market"));
		UI::Button* cancel = new UI::Button(
		   buttons, "cancel", 0, 0, kRowSize, kRowSize, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/wui/menu_abort.png"), _("Retract this trade offer"));

		go_to->sigclicked.connect([this, market]() {
			iplayer_->map_view()->scroll_to_field(market->get_position(), MapView::Transition::Smooth);
		});
		cancel->sigclicked.connect([this, trade_id]() {
			iplayer_->game().send_player_trade_action(
			   iplayer_->player_number(), trade_id, Widelands::TradeAction::kRetract, 0);
		});

		buttons->add(cancel, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		buttons->add_space(kSpacing);
		buttons->add(go_to, UI::Box::Resizing::kAlign, UI::Align::kCenter);

		std::string infotext("<rt><p>");
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelHeading,
		   format_l(
		      /** TRANSLATORS: "From" is the market's name, and "to" is the receiving player's name */
		      _("Trade offer from %1$s to %2$s"), market->get_market_name(),
		      other_player.get_name()));

		infotext += "</p><p>";
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelParagraph,
		   format_l(ngettext("%d batch", "%d batches", trade.num_batches), trade.num_batches));

		infotext += "</p>";
		infotext += as_vspace(kSpacing);
		infotext += "<p>";
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelHeading, _("You send:"));
		for (const auto& pair : trade.items_to_send) {
			infotext += as_listitem(
			   format_l(_("%1$i× %2$s"), pair.second,
			            iplayer_->egbase().descriptions().get_ware_descr(pair.first)->descname()),
			   UI::FontStyle::kWuiInfoPanelParagraph);
		}

		infotext += "</p>";
		infotext += as_vspace(kSpacing);
		infotext += "<p>";
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelHeading, _("You receive:"));
		for (const auto& pair : trade.items_to_receive) {
			infotext += as_listitem(
			   format_l(_("%1$i× %2$s"), pair.second,
			            iplayer_->egbase().descriptions().get_ware_descr(pair.first)->descname()),
			   UI::FontStyle::kWuiInfoPanelParagraph);
		}
		infotext += "</p></rt>";

		box->add(new UI::MultilineTextarea(
		            box, "description", 0, 0, 0, 0, UI::PanelStyle::kWui, infotext,
		            UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
		            UI::MultilineTextarea::ScrollMode::kNoScrolling),
		         UI::Box::Resizing::kExpandBoth);
		box->add_space(kSpacing);
		box->add(buttons, UI::Box::Resizing::kAlign, UI::Align::kTop);
		trades_box_proposed_.add(box, UI::Box::Resizing::kExpandBoth);
		trades_box_proposed_.add_space(kSpacing);
	}
}

void GameDiplomacyMenu::update_trades_active(bool always) {
	const std::vector<Widelands::TradeID> trades =
	   iplayer_->game().find_active_trades(iplayer_->player_number());
	if (!always && trades == cached_trades_active_) {
		return;
	}

	trades_tabs_.tabs()[2]->set_title(format(_("Active Trades (%u)"), trades.size()));

	cached_trades_active_ = trades;
	trades_box_active_.delete_all_children();

	if (trades.empty()) {
		UI::MultilineTextarea* txt = new UI::MultilineTextarea(
		   &trades_box_active_, "text", 0, 0, 0, 0, UI::PanelStyle::kWui, std::string(),
		   UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
		   UI::MultilineTextarea::ScrollMode::kNoScrolling);
		txt->set_style(UI::FontStyle::kWuiInfoPanelParagraph);
		txt->set_text(_("There are no active trade agreements at the moment."));
		trades_box_active_.add(txt, UI::Box::Resizing::kFullSize);
		return;
	}

	MutexLock m(MutexLock::ID::kObjects);
	for (Widelands::TradeID trade_id : trades) {
		const Widelands::TradeInstance& trade = iplayer_->game().get_trade(trade_id);

		const Widelands::Market* own_market = trade.initiator.get(iplayer_->egbase());
		const Widelands::Market* other_market = trade.receiver.get(iplayer_->egbase());

		if (own_market == nullptr || other_market == nullptr) {
			continue;
		}

		bool is_receiver = false;
		if (trade.receiving_player == iplayer_->player_number()) {
			std::swap(own_market, other_market);
			is_receiver = true;
		}

		const int batches_sent = own_market->trade_orders().at(trade_id).num_shipped_batches;
		assert(batches_sent == other_market->trade_orders().at(trade_id).num_shipped_batches);

		UI::Box* box = new UI::Box(&trades_box_active_, UI::PanelStyle::kWui,
		                           format("active_%u", trade_id), 0, 0, UI::Box::Horizontal);
		UI::Box* buttons =
		   new UI::Box(box, UI::PanelStyle::kWui, "buttons", 0, 0, UI::Box::Horizontal);

		UI::Button* go_to = new UI::Button(
		   buttons, "go_to", 0, 0, kRowSize, kRowSize, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/wui/menus/goto.png"), _("Center view on this market"));
		UI::Button* cancel = new UI::Button(
		   buttons, "cancel", 0, 0, kRowSize, kRowSize, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/wui/menu_abort.png"), _("Cancel this trade agreement"));

		go_to->sigclicked.connect([this, own_market]() {
			iplayer_->map_view()->scroll_to_field(
			   own_market->get_position(), MapView::Transition::Smooth);
		});
		cancel->sigclicked.connect([this, trade_id]() {
			iplayer_->game().send_player_trade_action(
			   iplayer_->player_number(), trade_id, Widelands::TradeAction::kCancel, 0);
		});

		buttons->add(cancel, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		buttons->add_space(kSpacing);
		buttons->add(go_to, UI::Box::Resizing::kAlign, UI::Align::kCenter);

		std::string infotext("<rt><p>");
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelHeading,
		   format_l(
		      /** TRANSLATORS: "At" is the market's name, and "with" is the receiving player's name */
		      _("Trade agreement at %1$s with %2$s"), own_market->get_market_name(),
		      other_market->owner().get_name()));

		infotext += "</p><p>";
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelParagraph,
		   format_l(ngettext("%d batch", "%d batches", trade.num_batches), trade.num_batches));

		infotext += "</p><p>";
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelParagraph,
		   format_l(ngettext("%d batch sent", "%d batches sent", batches_sent), batches_sent));

		infotext += "</p><p>";
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelParagraph,
		                        format_l(ngettext("%d batch remaining", "%d batches remaining",
		                                          trade.num_batches - batches_sent),
		                                 trade.num_batches - batches_sent));

		infotext += "</p>";
		infotext += as_vspace(kSpacing);
		infotext += "<p>";
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelHeading, _("You send:"));
		for (const auto& pair : is_receiver ? trade.items_to_receive : trade.items_to_send) {
			infotext += as_listitem(
			   format_l(_("%1$i× %2$s"), pair.second,
			            iplayer_->egbase().descriptions().get_ware_descr(pair.first)->descname()),
			   UI::FontStyle::kWuiInfoPanelParagraph);
		}

		infotext += "</p>";
		infotext += as_vspace(kSpacing);
		infotext += "<p>";
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelHeading, _("You receive:"));
		for (const auto& pair : is_receiver ? trade.items_to_send : trade.items_to_receive) {
			infotext += as_listitem(
			   format_l(_("%1$i× %2$s"), pair.second,
			            iplayer_->egbase().descriptions().get_ware_descr(pair.first)->descname()),
			   UI::FontStyle::kWuiInfoPanelParagraph);
		}
		infotext += "</p></rt>";

		box->add(new UI::MultilineTextarea(
		            box, "description", 0, 0, 0, 0, UI::PanelStyle::kWui, infotext,
		            UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
		            UI::MultilineTextarea::ScrollMode::kNoScrolling),
		         UI::Box::Resizing::kExpandBoth);
		box->add_space(kSpacing);
		box->add(buttons, UI::Box::Resizing::kAlign, UI::Align::kTop);
		trades_box_active_.add(box, UI::Box::Resizing::kExpandBoth);
		trades_box_active_.add_space(kSpacing);
	}
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
