/*
 * Copyright (C) 2002-2026 by the Widelands Development Team
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

#include "wui/actionconfirm.h"

#include "base/macros.h"
#include "economy/economy.h"
#include "graphic/font_handler.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/infinite_spinner.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"
#include "wui/interactive_player.h"

constexpr int kPadding = 8;

struct ActionConfirm : public UI::Window {
	ActionConfirm(InteractivePlayer& parent,
	              const std::string& windowtitle,
	              const std::string& message,
	              Widelands::MapObject* map_object);

	InteractivePlayer& iaplayer() const {
		return dynamic_cast<InteractivePlayer&>(*get_parent());
	}

	virtual void ok() = 0;

protected:
	Widelands::ObjectPointer object_;
	UI::Box* custom_content_box_{nullptr};
};

/**
 * Confirmation dialog box for the bulldoze request for a building.
 */
struct BulldozeConfirm : public ActionConfirm {
	/**
	 * Create a BulldozeConfirm window.
	 * No further action is required by the caller: any action necessary to actually
	 * bulldoze the building if the user confirms is taken automatically.
	 *
	 * \param building this is the building that the confirmation dialog displays.
	 * \param todestroy if this is non-zero, then this immovable will be bulldozed
	 * instead of \p building if the user confirms the dialog.
	 * This is useful in the combination where \p todestroy is the base flag
	 * of \p building.
	 */
	BulldozeConfirm(InteractivePlayer& parent,
	                Widelands::Building& building,
	                Widelands::PlayerImmovable* todestroy = nullptr);

	void think() override;
	void ok() override;

private:
	Widelands::ObjectPointer todestroy_;
};

/**
 * Confirmation dialog box for the dismantle request for a building.
 */
struct DismantleConfirm : public ActionConfirm {
	DismantleConfirm(InteractivePlayer& parent, Widelands::Building& building);

	void think() override;
	void ok() override;

private:
	UI::Checkbox* checkbox_{nullptr};
};

/**
 * Confirmation dialog box for the enhance request for a building.
 */
struct EnhanceConfirm : public ActionConfirm {
	EnhanceConfirm(InteractivePlayer& parent,
	               Widelands::Building& building,
	               const Widelands::DescriptionIndex& id,
	               bool still_under_construction);

	void think() override;
	void ok() override;

private:
	// Do not make this a reference - it is a stack variable in the caller
	const Widelands::DescriptionIndex id_;
	bool still_under_construction_;

	UI::Checkbox* checkbox_{nullptr};
};

/**
 * Confirmation dialog box for the sink request for a ship.
 */
struct ShipSinkConfirm : public ActionConfirm {
	ShipSinkConfirm(InteractivePlayer& parent, Widelands::Ship& ship);

	void think() override;
	void ok() override;
};

/**
 * Confirmation dialog box for the cancel expedition request for a ship.
 */
struct ShipCancelExpeditionConfirm : public ActionConfirm {
	ShipCancelExpeditionConfirm(InteractivePlayer& parent, Widelands::Ship& ship);

	void think() override;
	void ok() override;
};

/**
 * Confirmation dialog box for giving up.
 */
struct ResignConfirm : public ActionConfirm {
	explicit ResignConfirm(InteractivePlayer& parent);

	void think() override;
	void ok() override;
};

/**
 * Confirmation dialog box for cancelling a trade.
 */
struct CancelTradeConfirm : public ActionConfirm {
	explicit CancelTradeConfirm(InteractivePlayer& parent, Widelands::TradeID trade_id);

	void think() override;
	void ok() override;

private:
	Widelands::TradeID trade_id_;
};

/**
 * Dialog box for extending a trade by a configurable number of batches.
 */
struct TradeExtensionDialog : public ActionConfirm {
	explicit TradeExtensionDialog(InteractivePlayer& parent, Widelands::TradeID trade_id);

	void think() override;
	void ok() override;

private:
	Widelands::TradeID trade_id_;

	UI::InfiniteSpinner batches_;
};

/**
 * Confirmation dialog box for unpausing a trade.
 */
struct UnpauseTradeConfirm : public ActionConfirm {
	explicit UnpauseTradeConfirm(InteractivePlayer& parent,
	                             Widelands::Market& market,
	                             Widelands::TradeID trade_id);

	void think() override;
	void ok() override;

private:
	Widelands::OPtr<Widelands::Market> market_;
	Widelands::TradeID trade_id_;
};

/**
 * Confirmation dialog box with an arbitrary title, body, and callback function.
 */
struct GenericCallbackConfirm : public ActionConfirm {
	explicit GenericCallbackConfirm(InteractivePlayer& parent,
	                                Widelands::MapObject* object,
	                                const std::string& title,
	                                const std::string& body,
	                                std::function<void()> callback);

	void think() override;
	void ok() override;

private:
	std::function<void()> callback_;
};

ActionConfirm::ActionConfirm(InteractivePlayer& parent,
                             const std::string& windowtitle,
                             const std::string& message,
                             Widelands::MapObject* map_object)
   : UI::Window(
        &parent, UI::WindowStyle::kWui, "building_action_confirm", 0, 0, 200, 120, windowtitle),
     object_(map_object) {
	UI::Box* wrapper_box =
	   new UI::Box(this, UI::PanelStyle::kWui, "wrapper_box", 0, 0, UI::Box::Horizontal);
	UI::Box* main_box =
	   new UI::Box(wrapper_box, UI::PanelStyle::kWui, "main_box", 0, 0, UI::Box::Vertical);
	UI::Box* button_box =
	   new UI::Box(main_box, UI::PanelStyle::kWui, "buttons_box", 0, 0, UI::Box::Horizontal);

	custom_content_box_ =
	   new UI::Box(main_box, UI::PanelStyle::kWui, "box", 0, 0, UI::Box::Horizontal);

	UI::MultilineTextarea* textarea = new UI::MultilineTextarea(
	   main_box, "message", 0, 0, 200, 74, UI::PanelStyle::kWui, message, UI::Align::kCenter,
	   UI::MultilineTextarea::ScrollMode::kNoScrolling);

	UI::Button* okbtn = new UI::Button(button_box, "ok", 0, 0, 80, 34, UI::ButtonStyle::kWuiMenu,
	                                   g_image_cache->get("images/wui/menu_okay.png"));
	okbtn->sigclicked.connect([this]() { ok(); });

	UI::Button* cancelbtn =
	   new UI::Button(button_box, "abort", 0, 0, 80, 34, UI::ButtonStyle::kWuiMenu,
	                  g_image_cache->get("images/wui/menu_abort.png"));
	cancelbtn->sigclicked.connect([this]() { die(); });

	button_box->add_space(kPadding);
	button_box->add(UI::g_fh->fontset()->is_rtl() ? okbtn : cancelbtn, UI::Box::Resizing::kAlign,
	                UI::Align::kCenter);
	button_box->add_space(2 * kPadding);
	button_box->add(UI::g_fh->fontset()->is_rtl() ? cancelbtn : okbtn, UI::Box::Resizing::kAlign,
	                UI::Align::kCenter);
	button_box->add_space(kPadding);

	main_box->add(textarea, UI::Box::Resizing::kExpandBoth);
	main_box->add_space(kPadding);
	main_box->add(custom_content_box_, UI::Box::Resizing::kExpandBoth);
	main_box->add_space(kPadding);
	main_box->add(button_box, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	main_box->add_space(kPadding);

	wrapper_box->add_space(kPadding);
	wrapper_box->add(main_box, UI::Box::Resizing::kExpandBoth);
	wrapper_box->add_space(kPadding);

	set_center_panel(wrapper_box);
	set_z(UI::Panel::ZOrder::kConfirmation);
	center_to_parent();
	cancelbtn->center_mouse();

	initialization_complete();
}

/*
===============
Create the panels for bulldoze confirmation.
===============
*/
BulldozeConfirm::BulldozeConfirm(InteractivePlayer& parent,
                                 Widelands::Building& building,
                                 Widelands::PlayerImmovable* todestroy)
   : ActionConfirm(parent,
                   _("Destroy building?"),
                   _("Do you really want to destroy this building?"),
                   &building),
     todestroy_(todestroy != nullptr ? todestroy : &building) {
	// Nothing special to do
}

/*
===============
Make sure the building still exists and can in fact be bulldozed.
===============
*/
void BulldozeConfirm::think() {
	const Widelands::EditorGameBase& egbase = iaplayer().egbase();
	upcast(Widelands::Building, building, object_.get(egbase));
	upcast(Widelands::PlayerImmovable, todestroy, todestroy_.get(egbase));

	if ((todestroy == nullptr) || (building == nullptr) ||
	    !iaplayer().can_act(building->owner().player_number()) ||
	    ((building->get_playercaps() & Widelands::Building::PCap_Bulldoze) == 0u)) {
		die();
	}
}

/**
 * The "Ok" button was clicked, so issue the CMD_BULLDOZE command for this building.
 */
void BulldozeConfirm::ok() {
	Widelands::Game& game = iaplayer().game();
	upcast(Widelands::Building, building, object_.get(game));
	upcast(Widelands::PlayerImmovable, todestroy, todestroy_.get(game));

	if ((todestroy != nullptr) && (building != nullptr) &&
	    iaplayer().can_act(building->owner().player_number()) &&
	    ((building->get_playercaps() & Widelands::Building::PCap_Bulldoze) != 0u)) {
		game.send_player_bulldoze(*todestroy, (SDL_GetModState() & KMOD_CTRL) != 0);
	}

	die();
}

static bool should_allow_preserving_wares(const Widelands::BuildingDescr& d) {
	switch (d.type()) {
	case Widelands::MapObjectType::WAREHOUSE:
	case Widelands::MapObjectType::MARKET:
		return true;
	case Widelands::MapObjectType::PRODUCTIONSITE:
	case Widelands::MapObjectType::TRAININGSITE:
		return !dynamic_cast<const Widelands::ProductionSiteDescr&>(d).input_wares().empty();
	default:
		return false;
	}
}

/*
===============
Create the panels for dismantle confirmation.
===============
*/
DismantleConfirm::DismantleConfirm(InteractivePlayer& parent, Widelands::Building& building)
   : ActionConfirm(parent,
                   _("Dismantle building?"),
                   _("Do you really want to dismantle this building?"),
                   &building) {
	if (should_allow_preserving_wares(building.descr())) {
		checkbox_ = new UI::Checkbox(custom_content_box_, UI::PanelStyle::kWui, "checkbox",
		                             Vector2i(0, 0), _("Preserve wares"));
		custom_content_box_->add(checkbox_, UI::Box::Resizing::kFullSize);

		checkbox_->set_tooltip(_("Any wares left in the building will be dropped out by the builder, "
		                         "increasing the dismantling time"));
		checkbox_->set_state(true);
	}

	initialization_complete();
}

/*
===============
Make sure the building still exists and can in fact be dismantled.
===============
*/
void DismantleConfirm::think() {
	const Widelands::EditorGameBase& egbase = iaplayer().egbase();
	upcast(Widelands::Building, building, object_.get(egbase));

	if ((building == nullptr) || !iaplayer().can_act(building->owner().player_number()) ||
	    ((building->get_playercaps() & Widelands::Building::PCap_Dismantle) == 0u)) {
		die();
	}
}

/**
 * The "Ok" button was clicked, so issue the CMD_DISMANTLEBUILDING command for this building.
 */
void DismantleConfirm::ok() {
	Widelands::Game& game = iaplayer().game();
	upcast(Widelands::Building, building, object_.get(game));
	upcast(Widelands::PlayerImmovable, todismantle, object_.get(game));

	if ((building != nullptr) && iaplayer().can_act(building->owner().player_number()) &&
	    ((building->get_playercaps() & Widelands::Building::PCap_Dismantle) != 0u)) {
		game.send_player_dismantle(*todismantle, (checkbox_ != nullptr) && checkbox_->get_state());
		iaplayer().hide_workarea(building->get_position(), false);
	}

	die();
}

/*
===============
Create the panels for enhancement confirmation.
===============
*/
EnhanceConfirm::EnhanceConfirm(InteractivePlayer& parent,
                               Widelands::Building& building,
                               const Widelands::DescriptionIndex& id,
                               bool still_under_construction)
   : ActionConfirm(
        parent,
        _("Enhance building?"),
        building.descr().type() == Widelands::MapObjectType::MILITARYSITE ?
           format(
              "%s\n\n%s",
              _("Do you really want to enhance this building?"),
              /** TRANSLATORS: Warning message when player wants to enhance a military building */
              _("Be careful if the enemy is near!")) :
           _("Do you really want to enhance this building?"),
        &building),
     id_(id),
     still_under_construction_(still_under_construction) {

	if (should_allow_preserving_wares(building.descr())) {
		checkbox_ = new UI::Checkbox(custom_content_box_, UI::PanelStyle::kWui, "checkbox",
		                             Vector2i(0, 0), _("Preserve wares"));
		custom_content_box_->add(checkbox_, UI::Box::Resizing::kFullSize);

		checkbox_->set_tooltip(_("Any wares left in the building will be dropped out by the builder, "
		                         "increasing the enhancing time"));
		checkbox_->set_state(true);
	}

	initialization_complete();
}

/*
===============
Make sure the building still exists and can in fact be enhanced.
===============
*/
void EnhanceConfirm::think() {
	const Widelands::EditorGameBase& egbase = iaplayer().egbase();
	upcast(Widelands::Building, building, object_.get(egbase));

	if ((building == nullptr) || !iaplayer().can_act(building->owner().player_number()) ||
	    (!still_under_construction_ &&
	     ((building->get_playercaps() & Widelands::Building::PCap_Enhancable) == 0u))) {
		die();
	}
}

/**
 * The "Ok" button was clicked, so issue the CMD_ENHANCEBUILDING command for this building.
 */
void EnhanceConfirm::ok() {
	Widelands::Game& game = iaplayer().game();

	if (still_under_construction_) {
		upcast(Widelands::ConstructionSite, cs, object_.get(game));
		if ((cs != nullptr) && iaplayer().can_act(cs->owner().player_number())) {
			game.send_player_enhance_building(
			   *cs, cs->building().enhancement(), (checkbox_ != nullptr) && checkbox_->get_state());
		}
	} else {
		upcast(Widelands::Building, building, object_.get(game));
		if ((building != nullptr) && iaplayer().can_act(building->owner().player_number()) &&
		    ((building->get_playercaps() & Widelands::Building::PCap_Enhancable) != 0u)) {
			game.send_player_enhance_building(
			   *building, id_, (checkbox_ != nullptr) && checkbox_->get_state());
		}
	}

	die();
}

/**
 * Create the panels for confirmation.
 */
ShipSinkConfirm::ShipSinkConfirm(InteractivePlayer& parent, Widelands::Ship& ship)
   : ActionConfirm(parent,
                   _("Sink the ship?"),
                   /** TRANSLATORS: %s is a ship name */
                   format(_("Do you really want to sink %s?"), ship.get_shipname()),
                   &ship) {
	// Nothing special to do
}

/**
 * Make sure the ship still exists.
 */
void ShipSinkConfirm::think() {
	const Widelands::EditorGameBase& egbase = iaplayer().egbase();
	upcast(Widelands::Ship, ship, object_.get(egbase));

	if ((ship == nullptr) || !iaplayer().can_act(ship->get_owner()->player_number())) {
		die();
	}
}

/**
 * The "Ok" button was clicked, so issue the CMD_ENHANCEBUILDING command for this building.
 */
void ShipSinkConfirm::ok() {
	Widelands::Game& game = iaplayer().game();
	upcast(Widelands::Ship, ship, object_.get(game));

	if ((ship != nullptr) && iaplayer().can_act(ship->get_owner()->player_number())) {
		game.send_player_sink_ship(*ship);
	}

	die();
}

/**
 * Create the panels for confirmation.
 */
ShipCancelExpeditionConfirm::ShipCancelExpeditionConfirm(InteractivePlayer& parent,
                                                         Widelands::Ship& ship)
   : ActionConfirm(parent,
                   _("Cancel expedition?"),
                   _("Do you really want to cancel the active expedition?"),
                   &ship) {
	// Nothing special to do
}

/**
 * Make sure the ship still exists and that it is still in expedition mode and not colonizing.
 */
void ShipCancelExpeditionConfirm::think() {
	const Widelands::EditorGameBase& egbase = iaplayer().egbase();
	upcast(Widelands::Ship, ship, object_.get(egbase));

	if ((ship == nullptr) || !iaplayer().can_act(ship->get_owner()->player_number()) ||
	    !ship->state_is_expedition()) {
		die();
	}
}

/**
 * The "Ok" button was clicked, so issue the command for this building.
 */
void ShipCancelExpeditionConfirm::ok() {
	Widelands::Game& game = iaplayer().game();
	upcast(Widelands::Ship, ship, object_.get(game));

	if (ship != nullptr && iaplayer().can_act(ship->get_owner()->player_number()) &&
	    ship->get_ship_state() != Widelands::ShipStates::kTransport &&
	    ship->get_ship_state() != Widelands::ShipStates::kExpeditionColonizing) {
		game.send_player_cancel_expedition_ship(*ship);
	}

	die();
}

/**
 * Create the panels for confirmation.
 */
ResignConfirm::ResignConfirm(InteractivePlayer& parent)
   : ActionConfirm(
        parent, _("Resign?"), _("Do you really want to give up and become a spectator?"), nullptr) {
	// Nothing special to do
}

/**
 * Make sure the player is still in the game.
 */
void ResignConfirm::think() {
	if (iaplayer().egbase().player_manager()->get_player_end_status(iaplayer().player_number()) !=
	    nullptr) {
		die();
	}
}

/**
 * The "Ok" button was clicked, so issue the command for resigning.
 */
void ResignConfirm::ok() {
	iaplayer().game().send_player_diplomacy(
	   iaplayer().player_number(), Widelands::DiplomacyAction::kResign, 0 /* ignored */);
	die();
}

/**
 * Create the panels for confirmation.
 */
CancelTradeConfirm::CancelTradeConfirm(InteractivePlayer& parent, Widelands::TradeID trade_id)
   : ActionConfirm(
        parent, _("Cancel?"), _("Do you really want to cancel this trade agreement?"), nullptr),
     trade_id_(trade_id) {
	// Nothing special to do
}

/**
 * Make sure the trade still exists.
 */
void CancelTradeConfirm::think() {
	if (!iaplayer().game().has_trade(trade_id_)) {
		die();
	}
}

/**
 * The "Ok" button was clicked, so issue the command for cancelling the trade.
 */
void CancelTradeConfirm::ok() {
	iaplayer().game().send_player_trade_action(
	   iaplayer().player_number(), trade_id_, Widelands::TradeAction::kCancel, 0, 0);
	die();
}

/**
 * Create the panels for configuration.
 */
TradeExtensionDialog::TradeExtensionDialog(InteractivePlayer& parent, Widelands::TradeID trade_id)
   : ActionConfirm(parent,
                   _("Extend Trade"),
                   _("Select by how many batches you want to extend this trade:"),
                   nullptr),
     trade_id_(trade_id),
     batches_(custom_content_box_,
              "batches",
              UI::PanelStyle::kWui,
              _("Toggle indefinite trade"),
              _("Batches:"),
              1,
              1,
              Widelands::kMaxBatches) {
	custom_content_box_->add(&batches_, UI::Box::Resizing::kFullSize);

	initialization_complete();
}

/**
 * Make sure the trade still exists.
 */
void TradeExtensionDialog::think() {
	if (!iaplayer().game().has_trade(trade_id_)) {
		die();
	}
}

/**
 * The "Ok" button was clicked, so issue the command for proposing to extend the trade.
 */
void TradeExtensionDialog::ok() {
	iaplayer().game().send_player_extend_trade(
	   iaplayer().player_number(), trade_id_, Widelands::TradeAction::kExtend,
	   batches_.is_infinite() ? Widelands::kInfiniteTrade : batches_.get_value());
	die();
}

/**
 * Create the panels for confirmation.
 */
UnpauseTradeConfirm::UnpauseTradeConfirm(InteractivePlayer& parent,
                                         Widelands::Market& market,
                                         Widelands::TradeID trade_id)
   : ActionConfirm(parent,
                   _("Unpause?"),
                   _("Do you really want to unpause this trade?\n\nDoing so will reset all queues "
                     "to their maximum capacity."),
                   &market),
     market_(&market),
     trade_id_(trade_id) {
	// Nothing special to do
}

/**
 * Make sure the trade still exists.
 */
void UnpauseTradeConfirm::think() {
	if (!iaplayer().game().has_trade(trade_id_)) {
		die();
	}
}

/**
 * The "Ok" button was clicked, so issue the command for resuming the trade.
 */
void UnpauseTradeConfirm::ok() {
	Widelands::Game& game = iaplayer().game();
	MutexLock m(MutexLock::ID::kObjects);

	if (game.has_trade(trade_id_)) {
		Widelands::Market* market = market_.get(game);
		assert(market != nullptr);
		const Widelands::Market::TradeOrder& order = *market->trade_orders().at(trade_id_);
		for (const auto& pair : order.wares_queues_) {
			game.send_player_set_input_max_fill(*market, pair.second->get_index(),
			                                    pair.second->get_type(), pair.second->get_max_size(),
			                                    false, trade_id_);
		}
		game.send_player_set_input_max_fill(*market, order.carriers_queue_->get_index(),
		                                    order.carriers_queue_->get_type(),
		                                    order.carriers_queue_->get_max_size(), false, trade_id_);
		game.send_player_trade_action(iaplayer().player_number(), trade_id_,
		                              Widelands::TradeAction::kResume, market->serial(), 0);
	}

	die();
}

/**
 * Create the panels for confirmation.
 */
GenericCallbackConfirm::GenericCallbackConfirm(InteractivePlayer& parent,
                                               Widelands::MapObject* object,
                                               const std::string& title,
                                               const std::string& body,
                                               std::function<void()> callback)
   : ActionConfirm(parent, title, body, object), callback_(callback) {
	// Nothing special to do
}

/**
 * Make sure the building still exists, if we have one.
 */
void GenericCallbackConfirm::think() {
	if (object_.is_set() && object_.get(iaplayer().egbase()) == nullptr) {
		die();
	}
}

/**
 * The "Ok" button was clicked, so invoke the callback function.
 */
void GenericCallbackConfirm::ok() {
	callback_();
	die();
}

/**
 * Create a BulldozeConfirm window.
 * No further action is required by the caller: any action necessary to actually
 * bulldoze the building if the user confirms is taken automatically.
 *
 * \param building this is the building that the confirmation dialog displays.
 * \param todestroy if this is non-zero, then this immovable will be bulldozed
 * instead of \p building if the user confirms the dialog.
 * This is useful in the combination where \p todestroy is the base flag
 * of \p building
 */
void show_bulldoze_confirm(InteractivePlayer& player,
                           Widelands::Building& building,
                           Widelands::PlayerImmovable* const todestroy) {
	new BulldozeConfirm(player, building, todestroy);
}

/**
 * Create a DismantleConfirm window.
 * No further action is required by the caller: any action necessary to actually
 * dismantle the building if the user confirms is taken automatically.
 *
 * \param building this is the building that the confirmation dialog displays.
 */
void show_dismantle_confirm(InteractivePlayer& player, Widelands::Building& building) {
	new DismantleConfirm(player, building);
}

/**
 * Create a EnhanceConfirm window.
 * No further action is required by the caller: any action necessary to actually
 * enhance the building if the user confirms is taken automatically.
 *
 * \param building this is the building that the confirmation dialog displays.
 * \param id building ID
 */
void show_enhance_confirm(InteractivePlayer& player,
                          Widelands::Building& building,
                          const Widelands::DescriptionIndex& id,
                          bool constructionsite) {
	new EnhanceConfirm(player, building, id, constructionsite);
}

/**
 * Create a ShipSinkConfirm window.
 * No further action is required by the caller: any action necessary to actually
 * sink the ship if the user confirms is taken automatically.
 *
 * \param ship this is the ship that the confirmation dialog displays.
 */
void show_ship_sink_confirm(InteractivePlayer& player, Widelands::Ship& ship) {
	new ShipSinkConfirm(player, ship);
}

/**
 * Create a ShipCancelExpeditionConfirm window.
 * No further action is required by the caller: any action necessary to actually
 * cancel the expedition if the user confirms is taken automatically.
 *
 * \param ship this is the ship that the confirmation dialog displays.
 */
void show_ship_cancel_expedition_confirm(InteractivePlayer& player, Widelands::Ship& ship) {
	new ShipCancelExpeditionConfirm(player, ship);
}

void show_resign_confirm(InteractivePlayer& player) {
	new ResignConfirm(player);
}

void show_cancel_trade_confirm(InteractivePlayer& player, Widelands::TradeID trade_id) {
	new CancelTradeConfirm(player, trade_id);
}

void show_trade_extension_dialog(InteractivePlayer& player, Widelands::TradeID trade_id) {
	new TradeExtensionDialog(player, trade_id);
}

void show_resume_trade_confirm(InteractivePlayer& player,
                               Widelands::Market& market,
                               Widelands::TradeID trade_id) {
	new UnpauseTradeConfirm(player, market, trade_id);
}

void show_generic_callback_confirm(InteractivePlayer& player,
                                   Widelands::MapObject* object,
                                   const std::string& title,
                                   const std::string& body,
                                   std::function<void()> callback) {
	new GenericCallbackConfirm(player, object, title, body, callback);
}
