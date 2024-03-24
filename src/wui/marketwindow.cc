/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#include "ui_basic/box.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/spinbox.h"
#include "wui/interactive_player.h"
#include "wui/waresdisplay.h"

constexpr int kButtonSize = 34;
constexpr int kSpacing = 8;

constexpr const char* pic_tab_wares = "images/wui/buildings/menu_tab_wares.png";

class TradeProposalBox : public UI::Box {
public:
	TradeProposalBox(UI::Panel& parent, InteractivePlayer& iplayer, Widelands::Market& market)
		: UI::Box(&parent, UI::PanelStyle::kWui, "propose_trade", 0, 0, UI::Box::Vertical),
		iplayer_(iplayer),
		market_(&market),
		hbox_(this, UI::PanelStyle::kWui, "hbox", 0, 0, UI::Box::Horizontal),
		player_(this, "player", 0, 0, 200, 8, kButtonSize, _("Player"), UI::DropdownType::kTextual, UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary),
		batches_(this, "batches", 0, 0, 400, 250, 10, 1, 100, UI::PanelStyle::kWui, _("Batches:"), UI::SpinBox::Units::kNone, UI::SpinBox::Type::kBig),
		ok_(this, "ok", 0, 0, 0, 0, UI::ButtonStyle::kWuiPrimary, _("Propose"), _("Propose the configured trade to the selected player"))
	{
		set_size(400, 100);  // guard against SpinBox asserts

		iterate_players_existing(number, iplayer.egbase().map().get_nrplayers(), iplayer.egbase(), player) {
			if (number != iplayer.player_number() && !player->is_hidden_from_general_statistics()) {
				player_.add(player->get_name(), player, playercolor_image(player->get_playercolor(), "images/players/team.png"), !player_.has_selection());
			}
		}

		ok_.set_size(ok_.get_w(), std::max(ok_.get_h(), kButtonSize));
		player_.selected.connect([this]() { rebuild(); });
		ok_.sigclicked.connect([this]() { clicked_ok(); });

		hbox_.add(&TradeProposalWaresDisplay::create(&hbox_, iplayer.player().tribe(), _("You offer:"), kSpacing, &offer_), UI::Box::Resizing::kExpandBoth);
		hbox_.add_space(kSpacing);
		hbox_.add(&TradeProposalWaresDisplay::create(&hbox_, iplayer.player().tribe(), _("You demand:"), kSpacing, &demand_), UI::Box::Resizing::kExpandBoth);

		add(&player_, UI::Box::Resizing::kFullSize);
		add_space(kSpacing);
		add(&hbox_, UI::Box::Resizing::kExpandBoth);
		add_space(kSpacing);
		add(&batches_, UI::Box::Resizing::kFullSize);
		add_space(kSpacing);
		add(&ok_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

		rebuild();
	}

	void think() override {
		UI::Box::think();
		ok_.set_enabled(player_.has_selection() && offer_->anything_selected() && demand_->anything_selected());
	}

private:
	void clicked_ok() {
		if (!ok_.enabled()) {
			return;
		}

		Widelands::Trade trade;
		trade.items_to_send = offer_->get_selection();
		trade.items_to_receive = demand_->get_selection();
		trade.num_batches = batches_.get_value();
		trade.initiator = market_.serial();
		trade.receiver = 0;  // NOCOM let the recipient choose the market where to receive the trade

		iplayer_.game().send_player_propose_trade(trade);

		offer_->set_zero();
		demand_->set_zero();
		ok_.set_enabled(false);
	}

	void rebuild() {
		offer_->set_other(player_.has_selection() ? &player_.get_selected()->tribe() : nullptr);
		demand_->set_other(player_.has_selection() ? &player_.get_selected()->tribe() : nullptr);

		ok_.set_enabled(player_.has_selection() && offer_->anything_selected() && demand_->anything_selected());
	}

	InteractivePlayer& iplayer_;
	Widelands::OPtr<Widelands::Market> market_;

	UI::Box hbox_;
	UI::Dropdown<Widelands::Player*> player_;
	TradeProposalWaresDisplay* offer_;
	TradeProposalWaresDisplay* demand_;
	UI::SpinBox batches_;
	UI::Button ok_;
};

MarketWindow::MarketWindow(InteractiveBase& parent,
                                       BuildingWindow::Registry& reg,
                                       Widelands::Market& m,
                                       bool avoid_fastclick,
                                       bool workarea_preview_wanted)
   : BuildingWindow(parent, reg, m, avoid_fastclick), market_(&m) {
	init(avoid_fastclick, workarea_preview_wanted);
}

void MarketWindow::init(bool avoid_fastclick, bool workarea_preview_wanted) {
	Widelands::Market* market = market_.get(ibase()->egbase());
	assert(market != nullptr);
	BuildingWindow::init(avoid_fastclick, workarea_preview_wanted);

	if (upcast(InteractivePlayer, iplayer, ibase())) {
		get_tabs()->add("propose", g_image_cache->get(pic_tab_wares),
			            new TradeProposalBox(*get_tabs(), *iplayer, *market), _("Propose Trade"));
	}

	// NOCOM add tabs for ongoing trades



	think();
	initialization_complete();
}
