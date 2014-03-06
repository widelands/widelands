/*
 * Copyright (C) 2002-2004, 2006-2011, 2013 by the Widelands Development Team
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

#include "economy/economy.h"
#include "graphic/graphic.h"
#include "logic/building.h"
#include "logic/player.h"
#include "logic/ship.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"
#include "upcast.h"
#include "wui/interactive_player.h"

using boost::format;

struct ActionConfirm : public UI::Window {
	ActionConfirm
		(Interactive_Player & parent,
		 const std::string & windowtitle,
		 const std::string & message,
		 Widelands::Building & building);

	ActionConfirm
		(Interactive_Player & parent,
		 const std::string & windowtitle,
		 const std::string & message,
		 Widelands::Ship & ship);

	Interactive_Player & iaplayer() const {
		return ref_cast<Interactive_Player, UI::Panel>(*get_parent());
	}

	virtual void think() = 0;
	virtual void ok() = 0;

protected:
	Widelands::Object_Ptr m_object;
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
	BulldozeConfirm
		(Interactive_Player & parent,
		 Widelands::Building & building,
		 Widelands::PlayerImmovable * todestroy = nullptr);

	virtual void think() override;
	virtual void ok() override;

private:
	Widelands::Object_Ptr m_todestroy;
};

/**
 * Confirmation dialog box for the dismantle request for a building.
 */
struct DismantleConfirm : public ActionConfirm {
	DismantleConfirm
		(Interactive_Player & parent,
		 Widelands::Building & building);

	virtual void think() override;
	virtual void ok() override;
};

/**
 * Confirmation dialog box for the enhance request for a building.
 */
struct EnhanceConfirm : public ActionConfirm {
	EnhanceConfirm
		(Interactive_Player & parent,
		 Widelands::Building & building,
		 const Widelands::Building_Index & id);

	virtual void think() override;
	virtual void ok() override;

private:
    // Do not make this a reference - it is a stack variable in the caller
	const Widelands::Building_Index m_id;
};


/**
 * Confirmation dialog box for the sink request for a ship.
 */
struct ShipSinkConfirm : public ActionConfirm {
	ShipSinkConfirm(Interactive_Player & parent, Widelands::Ship & ship);

	virtual void think() override;
	virtual void ok() override;
};

/**
 * Confirmation dialog box for the cancel expedition request for a ship.
 */
struct ShipCancelExpeditionConfirm : public ActionConfirm {
	ShipCancelExpeditionConfirm(Interactive_Player & parent, Widelands::Ship & ship);

	virtual void think() override;
	virtual void ok() override;
};


ActionConfirm::ActionConfirm
	(Interactive_Player & parent,
	 const std::string & windowtitle,
	 const std::string & message,
	 Widelands::Building & building)
	:
	UI::Window
		(&parent, "building_action_confirm", 0, 0, 200, 120, windowtitle),
	m_object (&building)
{
	new UI::Multiline_Textarea
		(this,
		 0, 0, 200, 74,
		 (format(message) % building.descname()).str(),
		 UI::Align_Center);

	UI::Button * okbtn =
		new UI::Button
			(this, "ok",
			 6, 80, 80, 34,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/menu_okay.png"));
	okbtn->sigclicked.connect(boost::bind(&ActionConfirm::ok, this));

	UI::Button * cancelbtn =
		new UI::Button
			(this, "abort",
			 114, 80, 80, 34,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/menu_abort.png"));
	cancelbtn->sigclicked.connect(boost::bind(&ActionConfirm::die, this));

	center_to_parent();
	cancelbtn->center_mouse();
}


ActionConfirm::ActionConfirm
	(Interactive_Player & parent,
	 const std::string & windowtitle,
	 const std::string & message,
	 Widelands::Ship & ship)
	:
	UI::Window(&parent, "ship_action_confirm", 0, 0, 200, 120, windowtitle),
	m_object (&ship)
{
	new UI::Multiline_Textarea
		(this,
		 0, 0, 200, 74,
		 message,
		 UI::Align_Center);

	UI::Button * okbtn =
		new UI::Button
			(this, "ok",
			 6, 80, 80, 34,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/menu_okay.png"));
	okbtn->sigclicked.connect(boost::bind(&ActionConfirm::ok, this));

	UI::Button * cancelbtn =
		new UI::Button
			(this, "abort",
			 114, 80, 80, 34,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/menu_abort.png"));
	cancelbtn->sigclicked.connect(boost::bind(&ActionConfirm::die, this));

	center_to_parent();
	cancelbtn->center_mouse();
}


/*
===============
Create the panels for bulldoze confirmation.
===============
*/
BulldozeConfirm::BulldozeConfirm
	(Interactive_Player & parent,
	 Widelands::Building & building,
	 Widelands::PlayerImmovable * todestroy)
	:
	ActionConfirm
		(parent,
		 _("Destroy building?"),
		 _("Do you really want to destroy this %s?"),
		 building),
	m_todestroy(todestroy ? todestroy : &building)
{
	// Nothing special to do
}


/*
===============
Make sure the building still exists and can in fact be bulldozed.
===============
*/
void BulldozeConfirm::think()
{
	const Widelands::Editor_Game_Base & egbase = iaplayer().egbase();
	upcast(Widelands::Building, building, m_object .get(egbase));
	upcast(Widelands::PlayerImmovable, todestroy, m_todestroy.get(egbase));

	if
		(not todestroy ||
		 not building ||
		 not iaplayer().can_act(building->owner().player_number())
		 or not
		 (building->get_playercaps() & Widelands::Building::PCap_Bulldoze))
		die();
}


/**
 * The "Ok" button was clicked, so issue the CMD_BULLDOZE command for this building.
 */
void BulldozeConfirm::ok()
{
	Widelands::Game & game = iaplayer().game();
	upcast(Widelands::Building, building, m_object.get(game));
	upcast(Widelands::PlayerImmovable, todestroy, m_todestroy.get(game));

	if
		(todestroy &&
		 building &&
		 iaplayer().can_act(building->owner().player_number()) and
		 (building->get_playercaps() & Widelands::Building::PCap_Bulldoze))
	{
		game.send_player_bulldoze
			(*todestroy, get_key_state(SDLK_LCTRL) or get_key_state(SDLK_RCTRL));
		iaplayer().need_complete_redraw();
	}

	die();
}


/*
===============
Create the panels for dismantle confirmation.
===============
*/
DismantleConfirm::DismantleConfirm
	(Interactive_Player & parent,
	 Widelands::Building & building)
	:
	ActionConfirm
		(parent,
		 _("Dismantle building?"),
		 _("Do you really want to dismantle this %s?"),
		 building)
{
	// Nothing special to do
}


/*
===============
Make sure the building still exists and can in fact be dismantled.
===============
*/
void DismantleConfirm::think()
{
	const Widelands::Editor_Game_Base & egbase = iaplayer().egbase();
	upcast(Widelands::Building, building, m_object.get(egbase));

	if
		(not building ||
		 not iaplayer().can_act(building->owner().player_number())
		 or not
		 (building->get_playercaps() & Widelands::Building::PCap_Dismantle))
		die();
}


/**
 * The "Ok" button was clicked, so issue the CMD_DISMANTLEBUILDING command for this building.
 */
void DismantleConfirm::ok()
{
	Widelands::Game & game = iaplayer().game();
	upcast(Widelands::Building, building, m_object.get(game));
	upcast(Widelands::PlayerImmovable, todismantle, m_object.get(game));

	if
		(building &&
		 iaplayer().can_act(building->owner().player_number()) and
		 (building->get_playercaps() & Widelands::Building::PCap_Dismantle))
	{
		game.send_player_dismantle(*todismantle);
		iaplayer().need_complete_redraw();
	}

	die();
}


/*
===============
Create the panels for enhancement confirmation.
===============
*/
EnhanceConfirm::EnhanceConfirm
	(Interactive_Player & parent,
	 Widelands::Building & building,
	 const Widelands::Building_Index & id)
	:
	ActionConfirm
		(parent,
		 _("Enhance building?"),
		 _("Do you really want to enhance this %s?"),
		 building),
	m_id(id)
{
	// Nothing special to do
}


/*
===============
Make sure the building still exists and can in fact be enhanced.
===============
*/
void EnhanceConfirm::think()
{
	const Widelands::Editor_Game_Base & egbase = iaplayer().egbase();
	upcast(Widelands::Building, building, m_object.get(egbase));

	if
		(not building ||
		 not iaplayer().can_act(building->owner().player_number())
		 or not
		 (building->get_playercaps() & Widelands::Building::PCap_Enhancable))
		die();
}


/**
 * The "Ok" button was clicked, so issue the CMD_ENHANCEBUILDING command for this building.
 */
void EnhanceConfirm::ok()
{
	Widelands::Game & game = iaplayer().game();
	upcast(Widelands::Building, building, m_object.get(game));

	if
		(building &&
		 iaplayer().can_act(building->owner().player_number()) and
		 (building->get_playercaps() & Widelands::Building::PCap_Enhancable))
	{
		game.send_player_enhance_building(*building, m_id);
		iaplayer().need_complete_redraw();
	}

	die();
}

/**
 * Create the panels for confirmation.
 */
ShipSinkConfirm::ShipSinkConfirm(Interactive_Player & parent, Widelands::Ship & ship)
	:
	ActionConfirm(parent, _("Sink the ship?"), _("Do you really want to sink this ship?"), ship)
{
	// Nothing special to do
}


/**
 * Make sure the ship still exists.
 */
void ShipSinkConfirm::think()
{
	const Widelands::Editor_Game_Base & egbase = iaplayer().egbase();
	upcast(Widelands::Ship, ship, m_object.get(egbase));

	if (!ship || !iaplayer().can_act(ship->get_owner()->player_number()))
		die();
}


/**
 * The "Ok" button was clicked, so issue the CMD_ENHANCEBUILDING command for this building.
 */
void ShipSinkConfirm::ok()
{
	Widelands::Game & game = iaplayer().game();
	upcast(Widelands::Ship, ship, m_object.get(game));

	if (ship && iaplayer().can_act(ship->get_owner()->player_number())) {
		game.send_player_sink_ship(*ship);
		iaplayer().need_complete_redraw();
	}

	die();
}


/**
 * Create the panels for confirmation.
 */
ShipCancelExpeditionConfirm::ShipCancelExpeditionConfirm(Interactive_Player & parent, Widelands::Ship & ship)
	:
	ActionConfirm(parent, _("Sink the ship?"), _("Do you really want to cancel the active expedition?"), ship)
{
	// Nothing special to do
}


/**
 * Make sure the ship still exists and that it is still in expedition mode and not colonizing.
 */
void ShipCancelExpeditionConfirm::think()
{
	const Widelands::Editor_Game_Base & egbase = iaplayer().egbase();
	upcast(Widelands::Ship, ship, m_object.get(egbase));

	if
		(!ship
		 &&
		 !iaplayer().can_act(ship->get_owner()->player_number())
		 &&
		 ship->get_ship_state() == Widelands::Ship::TRANSPORT
		 &&
		 ship->get_ship_state() == Widelands::Ship::EXP_COLONIZING)
	{
		die();
	}
}


/**
 * The "Ok" button was clicked, so issue the command for this building.
 */
void ShipCancelExpeditionConfirm::ok()
{
	Widelands::Game & game = iaplayer().game();
	upcast(Widelands::Ship, ship, m_object.get(game));

	if
		(ship
		 &&
		 iaplayer().can_act(ship->get_owner()->player_number())
		 &&
		 ship->get_ship_state() != Widelands::Ship::TRANSPORT
		 &&
		 ship->get_ship_state() != Widelands::Ship::EXP_COLONIZING)
	{
		game.send_player_cancel_expedition_ship(*ship);
		iaplayer().need_complete_redraw();
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
void show_bulldoze_confirm
	(Interactive_Player & player,
	 Widelands::Building & building,
	 Widelands::PlayerImmovable * const todestroy)
{
	new BulldozeConfirm(player, building, todestroy);
}

/**
 * Create a DismantleConfirm window.
 * No further action is required by the caller: any action necessary to actually
 * dismantle the building if the user confirms is taken automatically.
 *
 * \param building this is the building that the confirmation dialog displays.
 */
void show_dismantle_confirm
	(Interactive_Player & player,
	 Widelands::Building & building)
{
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
void show_enhance_confirm
	(Interactive_Player & player,
	 Widelands::Building & building,
	 const Widelands::Building_Index & id)
{
	new EnhanceConfirm(player, building, id);
}


/**
 * Create a ShipSinkConfirm window.
 * No further action is required by the caller: any action necessary to actually
 * sink the ship if the user confirms is taken automatically.
 *
 * \param ship this is the ship that the confirmation dialog displays.
 */
void show_ship_sink_confirm(Interactive_Player & player, Widelands::Ship & ship)
{
	new ShipSinkConfirm(player, ship);
}


/**
 * Create a ShipCancelExpeditionConfirm window.
 * No further action is required by the caller: any action necessary to actually
 * cancel the expedition if the user confirms is taken automatically.
 *
 * \param ship this is the ship that the confirmation dialog displays.
 */
void show_ship_cancel_expedition_confirm(Interactive_Player & player, Widelands::Ship & ship)
{
	new ShipCancelExpeditionConfirm(player, ship);
}
