/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "bulldozeconfirm.h"

#include "interactive_player.h"
#include "logic/building.h"
#include "logic/player.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"
#include "upcast.h"

#include <boost/format.hpp>

using boost::format;


/**
 * Confirmation dialog box for the bulldoze request for a building.
 */
struct BulldozeConfirm : public UI::Window {
	BulldozeConfirm
		(Interactive_Player         & parent,
		 Widelands::Building        & building,
		 Widelands::PlayerImmovable * todestroy = 0);

	Interactive_Player & iaplayer() const {
		return ref_cast<Interactive_Player, UI::Panel>(*get_parent());
	}

	virtual void think();

private:
	void ok();

	Widelands::Object_Ptr m_building;
	Widelands::Object_Ptr m_todestroy;
};

/*
===============
Create the panels.
If todestroy is 0, the building will be destroyed when the user confirms it.
Otherwise, todestroy is destroyed when the user confirms it. This is useful to
confirm building destruction when the building's base flag is removed.
===============
*/
BulldozeConfirm::BulldozeConfirm
	(Interactive_Player         & parent,
	 Widelands::Building        & building,
	 Widelands::PlayerImmovable * todestroy)
	:
	UI::Window
		(&parent, "bulldoze_confirm", 0, 0, 200, 120, _("Destroy building?")),
	m_building (&building),
	m_todestroy(todestroy ? todestroy : &building)
{
	new UI::Multiline_Textarea
		(this,
		 0, 0, 200, 74,
		 (format(_("Do you really want to destroy this %s?")) % building.descname()).str(),
		 UI::Align_Center);

	UI::Button * okbtn =
		new UI::Button
			(this, "ok",
			 6, 80, 80, 34,
			 g_gr->get_picture(PicMod_UI,   "pics/but4.png"),
			 g_gr->get_picture(PicMod_Game, "pics/menu_okay.png"));
	okbtn->sigclicked.connect(boost::bind(&BulldozeConfirm::ok, this));

	UI::Button * cancelbtn =
		new UI::Button
			(this, "abort",
			 114, 80, 80, 34,
			 g_gr->get_picture(PicMod_UI,   "pics/but4.png"),
			 g_gr->get_picture(PicMod_Game, "pics/menu_abort.png"));
	cancelbtn->sigclicked.connect(boost::bind(&BulldozeConfirm::die, this));

	center_to_parent();
	cancelbtn->center_mouse();
}


/*
===============
Make sure the building still exists and can in fact be bulldozed.
===============
*/
void BulldozeConfirm::think()
{
	Widelands::Editor_Game_Base const & egbase = iaplayer().egbase();
	upcast(Widelands::Building,        building,  m_building .get(egbase));
	upcast(Widelands::PlayerImmovable, todestroy, m_todestroy.get(egbase));

	if
		(not todestroy ||
		 not building  ||
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
	Widelands::Game & game   = iaplayer().game();
	upcast(Widelands::Building,        building,  m_building.get(game));
	upcast(Widelands::PlayerImmovable, todestroy, m_todestroy.get(game));

	if
		(todestroy &&
		 building &&
		 iaplayer().can_act(building->owner().player_number()) and
		 building->get_playercaps() & Widelands::Building::PCap_Bulldoze)
	{
		game.send_player_bulldoze
			(*todestroy, get_key_state(SDLK_LCTRL) or get_key_state(SDLK_RCTRL));
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
	(Interactive_Player         &       player,
	 Widelands::Building        &       building,
	 Widelands::PlayerImmovable * const todestroy)
{
	new BulldozeConfirm(player, building, todestroy);
}
