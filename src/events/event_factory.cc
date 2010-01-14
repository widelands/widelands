/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "event_factory.h"

#include "event_allow_building_types.h"
#include "event_allow_retreat_change.h"
#include "event_allow_worker_types.h"
#include "event_building.h"
#include "event_conquer_area.h"
#include "event_flag.h"
#include "event_forbid_building_types.h"
#include "event_forbid_retreat_change.h"
#include "event_forbid_worker_types.h"
#include "event_immovable.h"
#include "event_lua.h"
#include "event_message.h"
#include "event_message_box.h"
#include "event_expire_message.h"
#include "event_move_view.h"
#include "event_player_seeall.h"
#include "event_retreat_change.h"
#include "event_reveal_campaign.h"
#include "event_reveal_objective.h"
#include "event_reveal_scenario.h"
#include "event_road.h"
#include "event_set_player_flag_style.h"
#include "event_set_player_frontier_style.h"
#include "event_set_timer.h"
#include "event_unhide_area.h"
#include "profile/profile.h"

#include "wexception.h"

#include "i18n.h"

namespace Widelands {

namespace Event_Factory {

///\todo This information belongs into the event classes
Type_Descr EVENT_TYPE_DESCRIPTIONS[] = {
	{
		true,
		"allow_worker_types",      _("Allow worker types"),
		_("Allows the creation of workers of certain types for a player.")
	},
	{
		true,
		"forbid_worker_types",     _("Forbid worker types"),
		_("Forbids the creation of workers of certain types for a player.")
	},
	{
		true,
		"allow_building_types",      _("Allow building types"),
		_("Allows the construction of buildings of certain types for a player.")
	},
	{
		true,
		"forbid_building_types",     _("Forbid building types"),
		_("Forbids the construction of buildings of certain types for a player.")
	},
	{
		false,
		"building",               _("Create building"),
		_("Create a building and optionally place workers in it")
	},
	{
		true,
		"conquer_area",           _("Conquer area"),
		_
			("Gives ownership of all unowned locations within an area to the "
			 "player.")
	},
	{
		false,
		"flag",                   _("Create flag"),
		_("Create a flag.")
	},
	{
		false,
		"immovable",              _("Create immovable"),
		_("Create an immovable.")
	},
	{
		false,
		"lua",              _("Lua event"),
		_("Run a lua command")
	},
	{
		true,
		"message_box",            _("Show message box"),
		_
			("Show a message box. It can be modal or and have a picture. An "
			 "events can be assigned to each button to give the user chose the "
			 "course of the game.")
	},
	{
		true,
		"move_view",              _("Move view"),
		_("Center the player's view on a certain location.")
	},
	{
		false,
		"reveal_campaign",        _("Reveal campaign"),
		_("Reveal a campaign.")
	},
	{
		false,
		"reveal_objective",       _("Reveal objective"),
		_("Reveal an objective.")
	},
	{
		false,
		"reveal_scenario",        _("Reveal scenario"),
		_("Reveal a scenario.")
	},
	{
		false,
		"road",                   _("Create road"),
		_("Create a road and optionally place a carrier on it")
	},
	{
		true,
		"set_timer",              _("Set timer"),
		_("Set a timer to trigger after a certain duration.")
	},
	{
		true,
		"unhide_area",            _("Unhide area"),
		_
			("Gives vision for a specified amount of time of all locations "
			 "within an area to the player.")
	},
	{
		true,
		"set_player_frontier_style", _("Set player's frontier style"),
		_
			("Sets the style of the player's frontier markers to one of the "
			 "styles that are defined in the player's tribe.")
	},
	{
		true,
		"set_player_flag_style",     _("Set player's flag style"),
		_
			("Sets the style of the player's flags to one of the styles that are "
			 "defined in the player's tribe.")
	},
	{
		false, // Change this value when option window exists
		"allow_retreat_change",      _("Allows retreat value customization"),
		_
			("Allows that the player can change the value of 'retreat when hit"
			 "points below x%'.")
	},
	{
		false, // Change this value when option window exists
		"forbid_retreat_change",     _("Forbids retreat value customization"),
		_
			("Forbids that the player can change the value of 'retreat when hit"
			 "points below x%'.")
	},
	{
		false, // Change this value when option window exists
		"retreat_change",            _("Set player retreat percentage"),
		_
			("Sets the player value of 'retreat when hit points below x%', this "
			 "value could be overridden by players if allowed.")
	},
	{
		false, // Change this value when option window exists
		"seeall",                    _("Set see all mode for a player"),
		_("Sets the see all mode for a specific player to on/off.")
	},
	{
		false,
		"message",                   _("Creates a message for a player"),
		_
			("Creates a message for a player. This message can last forever, "
			 "during a specified duration, or until a special event is run.")
	},
	{
		true,
		"expire_message",            _("Expires a previously created message"),
		_
			("Expires a message that was previously created for a player. The "
			 "message will cease to exist.")
	}
};


Event & create(size_t const id) {
	return
		create
			(id, i18n::translate(EVENT_TYPE_DESCRIPTIONS[id].name), Event::INIT);
}


Event & create
	(size_t const id, char const * const name, Event::State const state)
{
	switch (id) {
	case  0: return *new Event_Allow_Worker_Types       (name, state);
	case  1: return *new Event_Forbid_Worker_Types      (name, state);
	case  2: return *new Event_Allow_Building_Types     (name, state);
	case  3: return *new Event_Forbid_Building_Types    (name, state);
	case  4: return *new Event_Building                 (name, state);
	case  5: return *new Event_Conquer_Area             (name, state);
	case  6: return *new Event_Flag                     (name, state);
	case  7: return *new Event_Immovable                (name, state);
	case  8: return *new Event_Lua                      (name, state);
	case  9: return *new Event_Message_Box              (name, state);
	case 10: return *new Event_Move_View                (name, state);
	case 11: return *new Event_Reveal_Campaign          (name, state);
	case 12: return *new Event_Reveal_Objective         (name, state);
	case 13: return *new Event_Reveal_Scenario          (name, state);
	case 14: return *new Event_Road                     (name, state);
	case 15: return *new Event_Set_Timer                (name, state);
	case 16: return *new Event_Unhide_Area              (name, state);
	case 17: return *new Event_Set_Player_Frontier_Style(name, state);
	case 18: return *new Event_Set_Player_Flag_Style    (name, state);
	case 19: return *new Event_Allow_Retreat_Change     (name, state);
	case 20: return *new Event_Forbid_Retreat_Change    (name, state);
	case 21: return *new Event_Retreat_Change           (name, state);
	case 22: return *new Event_Player_See_All           (name, state);
	case 23: return *new Event_Message                  (name, state);
	case 24: return *new Event_Expire_Message           (name, state);
	default:
		assert(false);
	}
}


Event & create(Section & s, Editor_Game_Base & egbase) {
	char const * type_name = s.get_safe_string("type");

	//  Handle old names.
	if (not strcmp(type_name, "unhide_objective"))
		type_name = "reveal_objective";
	if (not strcmp(type_name, "set_map_visiblity"))
		type_name = "reveal_scenario";
	if (not strcmp(type_name, "set_campaign_visiblity"))
		type_name = "reveal_campaign";
	if (not strcmp(type_name, "set_null_trigger"))
		type_name = "set_timer";
	if (not strcmp(type_name, "allow_building"))
		type_name =
			s.get_bool("allow", true) ? "allow_building_types" :
			"forbid_building_types";

	size_t i = 0;
	while (strcmp(type_name, EVENT_TYPE_DESCRIPTIONS[i].id))
		if (++i == nr_event_types())
			throw wexception("invalid type \"%s\"", type_name);
	switch (i) {
	case  0: return *new Event_Allow_Worker_Types       (s, egbase);
	case  1: return *new Event_Forbid_Worker_Types      (s, egbase);
	case  2: return *new Event_Allow_Building_Types     (s, egbase);
	case  3: return *new Event_Forbid_Building_Types    (s, egbase);
	case  4: return *new Event_Building                 (s, egbase);
	case  5: return *new Event_Conquer_Area             (s, egbase);
	case  6: return *new Event_Flag                     (s, egbase);
	case  7: return *new Event_Immovable                (s, egbase);
	case  8: return *new Event_Lua                      (s, egbase);
	case  9: return *new Event_Message_Box              (s, egbase);
	case 10: return *new Event_Move_View                (s, egbase);
	case 11: return *new Event_Reveal_Campaign          (s, egbase);
	case 12: return *new Event_Reveal_Objective         (s, egbase);
	case 13: return *new Event_Reveal_Scenario          (s, egbase);
	case 14: return *new Event_Road                     (s, egbase);
	case 15: return *new Event_Set_Timer                (s, egbase);
	case 16: return *new Event_Unhide_Area              (s, egbase);
	case 17: return *new Event_Set_Player_Frontier_Style(s, egbase);
	case 18: return *new Event_Set_Player_Flag_Style    (s, egbase);
	case 19: return *new Event_Allow_Retreat_Change     (s, egbase);
	case 20: return *new Event_Forbid_Retreat_Change    (s, egbase);
	case 21: return *new Event_Retreat_Change           (s, egbase);
	case 22: return *new Event_Player_See_All           (s, egbase);
	case 23: return *new Event_Message                  (s, egbase);
	case 24: return *new Event_Expire_Message           (s, egbase);
	default:
		assert(false);
	}
}


Type_Descr const & type_descr(size_t const id) {
	assert(id < nr_event_types());
	return EVENT_TYPE_DESCRIPTIONS[id];
}


size_t nr_event_types() {
	return sizeof(EVENT_TYPE_DESCRIPTIONS) / sizeof(*EVENT_TYPE_DESCRIPTIONS);
}

}

}
