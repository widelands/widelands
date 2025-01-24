/*
 * Copyright (C) 2024-2025 by the Widelands Development Team
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

#include "wui/marketwindow.h"

#include "graphic/font_handler.h"
#include "graphic/text_layout.h"
#include "ui_basic/box.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/textinput.h"
#include "wui/actionconfirm.h"
#include "wui/inputqueuedisplay.h"
#include "wui/interactive_player.h"
#include "wui/waresdisplay.h"

constexpr int kButtonSize = 34;
constexpr int kSpacing = 8;

constexpr const char* pic_tab_wares = "images/wui/buildings/menu_tab_wares.png";

constexpr Duration kUpdateTimeInGametimeMs(500);  //  half a second, gametime

class TradeProposalBox : public UI::Box {
public:
	TradeProposalBox(UI::Panel& parent, InteractivePlayer& iplayer, Widelands::Market& market)
	   : UI::Box(&parent, UI::PanelStyle::kWui, "propose_trade", 0, 0, UI::Box::Vertical),
	     iplayer_(iplayer),
	     market_(&market),
	     hbox_(this, UI::PanelStyle::kWui, "hbox", 0, 0, UI::Box::Horizontal),
	     buttons_box_(this, UI::PanelStyle::kWui, "buttons_box", 0, 0, UI::Box::Horizontal),
	     player_(this,
	             "player",
	             0,
	             0,
	             200,
	             8,
	             kButtonSize,
	             _("Player"),
	             UI::DropdownType::kTextual,
	             UI::PanelStyle::kWui,
	             UI::ButtonStyle::kWuiSecondary),
	     batches_(this,
	              "batches",
	              0,
	              0,
	              400,
	              250,
	              1,
	              1,
	              100,
	              UI::PanelStyle::kWui,
	              _("Batches:"),
	              UI::SpinBox::Units::kNone,
	              UI::SpinBox::Type::kBig),
	     ok_(this, "ok", 0, 0, 0, 0, UI::ButtonStyle::kWuiPrimary, _("Propose")) {
		set_size(400, 100);  // guard against SpinBox asserts

		iterate_players_existing(
		   number, iplayer.egbase().map().get_nrplayers(), iplayer.egbase(), player) {
			if (number != iplayer.player_number() && !player->is_hidden_from_general_statistics()) {
				player_.add(player->get_name(), player,
				            playercolor_image(player->get_playercolor(), "images/players/team.png"),
				            !player_.has_selection());
			}
		}

		player_.selected.connect([this]() { rebuild(); });
		ok_.sigclicked.connect([this]() { clicked_ok(); });

		hbox_.add(&TradeProposalWaresDisplay::create(
		             &hbox_, iplayer.player().tribe(), _("You offer:"), kSpacing, &offer_),
		          UI::Box::Resizing::kExpandBoth);
		hbox_.add_space(kSpacing);
		hbox_.add(&TradeProposalWaresDisplay::create(
		             &hbox_, iplayer.player().tribe(), _("You demand:"), kSpacing, &demand_),
		          UI::Box::Resizing::kExpandBoth);

		buttons_box_.add_space(kSpacing);
		UI::Button* b = new UI::Button(
		   &buttons_box_, "decrease_fast", 0, 0, 40, 28, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/ui_basic/scrollbar_down_fast.png"), _("Decrease amount by 10"));
		b->sigclicked.connect([this] { change(-10); });
		buttons_box_.add(b, UI::Box::Resizing::kExpandBoth);
		buttons_box_.add_space(kSpacing);
		b->set_repeating(true);
		b = new UI::Button(&buttons_box_, "decrease", 0, 0, 40, 28, UI::ButtonStyle::kWuiSecondary,
		                   g_image_cache->get("images/ui_basic/scrollbar_down.png"),
		                   _("Decrease amount"));
		b->sigclicked.connect([this] { change(-1); });
		buttons_box_.add(b, UI::Box::Resizing::kExpandBoth);
		buttons_box_.add_space(kSpacing);

		b->set_repeating(true);
		b = new UI::Button(&buttons_box_, "increase", 0, 0, 40, 28, UI::ButtonStyle::kWuiSecondary,
		                   g_image_cache->get("images/ui_basic/scrollbar_up.png"),
		                   _("Increase amount"));
		b->sigclicked.connect([this] { change(1); });
		buttons_box_.add(b, UI::Box::Resizing::kExpandBoth);
		buttons_box_.add_space(kSpacing);
		b->set_repeating(true);
		b = new UI::Button(
		   &buttons_box_, "increase_fast", 0, 0, 40, 28, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/ui_basic/scrollbar_up_fast.png"), _("Increase amount by 10"));
		b->sigclicked.connect([this] { change(10); });
		buttons_box_.add(b, UI::Box::Resizing::kExpandBoth);
		buttons_box_.add_space(kSpacing);
		b->set_repeating(true);

		add(&player_, UI::Box::Resizing::kFullSize);
		add_space(kSpacing);
		add(&hbox_, UI::Box::Resizing::kExpandBoth);
		add_space(kSpacing);
		add(&buttons_box_, UI::Box::Resizing::kExpandBoth);
		add_space(kSpacing);
		add(&batches_, UI::Box::Resizing::kFullSize);
		add_space(kSpacing);
		add(&ok_, UI::Box::Resizing::kFullSize);
		add_space(kSpacing);

		rebuild();
	}

	void think() override {
		UI::Box::think();
		update_ok_button();
	}

private:
	void clicked_ok() {
		if (!ok_.enabled()) {
			return;
		}

		Widelands::TradeInstance trade;
		trade.items_to_send = offer_->get_selection();
		trade.items_to_receive = demand_->get_selection();
		trade.num_batches = batches_.get_value();
		trade.initiator = market_;
		trade.sending_player = iplayer_.player_number();
		trade.receiving_player = player_.get_selected()->player_number();

		iplayer_.game().send_player_propose_trade(trade);

		offer_->set_zero();
		demand_->set_zero();
		update_ok_button();
	}

	void change(int delta) {
		offer_->change(delta);
		demand_->change(delta);
	}

	void rebuild() {
		offer_->set_other(player_.has_selection() ? &player_.get_selected()->tribe() : nullptr);
		demand_->set_other(player_.has_selection() ? &player_.get_selected()->tribe() : nullptr);

		update_ok_button();
	}

	void update_ok_button() {
		if (!player_.has_selection()) {
			ok_.set_enabled(false);
			ok_.set_tooltip(_("Please select the player you want to trade with."));
			return;
		}

		if (!offer_->anything_selected() || !demand_->anything_selected()) {
			ok_.set_enabled(false);
			ok_.set_tooltip(_("Please select the wares you want to send and receive."));
			return;
		}

		const Widelands::BillOfMaterials b1 = offer_->get_selection();
		const Widelands::BillOfMaterials b2 = demand_->get_selection();
		std::set<Widelands::DescriptionIndex> set1;
		std::vector<std::string> conflicts;
		for (const Widelands::WareAmount& w1 : b1) {
			set1.insert(w1.first);
		}
		for (const Widelands::WareAmount& w2 : b2) {
			if (set1.count(w2.first) != 0) {
				conflicts.emplace_back(
				   iplayer_.egbase().descriptions().get_ware_descr(w2.first)->descname());
			}
		}
		if (!conflicts.empty()) {
			ok_.set_enabled(false);
			ok_.set_tooltip(format(_("You cannot both send and receive the same ware type (%s)."),
			                       i18n::localize_list(conflicts, i18n::ConcatenateWith::AND)));
			return;
		}

		ok_.set_enabled(true);
		ok_.set_tooltip(_("Propose the configured trade to the selected player"));
	}

	InteractivePlayer& iplayer_;
	Widelands::OPtr<Widelands::Market> market_;

	UI::Box hbox_, buttons_box_;
	UI::Dropdown<Widelands::Player*> player_;
	TradeProposalWaresDisplay* offer_;
	TradeProposalWaresDisplay* demand_;
	UI::SpinBox batches_;
	UI::Button ok_;
};

class TradeAgreementTab : public UI::Box {
public:
	TradeAgreementTab(UI::Panel& parent,
	                  InteractiveBase& ibase,
	                  Widelands::Market& market,
	                  Widelands::TradeID trade_id,
	                  bool can_act,
	                  BuildingWindow::CollapsedState* collapsed)
	   : UI::Box(&parent,
	             UI::PanelStyle::kWui,
	             format("trade_agreement_%u", trade_id),
	             0,
	             0,
	             UI::Box::Vertical),
	     ibase_(ibase),
	     market_(&market),
	     trade_id_(trade_id),
	     can_act_(can_act),
	     nextupdate_(ibase.egbase().get_gametime()),
	     info_(this,
	           "info",
	           0,
	           0,
	           0,
	           0,
	           UI::PanelStyle::kWui,
	           std::string(),
	           UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
	           UI::MultilineTextarea::ScrollMode::kNoScrolling) {
		ensure_box_can_hold_input_queues(*this);
		set_min_desired_breadth(250);

		add(&info_, UI::Box::Resizing::kExpandBoth);
		add_space(kSpacing);

		const Widelands::Market::TradeOrder& order = market.trade_orders().at(trade_id_);
		// TODO(Nordfriese): Implement controls for those
		add(
		   new InputQueueDisplay(this, ibase, market, *order.carriers_queue_, true, false, collapsed),
		   UI::Box::Resizing::kFullSize);
		for (const auto& pair : order.wares_queues_) {
			add_space(kSpacing);
			add(new InputQueueDisplay(this, ibase, market, *pair.second, true, true, collapsed),
			    UI::Box::Resizing::kFullSize);
		}

		if (can_act) {
			UI::Button* cancel =
			   new UI::Button(this, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kWuiSecondary, _("Cancel"),
			                  _("Cancel this trade"));
			cancel->sigclicked.connect([this]() {
				upcast(InteractivePlayer, ipl, &ibase_);
				assert(ipl != nullptr);
				Widelands::Game& game = ipl->game();
				if ((SDL_GetModState() & KMOD_CTRL) != 0) {
					game.send_player_trade_action(
					   ipl->player_number(), trade_id_, Widelands::TradeAction::kCancel, 0);
				} else {
					show_cancel_trade_confirm(*ipl, trade_id_);
				}
			});
			add_space(kSpacing);
			add(cancel, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		}

		add_space(kSpacing);
		think();
	}

	void think() override {
		UI::Box::think();

		if (nextupdate_ > ibase_.egbase().get_gametime()) {
			return;
		}

		nextupdate_ = ibase_.egbase().get_gametime() + kUpdateTimeInGametimeMs;
		MutexLock m(MutexLock::ID::kObjects);

		Widelands::Market* market = market_.get(ibase_.egbase());
		if (market == nullptr) {
			return;
		}

		const auto trade = market->trade_orders().find(trade_id_);
		if (trade == market->trade_orders().end()) {
			return;
		}

		Widelands::Market* other_market = trade->second.other_side.get(ibase_.egbase());
		if (other_market == nullptr) {
			return;
		}
		const Widelands::TradeInstance& agreement = ibase_.game().get_trade(trade_id_);
		const bool is_receiver = agreement.initiator == other_market;

		std::string infotext("<rt><p>");
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelHeading,
		                        format_l(_("Trade with %s"), other_market->owner().get_name()));

		infotext += "</p><p>";
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelParagraph,
		   format_l(ngettext("%d batch total", "%d batches total", trade->second.initial_num_batches),
		            trade->second.initial_num_batches));

		infotext += "</p><p>";
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelParagraph,
		   format_l(ngettext("%d batch sent", "%d batches sent", trade->second.num_shipped_batches),
		            trade->second.num_shipped_batches));

		infotext += "</p><p>";
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelParagraph,
		   format_l(ngettext("%d batch remaining", "%d batches remaining",
		                     trade->second.initial_num_batches - trade->second.num_shipped_batches),
		            trade->second.initial_num_batches - trade->second.num_shipped_batches));

		infotext += "</p>";
		infotext += as_vspace(kSpacing);
		infotext += "<p>";
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelHeading, can_act_ ? _("You send:") : _("Player sends:"));
		for (const auto& pair : is_receiver ? agreement.items_to_receive : agreement.items_to_send) {
			infotext += as_listitem(
			   format_l(_("%1$i× %2$s"), pair.second,
			            ibase_.egbase().descriptions().get_ware_descr(pair.first)->descname()),
			   UI::FontStyle::kWuiInfoPanelParagraph);
		}

		infotext += "</p>";
		infotext += as_vspace(kSpacing);
		infotext += "<p>";
		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelHeading, can_act_ ? _("You receive:") : _("Player receives:"));
		for (const auto& pair : is_receiver ? agreement.items_to_send : agreement.items_to_receive) {
			infotext += as_listitem(
			   format_l(_("%1$i× %2$s"), pair.second,
			            ibase_.egbase().descriptions().get_ware_descr(pair.first)->descname()),
			   UI::FontStyle::kWuiInfoPanelParagraph);
		}

		infotext += "</p></rt>";
		info_.set_text(infotext);
	}

private:
	InteractiveBase& ibase_;
	Widelands::OPtr<Widelands::Market> market_;
	Widelands::TradeID trade_id_;
	bool can_act_;

	Time nextupdate_;
	UI::MultilineTextarea info_;
};

MarketWindow::MarketWindow(InteractiveBase& parent,
                           BuildingWindow::Registry& reg,
                           Widelands::Market& m,
                           bool avoid_fastclick,
                           bool workarea_preview_wanted)
   : BuildingWindow(parent, reg, m, avoid_fastclick), market_(&m) {
	init(avoid_fastclick, workarea_preview_wanted);
}

void MarketWindow::setup_name_field_editbox(UI::Box& vbox) {
	Widelands::Market* market = market_.get(ibase()->egbase());
	if (market == nullptr || !ibase()->can_act(market->owner().player_number())) {
		return BuildingWindow::setup_name_field_editbox(vbox);
	}

	UI::EditBox* name_field = new UI::EditBox(&vbox, "name", 0, 0, 0, UI::PanelStyle::kWui);
	name_field->set_text(market->get_market_name());
	name_field->changed.connect([this, name_field]() {
		Widelands::Market* mkt = market_.get(ibase()->egbase());
		if (mkt == nullptr) {
			return;
		}
		if (Widelands::Game* game = ibase()->get_game(); game != nullptr) {
			game->send_player_ship_port_name(
			   mkt->owner().player_number(), mkt->serial(), name_field->get_text());
		} else {
			mkt->set_market_name(name_field->get_text());
		}
	});
	vbox.add(name_field, UI::Box::Resizing::kFullSize);
}

void MarketWindow::init(bool avoid_fastclick, bool workarea_preview_wanted) {
	Widelands::Market* market = market_.get(ibase()->egbase());
	assert(market != nullptr);
	BuildingWindow::init(avoid_fastclick, workarea_preview_wanted);

	if (upcast(InteractivePlayer, iplayer, ibase());
	    iplayer != nullptr && iplayer->can_act(market->owner().player_number())) {
		get_tabs()->add("propose", g_image_cache->get(pic_tab_wares),
		                new TradeProposalBox(*get_tabs(), *iplayer, *market), _("Propose Trade"));
	}

	for (const auto& pair : market->trade_orders()) {
		get_tabs()->add(
		   format("trade_%u", pair.first),
		   ibase()->egbase().descriptions().get_ware_descr(pair.second.items.front().first)->icon(),
		   new TradeAgreementTab(*get_tabs(), *ibase(), *market, pair.first,
		                         ibase()->can_act(market->owner().player_number()),
		                         priority_collapsed()),
		   _("Propose Trade"));
	}

	think();
	initialization_complete();
}
