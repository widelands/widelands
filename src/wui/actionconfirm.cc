/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "wui/actionconfirm.h"

#include <boost/format.hpp>

#include "base/macros.h"
#include "economy/economy.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"
#include "wui/interactive_player.h"

struct ActionConfirm : public UI::Window {
	ActionConfirm(InteractivePlayer& parent,
	              const std::string& windowtitle,
	              const std::string& message,
	              Widelands::MapObject& map_object);

	InteractivePlayer& iaplayer() const {
		return dynamic_cast<InteractivePlayer&>(*get_parent());
	}

	virtual void think() = 0;
	virtual void ok() = 0;

protected:
	Widelands::ObjectPointer object_;
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
};

/**
 * Confirmation dialog box for the enhance request for a building.
 */
struct EnhanceConfirm : public ActionConfirm {
	EnhanceConfirm(InteractivePlayer& parent,
	               Widelands::Building& building,
	               const Widelands::DescriptionIndex& id);

	void think() override;
	void ok() override;

private:
	// Do not make this a reference - it is a stack variable in the caller
	const Widelands::DescriptionIndex id_;
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

ActionConfirm::ActionConfirm(InteractivePlayer& parent,
                             const std::string& windowtitle,
                             const std::string& message,
                             Widelands::MapObject& map_object)
   : UI::Window(&parent, "building_action_confirm", 0, 0, 200, 120, windowtitle),
     object_(&map_object) {
	const int padding = 6;
	UI::Box* main_box = new UI::Box(this, padding, padding, UI::Box::Vertical);
	UI::Box* button_box = new UI::Box(main_box, 0, 0, UI::Box::Horizontal);

	UI::MultilineTextarea* textarea =
	   new UI::MultilineTextarea(main_box, 0, 0, 200, 74, message, UI::Align::kCenter,
	                             g_gr->images().get("images/ui_basic/but1.png"),
	                             UI::MultilineTextarea::ScrollMode::kNoScrolling);
	textarea->force_new_renderer();

	UI::Button* okbtn =
	   new UI::Button(button_box, "ok", 0, 0, 80, 34, g_gr->images().get("images/ui_basic/but4.png"),
	                  g_gr->images().get("images/wui/menu_okay.png"));
	okbtn->sigclicked.connect(boost::bind(&ActionConfirm::ok, this));

	UI::Button* cancelbtn = new UI::Button(button_box, "abort", 0, 0, 80, 34,
	                                       g_gr->images().get("images/ui_basic/but4.png"),
	                                       g_gr->images().get("images/wui/menu_abort.png"));
	cancelbtn->sigclicked.connect(boost::bind(&ActionConfirm::die, this));

	button_box->add(
	   UI::g_fh1->fontset()->is_rtl() ? okbtn : cancelbtn, UI::Align::kLeft, false, true);
	button_box->add_space(2 * padding);
	button_box->add(
	   UI::g_fh1->fontset()->is_rtl() ? cancelbtn : okbtn, UI::Align::kLeft, false, true);
	main_box->add(textarea);
	main_box->add_space(1.5 * padding);
	main_box->add(button_box, UI::Align::kLeft, true);
	button_box->set_size(textarea->get_w(), okbtn->get_h());
	main_box->set_size(textarea->get_w(), textarea->get_h() + button_box->get_h() + 1.5 * padding);
	set_inner_size(main_box->get_w() + 2 * padding, main_box->get_h() + 2 * padding);

	center_to_parent();
	cancelbtn->center_mouse();
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
                   building),
     todestroy_(todestroy ? todestroy : &building) {
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

	if (!todestroy || !building || !iaplayer().can_act(building->owner().player_number()) ||
	    !(building->get_playercaps() & Widelands::Building::PCap_Bulldoze))
		die();
}

/**
 * The "Ok" button was clicked, so issue the CMD_BULLDOZE command for this building.
 */
void BulldozeConfirm::ok() {
	Widelands::Game& game = iaplayer().game();
	upcast(Widelands::Building, building, object_.get(game));
	upcast(Widelands::PlayerImmovable, todestroy, todestroy_.get(game));

	if (todestroy && building && iaplayer().can_act(building->owner().player_number()) &&
	    (building->get_playercaps() & Widelands::Building::PCap_Bulldoze)) {
		game.send_player_bulldoze(*todestroy, SDL_GetModState() & KMOD_CTRL);
	}

	die();
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
                   building) {
	// Nothing special to do
}

/*
===============
Make sure the building still exists and can in fact be dismantled.
===============
*/
void DismantleConfirm::think() {
	const Widelands::EditorGameBase& egbase = iaplayer().egbase();
	upcast(Widelands::Building, building, object_.get(egbase));

	if (!building || !iaplayer().can_act(building->owner().player_number()) ||
	    !(building->get_playercaps() & Widelands::Building::PCap_Dismantle))
		die();
}

/**
 * The "Ok" button was clicked, so issue the CMD_DISMANTLEBUILDING command for this building.
 */
void DismantleConfirm::ok() {
	Widelands::Game& game = iaplayer().game();
	upcast(Widelands::Building, building, object_.get(game));
	upcast(Widelands::PlayerImmovable, todismantle, object_.get(game));

	if (building && iaplayer().can_act(building->owner().player_number()) &&
	    (building->get_playercaps() & Widelands::Building::PCap_Dismantle)) {
		game.send_player_dismantle(*todismantle);
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
                               const Widelands::DescriptionIndex& id)
   : ActionConfirm(
        parent,
        _("Enhance building?"),
        building.descr().type() == Widelands::MapObjectType::MILITARYSITE ?
           (boost::format("%s\n\n%s") % _("Do you really want to enhance this building?") %
            /** TRANSLATORS: Warning message when player wants to enhance a military building */
            _("Be careful if the enemy is near!"))
              .str() :
           _("Do you really want to enhance this building?"),
        building),
     id_(id) {
	// Nothing special to do
}

/*
===============
Make sure the building still exists and can in fact be enhanced.
===============
*/
void EnhanceConfirm::think() {
	const Widelands::EditorGameBase& egbase = iaplayer().egbase();
	upcast(Widelands::Building, building, object_.get(egbase));

	if (!building || !iaplayer().can_act(building->owner().player_number()) ||
	    !(building->get_playercaps() & Widelands::Building::PCap_Enhancable))
		die();
}

/**
 * The "Ok" button was clicked, so issue the CMD_ENHANCEBUILDING command for this building.
 */
void EnhanceConfirm::ok() {
	Widelands::Game& game = iaplayer().game();
	upcast(Widelands::Building, building, object_.get(game));

	if (building && iaplayer().can_act(building->owner().player_number()) &&
	    (building->get_playercaps() & Widelands::Building::PCap_Enhancable)) {
		game.send_player_enhance_building(*building, id_);
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
                   (boost::format(_("Do you really want to sink %s?")) % ship.get_shipname()).str(),
                   ship) {
	// Nothing special to do
}

/**
 * Make sure the ship still exists.
 */
void ShipSinkConfirm::think() {
	const Widelands::EditorGameBase& egbase = iaplayer().egbase();
	upcast(Widelands::Ship, ship, object_.get(egbase));

	if (!ship || !iaplayer().can_act(ship->get_owner()->player_number()))
		die();
}

/**
 * The "Ok" button was clicked, so issue the CMD_ENHANCEBUILDING command for this building.
 */
void ShipSinkConfirm::ok() {
	Widelands::Game& game = iaplayer().game();
	upcast(Widelands::Ship, ship, object_.get(game));

	if (ship && iaplayer().can_act(ship->get_owner()->player_number())) {
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
                   ship) {
	// Nothing special to do
}

/**
 * Make sure the ship still exists and that it is still in expedition mode and not colonizing.
 */
void ShipCancelExpeditionConfirm::think() {
	const Widelands::EditorGameBase& egbase = iaplayer().egbase();
	upcast(Widelands::Ship, ship, object_.get(egbase));

	if (!ship || !iaplayer().can_act(ship->get_owner()->player_number()) ||
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

	if (ship && iaplayer().can_act(ship->get_owner()->player_number()) &&
	    ship->get_ship_state() != Widelands::Ship::ShipStates::kTransport &&
	    ship->get_ship_state() != Widelands::Ship::ShipStates::kExpeditionColonizing) {
		game.send_player_cancel_expedition_ship(*ship);
	}

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
                          const Widelands::DescriptionIndex& id) {
	new EnhanceConfirm(player, building, id);
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
