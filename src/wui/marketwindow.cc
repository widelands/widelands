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

#include <memory>

#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "ui_basic/box.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/infinite_spinner.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/textinput.h"
#include "wui/actionconfirm.h"
#include "wui/inputqueuedisplay.h"
#include "wui/interactive_player.h"
#include "wui/waresdisplay.h"

constexpr int kButtonSize = 34;
constexpr int kSpacing = 8;
constexpr int kMinBoxWidth = 250;

constexpr const char* kIconTabTradeNew = "images/wui/buildings/menu_tab_trade.png";
constexpr const char* kIconTabTradeProposals = "images/wui/buildings/menu_tab_trade_proposals.png";
constexpr const char* kIconTabTradeOffers = "images/wui/buildings/menu_tab_trade_offers.png";
constexpr const char* kIconMoveTrade = "images/wui/buildings/menu_tab_trade_offers.png";

constexpr Duration kUpdateTimeInGametimeMs(500);  //  half a second, gametime

static inline int32_t calc_max_box_height() {
	return g_gr->get_yres() - 7 * kButtonSize;
}

class NewTradeProposalBox : public UI::Box {
public:
	NewTradeProposalBox(UI::Panel& parent, InteractivePlayer& iplayer, Widelands::Market& market)
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
	              UI::PanelStyle::kWui,
	              _("Toggle indefinite trade"),
	              _("Batches:"),
	              1,
	              1,
	              Widelands::kMaxBatches),
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
		trade.num_batches = batches_.is_infinite() ? Widelands::kInfiniteTrade : batches_.get_value();
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
		int32_t count1 = 0;
		int32_t count2 = 0;
		for (const Widelands::WareAmount& w1 : b1) {
			if (w1.second < Widelands::kMaxWaresPerBatch) {
				count1 += w1.second;
			} else {  // integer overflow protection
				count1 = Widelands::kMaxWaresPerBatch + 1;
				break;
			}
			set1.insert(w1.first);
		}
		for (const Widelands::WareAmount& w2 : b2) {
			if (w2.second < Widelands::kMaxWaresPerBatch) {
				count2 += w2.second;
			} else {  // integer overflow protection
				count2 = Widelands::kMaxWaresPerBatch + 1;
				break;
			}
			if (set1.count(w2.first) != 0) {
				conflicts.emplace_back(
				   iplayer_.egbase().descriptions().get_ware_descr(w2.first)->descname());
			}
		}

		if (count1 > Widelands::kMaxWaresPerBatch || count2 > Widelands::kMaxWaresPerBatch) {
			ok_.set_enabled(false);
			ok_.set_tooltip(
			   format(ngettext("It is not allowed to exchange more than %d ware per batch.",
			                   "It is not allowed to exchange more than %d wares per batch.",
			                   Widelands::kMaxWaresPerBatch),
			          Widelands::kMaxWaresPerBatch));
			return;
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
	UI::InfiniteSpinner batches_;
	UI::Button ok_;
};

class TradeProposalsBox : public UI::Box {
public:
	TradeProposalsBox(UI::Panel& parent,
	                  MarketWindow& mw,
	                  InteractiveBase& ibase,
	                  const Widelands::Market& market)
	   : UI::Box(&parent, UI::PanelStyle::kWui, "proposed_trades", 0, 0, UI::Box::Vertical),
	     ibase_(ibase),
	     iplayer_(dynamic_cast<InteractivePlayer*>(&ibase_)),
	     window_(mw),
	     player_number_(market.owner().player_number()),
	     market_serial_(market.serial()) {
		set_min_desired_breadth(kMinBoxWidth);
		set_force_scrolling(true);
		set_max_size(get_max_x(), calc_max_box_height());

		trade_changed_subscriber_ = Notifications::subscribe<Widelands::NoteTradeChanged>(
		   [this](const Widelands::NoteTradeChanged& /*note*/) { needs_update_ = true; });

		update(true);
	}

	void think() override {
		UI::Box::think();

		update(needs_update_);
		needs_update_ = false;
	}

	inline void update_proposals_tooltip() {
		window_.update_proposals_tooltip(cached_trades_.size());
	}

private:
	void update(const bool always) {
		const std::vector<Widelands::TradeID> trades =
		   ibase_.game().find_trade_proposals(player_number_, market_serial_);
		if (!always && trades == cached_trades_) {
			return;
		}

		cached_trades_ = trades;
		delete_all_children();

		update_proposals_tooltip();

		if (trades.empty()) {
			UI::MultilineTextarea* txt = new UI::MultilineTextarea(
			   this, "text", 0, 0, 0, 0, UI::PanelStyle::kWui, std::string(),
			   UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
			   UI::MultilineTextarea::ScrollMode::kNoScrolling);
			txt->set_style(UI::FontStyle::kWuiInfoPanelParagraph);
			if (iplayer_ != nullptr) {
				txt->set_text(
				   _("There are no pending trade proposals at the moment. You can propose a new "
				     "trade from the first tab."));
			} else {
				txt->set_text(_("There are no pending trade proposals at the moment."));
			}
			add(txt, UI::Box::Resizing::kFullSize);
			add_space(3 * UI::Scrollbar::kSize);

		} else {
			const bool can_act = iplayer_ != nullptr && iplayer_->can_act(player_number_);

			for (Widelands::TradeID trade_id : trades) {
				const Widelands::TradeInstance& trade = ibase_.game().get_trade(trade_id);
				UI::Box* box = new UI::Box(this, UI::PanelStyle::kWui, format("proposal_%u", trade_id),
				                           0, 0, UI::Box::Horizontal);

				box->add(new UI::MultilineTextarea(
				            box, "description", 0, 0, 0, 0, UI::PanelStyle::kWui,
				            trade.format_richtext(trade_id, ibase_.egbase(), player_number_, can_act),
				            UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
				            UI::MultilineTextarea::ScrollMode::kNoScrolling),
				         UI::Box::Resizing::kExpandBoth);

				if (can_act) {
					UI::Button* cancel = new UI::Button(
					   box, "cancel", 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
					   g_image_cache->get("images/wui/menu_abort.png"), _("Retract this trade offer"));

					cancel->sigclicked.connect([this, trade_id]() {
						iplayer_->game().send_player_trade_action(
						   iplayer_->player_number(), trade_id, Widelands::TradeAction::kRetract, 0, 0);
					});

					UI::Dropdown<Widelands::Serial>* move = new UI::Dropdown<Widelands::Serial>(
					   box, "move", 0, 0, 100, 8, kButtonSize, _("Move this trade proposal…"),
					   UI::DropdownType::kPictorialMenu, UI::PanelStyle::kWui,
					   UI::ButtonStyle::kWuiSecondary);
					move->set_min_lineheight(kButtonSize);
					move->set_image(g_image_cache->get(kIconMoveTrade));

					std::vector<const Widelands::Market*> markets = iplayer_->player().get_markets();
					for (const Widelands::Market* candidate : markets) {
						if (candidate->serial() != market_serial_) {
							move->add(candidate->get_market_name(), candidate->serial(),
							          candidate->descr().icon(), false,
							          format_l(_("Move this trade proposal to %s"),
							                   candidate->get_market_name()));
						}
					}
					if (move->empty()) {
						move->set_enabled(false);
						move->set_tooltip(_("You have no markets you could move this trade to."));
					} else {
						move->selected.connect([this, move, trade_id]() {
							iplayer_->game().send_player_trade_action(
							   iplayer_->player_number(), trade_id, Widelands::TradeAction::kMove,
							   move->get_selected(), market_serial_);
						});
					}

					box->add_space(kSpacing);
					box->add(move, UI::Box::Resizing::kAlign, UI::Align::kTop);
					box->add_space(kSpacing);
					box->add(cancel, UI::Box::Resizing::kAlign, UI::Align::kTop);
				}

				add(box, UI::Box::Resizing::kExpandBoth);
			}
		}

		initialization_complete();
	}

	InteractiveBase& ibase_;
	InteractivePlayer* iplayer_;
	MarketWindow& window_;
	const Widelands::PlayerNumber player_number_;
	const Widelands::Serial market_serial_;
	std::vector<Widelands::TradeID> cached_trades_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteTradeChanged>>
	   trade_changed_subscriber_;
	bool needs_update_ = false;
};

class TradeOffersBox : public UI::Box {
public:
	TradeOffersBox(UI::Panel& parent,
	               MarketWindow& mw,
	               InteractivePlayer& iplayer,
	               const Widelands::Market& market)
	   : UI::Box(&parent, UI::PanelStyle::kWui, "offered_trades", 0, 0, UI::Box::Vertical),
	     iplayer_(iplayer),
	     window_(mw),
	     player_number_(market.owner().player_number()),
	     market_serial_(market.serial()),
	     market_coords_(market.get_position()) {
		set_min_desired_breadth(kMinBoxWidth);
		set_force_scrolling(true);
		set_max_size(get_max_x(), calc_max_box_height());

		trade_changed_subscriber_ = Notifications::subscribe<Widelands::NoteTradeChanged>(
		   [this](const Widelands::NoteTradeChanged& /*note*/) { needs_update_ = true; });

		update(true);
	}

	void think() override {
		UI::Box::think();

		update(needs_update_);
		needs_update_ = false;
	}

	inline void update_offers_tooltip() {
		window_.update_offers_tooltip(cached_trades_.size());
	}

private:
	void update(const bool always) {
		const std::vector<Widelands::TradeID> trades =
		   iplayer_.game().find_trade_offers(player_number_, market_coords_);
		if (!always && trades == cached_trades_) {
			return;
		}

		cached_trades_ = trades;
		delete_all_children();

		update_offers_tooltip();

		if (trades.empty()) {
			UI::MultilineTextarea* txt = new UI::MultilineTextarea(
			   this, "text", 0, 0, 0, 0, UI::PanelStyle::kWui, std::string(),
			   UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
			   UI::MultilineTextarea::ScrollMode::kNoScrolling);
			txt->set_style(UI::FontStyle::kWuiInfoPanelParagraph);
			txt->set_text(_("Nobody has offered you any trades at the moment."));
			add(txt, UI::Box::Resizing::kFullSize);
			add_space(3 * UI::Scrollbar::kSize);

		} else {
			for (Widelands::TradeID trade_id : trades) {
				const Widelands::TradeInstance& trade = iplayer_.game().get_trade(trade_id);
				UI::Box* box = new UI::Box(this, UI::PanelStyle::kWui, format("proposal_%u", trade_id),
				                           0, 0, UI::Box::Horizontal);

				box->add(new UI::MultilineTextarea(
				            box, "description", 0, 0, 0, 0, UI::PanelStyle::kWui,
				            trade.format_richtext(trade_id, iplayer_.egbase(), player_number_, true),
				            UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()),
				            UI::MultilineTextarea::ScrollMode::kNoScrolling),
				         UI::Box::Resizing::kExpandBoth);

				UI::Button* yes = new UI::Button(
				   box, "yes", 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
				   g_image_cache->get("images/wui/menu_okay.png"), _("Accept this trade offer"));
				UI::Button* no = new UI::Button(
				   box, "no", 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
				   g_image_cache->get("images/wui/menu_abort.png"), _("Reject this trade offer"));

				yes->sigclicked.connect([this, trade_id]() {
					iplayer_.game().send_player_trade_action(iplayer_.player_number(), trade_id,
					                                         Widelands::TradeAction::kAccept,
					                                         market_serial_, 0);
				});
				no->sigclicked.connect([this, trade_id]() {
					iplayer_.game().send_player_trade_action(
					   iplayer_.player_number(), trade_id, Widelands::TradeAction::kReject, 0, 0);
				});

				box->add_space(kSpacing);
				box->add(no, UI::Box::Resizing::kAlign, UI::Align::kTop);
				box->add_space(kSpacing);
				box->add(yes, UI::Box::Resizing::kAlign, UI::Align::kTop);

				add(box, UI::Box::Resizing::kExpandBoth);
			}
		}

		initialization_complete();
	}

	InteractivePlayer& iplayer_;
	MarketWindow& window_;
	const Widelands::PlayerNumber player_number_;
	const Widelands::Serial market_serial_;
	const Widelands::Coords market_coords_;
	std::vector<Widelands::TradeID> cached_trades_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteTradeChanged>>
	   trade_changed_subscriber_;
	bool needs_update_ = false;
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
	           UI::MultilineTextarea::ScrollMode::kNoScrolling),
	     extensions_box_(this, UI::PanelStyle::kWui, "extensions_box", 0, 0, UI::Box::Vertical) {
		ensure_box_can_hold_input_queues(*this);
		set_min_desired_breadth(kMinBoxWidth);

		add(&info_, UI::Box::Resizing::kExpandBoth);

		const Widelands::TradeInstance& trade_instance = ibase.game().get_trade(trade_id_);
		const Widelands::Market::TradeOrder& order = *market.trade_orders().at(trade_id_);
		InputQueueDisplay* iqd = new InputQueueDisplay(
		   this, ibase, market, *order.carriers_queue_, false, false, collapsed, trade_id_);
		input_queues_.push_back(iqd);
		add(iqd, UI::Box::Resizing::kFullSize);
		for (const auto& pair : order.wares_queues_) {
			iqd = new InputQueueDisplay(
			   this, ibase, market, *pair.second, false, true, collapsed, trade_id_);
			input_queues_.push_back(iqd);
			add_space(kSpacing);
			add(iqd, UI::Box::Resizing::kFullSize);
		}

		if (can_act) {
			action_box_ =
			   new UI::Box(this, UI::PanelStyle::kWui, "actions", 0, 0, UI::Box::Horizontal);

			button_pause_ = new UI::Button(action_box_, "toggle_pause", 0, 0, kButtonSize, kButtonSize,
			                               UI::ButtonStyle::kWuiSecondary, std::string());
			button_pause_->sigclicked.connect([this]() { toggle_pause_action(); });

			const bool can_extend = trade_instance.num_batches != Widelands::kInfiniteTrade;
			button_extend_ = new UI::Button(
			   action_box_, "extend", 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
			   g_image_cache->get("images/wui/buildings/menu_tab_trade.png"),
			   can_extend ?
			      format("<p>%s%s%s</p>",
			             g_style_manager->font_style(UI::FontStyle::kWuiTooltipHeader)
			                .as_font_tag(_("Propose extending this trade")),
			             as_listitem(format(ngettext("Hold down Ctrl to extend it by %d batch",
			                                         "Hold down Ctrl to extend it by %d batches",
			                                         trade_instance.num_batches),
			                                trade_instance.num_batches),
			                         UI::FontStyle::kWuiTooltip),
			             as_listitem(_("Hold down Shift to extend the trade indefinitely"),
			                         UI::FontStyle::kWuiTooltip)) :
			      _("Propose extending this trade"));
			if (can_extend) {
				button_extend_->sigclicked.connect([this]() { propose_extending_trade(); });
			} else {
				button_extend_->set_enabled(false);
			}

			UI::Button* cancel = new UI::Button(
			   action_box_, "cancel", 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
			   g_image_cache->get("images/wui/menu_abort.png"), _("Cancel this trade"));
			cancel->sigclicked.connect([this]() {
				upcast(InteractivePlayer, ipl, &ibase_);
				assert(ipl != nullptr);
				Widelands::Game& game = ipl->game();
				if ((SDL_GetModState() & KMOD_CTRL) != 0) {
					game.send_player_trade_action(
					   ipl->player_number(), trade_id_, Widelands::TradeAction::kCancel, 0, 0);
				} else {
					show_cancel_trade_confirm(*ipl, trade_id_);
				}
			});

			dropdown_move_ = new UI::Dropdown<Widelands::Serial>(
			   action_box_, "move", 0, 0, 100, 8, kButtonSize, _("Move this trade…"),
			   UI::DropdownType::kPictorialMenu, UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary);
			dropdown_move_->set_desired_size(kButtonSize, kButtonSize);
			dropdown_move_->set_min_lineheight(kButtonSize);
			dropdown_move_->set_image(
			   g_image_cache->get("images/wui/buildings/menu_tab_trade_offers.png"));

			std::multimap<uint32_t, const Widelands::Market*> markets =
			   market.owner().get_markets(order.other_side.get(ibase.egbase())->get_position());
			for (const auto& pair : markets) {
				if (pair.second != &market) {
					const bool is_closest = pair.first == markets.begin()->first;
					dropdown_move_->add(is_closest ?
					                       format_l(_("%s (closest)"), pair.second->get_market_name()) :
					                       pair.second->get_market_name(),
					                    pair.second->serial(), pair.second->descr().icon(), false,
					                    format_l(is_closest ? _("Move this trade to %s (closest)") :
					                                          _("Move this trade to %s"),
					                             pair.second->get_market_name()));
				}
			}
			if (dropdown_move_->empty()) {
				dropdown_move_->set_enabled(false);
				dropdown_move_->set_tooltip(_("You have no markets you could move this trade to."));
			}

			dropdown_move_->selected.connect([this]() {
				upcast(InteractivePlayer, ipl, &ibase_);
				assert(ipl != nullptr);
				ipl->game().send_player_trade_action(ipl->player_number(), trade_id_,
				                                     Widelands::TradeAction::kMove,
				                                     dropdown_move_->get_selected(), market_.serial());
			});

			action_box_->add_inf_space();
			action_box_->add(button_pause_, UI::Box::Resizing::kFullSize);
			action_box_->add_inf_space();
			action_box_->add(dropdown_move_, UI::Box::Resizing::kFullSize);
			action_box_->add_inf_space();
			action_box_->add(button_extend_, UI::Box::Resizing::kFullSize);
			action_box_->add_inf_space();
			action_box_->add(cancel, UI::Box::Resizing::kFullSize);
			action_box_->add_inf_space();

			add_space(kSpacing);
			add(action_box_, UI::Box::Resizing::kFullSize);
		}

		add_space(kSpacing);
		add(&extensions_box_, UI::Box::Resizing::kFullSize);

		rebuild_extensions();

		trade_changed_subscriber_ = Notifications::subscribe<Widelands::NoteTradeChanged>(
		   [this](const Widelands::NoteTradeChanged& note) {
			   if (note.id == trade_id_) {
				   rebuild_extensions();
			   }
		   });

		update_paused(market);
		think();
	}

	void think() override {
		UI::Box::think();

		if (nextupdate_ > ibase_.egbase().get_gametime()) {
			return;
		}

		nextupdate_ = ibase_.egbase().get_gametime() + kUpdateTimeInGametimeMs;
		MutexLock m(MutexLock::ID::kObjects);

		Widelands::Market* own_market = market_.get(ibase_.egbase());
		if (own_market == nullptr) {
			return;
		}

		const Widelands::TradeInstance& agreement = ibase_.game().get_trade(trade_id_);
		info_.set_text(agreement.format_richtext(
		   trade_id_, ibase_.egbase(), own_market->owner().player_number(), can_act_));
		update_paused(*own_market);
	}

private:
	void rebuild_extensions() {
		extensions_box_.delete_all_children();
		extensions_box_.set_desired_size(0, 0);
		set_desired_size(0, 0);

		MutexLock m(MutexLock::ID::kObjects);
		if (const Widelands::Market* market = market_.get(ibase_.egbase()); market != nullptr) {
			for (const Widelands::TradeExtension& te : ibase_.game().find_trade_extensions(
			        trade_id_, market->owner().player_number(), false)) {
				UI::Box* box = new UI::Box(&extensions_box_, UI::PanelStyle::kWui, "extension_offer", 0,
				                           0, UI::Box::Horizontal);

				box->add(
				   new UI::Textarea(
				      box, UI::PanelStyle::kWui, "description", UI::FontStyle::kWuiInfoPanelHeading,
				      te.batches == Widelands::kInfiniteTrade ?
				         format(_("%1$s proposed to extend this trade indefinitely."),
				                ibase_.egbase().player(te.proposer).get_name()) :
				         format(
				            ngettext("%1$s proposed to extend this trade by %2$d batch.",
				                     "%1$s proposed to extend this trade by %2$d batches.", te.batches),
				            ibase_.egbase().player(te.proposer).get_name(), te.batches),
				      UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl())),
				   UI::Box::Resizing::kFillSpace, UI::Align::kCenter);

				if (can_act_) {
					UI::Button* reject = new UI::Button(box, "reject", 0, 0, kButtonSize, kButtonSize,
					                                    UI::ButtonStyle::kWuiSecondary,
					                                    g_image_cache->get("images/wui/menu_abort.png"),
					                                    _("Reject this trade extension proposal"));
					UI::Button* accept = new UI::Button(box, "accept", 0, 0, kButtonSize, kButtonSize,
					                                    UI::ButtonStyle::kWuiSecondary,
					                                    g_image_cache->get("images/wui/menu_okay.png"),
					                                    _("Accept this trade extension proposal"));

					reject->sigclicked.connect([this, te]() {
						upcast(InteractivePlayer, ipl, &ibase_);
						assert(ipl != nullptr);
						ipl->game().send_player_extend_trade(
						   ipl->player_number(), trade_id_, Widelands::TradeAction::kReject, te.batches);
					});
					accept->sigclicked.connect([this, te]() {
						upcast(InteractivePlayer, ipl, &ibase_);
						assert(ipl != nullptr);
						ipl->game().send_player_extend_trade(
						   ipl->player_number(), trade_id_, Widelands::TradeAction::kAccept, te.batches);
					});

					box->add_space(kSpacing);
					box->add(reject);
					box->add_space(kSpacing);
					box->add(accept);
				}

				extensions_box_.add(box, UI::Box::Resizing::kFullSize);
				extensions_box_.add_space(kSpacing);
			}

			for (const Widelands::TradeExtension& te : ibase_.game().find_trade_extensions(
			        trade_id_, market->owner().player_number(), true)) {
				UI::Box* box = new UI::Box(&extensions_box_, UI::PanelStyle::kWui, "extension_proposal",
				                           0, 0, UI::Box::Horizontal);

				UI::Textarea* description = new UI::Textarea(
				   box, UI::PanelStyle::kWui, "description", UI::FontStyle::kWuiInfoPanelHeading,
				   std::string(),
				   UI::mirror_alignment(UI::Align::kLeft, UI::g_fh->fontset()->is_rtl()));
				box->add(description, UI::Box::Resizing::kFillSpace, UI::Align::kCenter);

				if (can_act_) {
					if (te.batches != Widelands::kInfiniteTrade) {
						description->set_text(format(
						   ngettext("You proposed to extend this trade by %d batch.",
						            "You proposed to extend this trade by %d batches.", te.batches),
						   te.batches));
					} else {
						description->set_text(_("You proposed to extend this trade indefinitely."));
					}

					UI::Button* retract = new UI::Button(box, "retract", 0, 0, kButtonSize, kButtonSize,
					                                     UI::ButtonStyle::kWuiSecondary,
					                                     g_image_cache->get("images/wui/menu_abort.png"),
					                                     _("Retract this trade extension proposal"));

					retract->sigclicked.connect([this, te]() {
						upcast(InteractivePlayer, ipl, &ibase_);
						assert(ipl != nullptr);
						ipl->game().send_player_extend_trade(
						   ipl->player_number(), trade_id_, Widelands::TradeAction::kRetract, te.batches);
					});

					box->add_space(kSpacing);
					box->add(retract);

				} else {
					if (te.batches != Widelands::kInfiniteTrade) {
						description->set_text(format(
						   ngettext("%1$s proposed to extend this trade by %2$d batch.",
						            "%1$s proposed to extend this trade by %2$d batches.", te.batches),
						   market->owner().get_name(), te.batches));
					} else {
						description->set_text(
						   format(_("%1$s proposed to extend this trade indefinitely."),
						          market->owner().get_name()));
					}
				}

				extensions_box_.add(box, UI::Box::Resizing::kFullSize);
				extensions_box_.add_space(kSpacing);
			}
		}

		layout();
		extensions_box_.initialization_complete();
	}

	void update_paused(const Widelands::Market& market) {
		const bool paused = market.is_paused(trade_id_);

		if (button_pause_ != nullptr) {
			if (paused) {
				button_pause_->set_pic(g_image_cache->get("images/ui_basic/continue.png"));
				button_pause_->set_tooltip(_("Resume this paused trade. This requires all queues to be "
				                             "at their maximum capacity."));
			} else {
				button_pause_->set_pic(g_image_cache->get("images/ui_basic/stop.png"));
				button_pause_->set_tooltip(_("Pause this trade"));
			}
			button_pause_->set_perm_pressed(paused);
		}

		if (paused) {
			for (InputQueueDisplay* iqd : input_queues_) {
				iqd->unlock_desired_fill(false);
			}
		} else {
			std::string reason(
			   _("You need to pause the trade before you can change the queue capacity."));
			std::string title(_("Pause Trade?"));
			std::string body(_("Changing the capacity of the input queues will cause the trade to be "
			                   "paused. Pause the trade now?"));

			std::function<void()> unlock_fn = [this]() { toggle_pause_action(); };

			for (InputQueueDisplay* iqd : input_queues_) {
				iqd->lock_desired_fill(reason, title, body, unlock_fn);
			}
		}
	}

	void toggle_pause_action() {
		upcast(InteractivePlayer, ipl, &ibase_);
		assert(ipl != nullptr);
		Widelands::Game& game = ipl->game();

		MutexLock m(MutexLock::ID::kObjects);
		Widelands::Market* own_market = market_.get(game);
		if (own_market == nullptr) {
			return;
		}

		const bool is_paused = own_market->is_paused(trade_id_);
		if (is_paused && !own_market->can_resume(trade_id_)) {
			const auto it = own_market->trade_orders().find(trade_id_);
			if (it == own_market->trade_orders().end()) {
				return;
			}

			if ((SDL_GetModState() & KMOD_CTRL) == 0) {
				show_resume_trade_confirm(*ipl, *own_market, trade_id_);
				return;
			}

			for (const auto& pair : it->second->wares_queues_) {
				game.send_player_set_input_max_fill(*own_market, pair.second->get_index(),
				                                    pair.second->get_type(),
				                                    pair.second->get_max_size(), false, trade_id_);
			}
			game.send_player_set_input_max_fill(*own_market, it->second->carriers_queue_->get_index(),
			                                    it->second->carriers_queue_->get_type(),
			                                    it->second->carriers_queue_->get_max_size(), false,
			                                    trade_id_);
		}

		game.send_player_trade_action(
		   ipl->player_number(), trade_id_,
		   is_paused ? Widelands::TradeAction::kResume : Widelands::TradeAction::kPause,
		   own_market->serial(), 0);
	}

	void propose_extending_trade() {
		upcast(InteractivePlayer, ipl, &ibase_);
		assert(ipl != nullptr);
		Widelands::Game& game = ipl->game();

		MutexLock m(MutexLock::ID::kObjects);
		if (!game.has_trade(trade_id_)) {
			return;
		}

		if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
			return game.send_player_extend_trade(ipl->player_number(), trade_id_,
			                                     Widelands::TradeAction::kExtend,
			                                     Widelands::kInfiniteTrade);
		}
		if ((SDL_GetModState() & KMOD_CTRL) != 0) {
			return game.send_player_extend_trade(ipl->player_number(), trade_id_,
			                                     Widelands::TradeAction::kExtend,
			                                     game.get_trade(trade_id_).num_batches);
		}

		show_trade_extension_dialog(*ipl, trade_id_);
	}

	InteractiveBase& ibase_;
	Widelands::OPtr<Widelands::Market> market_;
	Widelands::TradeID trade_id_;
	bool can_act_;

	Time nextupdate_;
	UI::MultilineTextarea info_;
	UI::Box* action_box_{nullptr};
	UI::Button* button_pause_{nullptr};
	UI::Button* button_extend_{nullptr};
	UI::Dropdown<Widelands::Serial>* dropdown_move_{nullptr};
	std::vector<InputQueueDisplay*> input_queues_;
	UI::Box extensions_box_;

	std::unique_ptr<Notifications::Subscriber<Widelands::NoteTradeChanged>>
	   trade_changed_subscriber_;
};

MarketWindow::MarketWindow(InteractiveBase& parent,
                           BuildingWindow::Registry& reg,
                           Widelands::Market& m,
                           bool avoid_fastclick,
                           bool workarea_preview_wanted)
   : BuildingWindow(parent, reg, m, avoid_fastclick), market_(&m) {
	init(avoid_fastclick, workarea_preview_wanted);
}

void MarketWindow::update_proposals_tooltip(const uint32_t count) {
	if (tab_proposals_ != nullptr) {
		tab_proposals_->set_tooltip_text(format_l(_("Proposed Trades (%u)"), count));
	}
}

void MarketWindow::update_offers_tooltip(const uint32_t count) {
	if (tab_offers_ != nullptr) {
		tab_offers_->set_tooltip_text(format_l(_("Trade Offers (%u)"), count));
	}
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
			game->send_player_building_name(
			   mkt->owner().player_number(), mkt->serial(), name_field->get_text());
		} else {
			mkt->set_market_name(name_field->get_text());
		}
	});
	vbox.add(name_field, UI::Box::Resizing::kFullSize);
}

void MarketWindow::init(bool avoid_fastclick, bool workarea_preview_wanted) {
	tab_offers_ = nullptr;
	tab_proposals_ = nullptr;

	Widelands::Market* market = market_.get(ibase()->egbase());
	assert(market != nullptr);
	BuildingWindow::init(avoid_fastclick, workarea_preview_wanted);

	upcast(InteractivePlayer, iplayer, ibase());

	const bool can_act = iplayer != nullptr && iplayer->can_act(market->owner().player_number());

	if (can_act) {
		get_tabs()->add("propose", g_image_cache->get(kIconTabTradeNew),
		                new NewTradeProposalBox(*get_tabs(), *iplayer, *market), _("Propose Trade"));
	}

	TradeProposalsBox* tpb = new TradeProposalsBox(*get_tabs(), *this, *ibase(), *market);
	get_tabs()->add(
	   "proposals", g_image_cache->get(kIconTabTradeProposals), tpb, _("Proposed Trades"));
	tab_proposals_ = get_tabs()->tabs().back();
	tpb->update_proposals_tooltip();

	if (can_act) {
		TradeOffersBox* tob = new TradeOffersBox(*get_tabs(), *this, *iplayer, *market);
		get_tabs()->add("offers", g_image_cache->get(kIconTabTradeOffers), tob, _("Trade Offers"));
		tab_offers_ = get_tabs()->tabs().back();
		tob->update_offers_tooltip();
	}

	for (const auto& pair : market->trade_orders()) {
		get_tabs()->add(
		   format("trade_%u", pair.first),
		   ibase()->egbase().descriptions().get_ware_descr(pair.second->items.front().first)->icon(),
		   new TradeAgreementTab(
		      *get_tabs(), *ibase(), *market, pair.first, can_act, priority_collapsed()),
		   _("Active Trade"));
	}

	think();
	initialization_complete();
}
