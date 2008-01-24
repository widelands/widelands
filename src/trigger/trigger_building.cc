/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "trigger_building.h"

#include "building.h"
#include "editorinteractive.h"
#include "filesystem.h"
#include "game.h"
#include "i18n.h"
#include "map.h"
#include "player.h"
#include "profile.h"
#include "wexception.h"

#include "upcast.h"

#define PACKET_VERSION 3

namespace Widelands {

Trigger_Building::Trigger_Building(char const * const Name, bool const set)
:
Trigger_Player_Area(Name, set),
m_building         (_("<unset>"))
{}


void Trigger_Building::Read(Section & s, Editor_Game_Base & egbase) {
	try {
		int32_t const packet_version = s.get_safe_int("version");
		if (1 <= packet_version and packet_version <= PACKET_VERSION) {
			Trigger_Player_Area::Read(s, egbase);
			egbase.get_iabase()->reference_player_tribe
				(m_player_area.player_number, this);
			set_building(s.get_safe_string("building"));
		} else
			throw wexception("unknown/unhandled version %i", packet_version);
	} catch (std::exception const & e) {
		throw wexception("(building): %s", e.what());
	}
}

void Trigger_Building::Write(Section & s) const {
	s.set_string("type",     "building");
	s.set_int   ("version",  PACKET_VERSION);
	Trigger_Player_Area::Write(s);
	s.set_string("building", m_building);
}

/*
 * check if trigger conditions are done
 */
void Trigger_Building::check_set_conditions(Game const & game) {
	Map const & map = game.map();
	if
		(m_player_area.x < 0 or map.get_width () <= m_player_area.x
		 or
		 m_player_area.y < 0 or map.get_height() <= m_player_area.y
		 or
		 m_player_area.player_number <= 0
		 or
		 map.get_nrplayers() < m_player_area.player_number)
		return;


	uint32_t count = 0;
	MapRegion<Area<FCoords> > mr (map, m_player_area);
	do if
		(upcast(Building const, building, mr.location().field->get_immovable()))
		if
			(building->owner().get_player_number() == m_player_area.player_number
			 and
			 building->name() == m_building)
			++count;
	while (mr.advance(map));

	if (count >= m_count)
		set_trigger(true);

	//  Set Variable inttemp.
	Manager<Variable> & mvm = game.map().mvm();
	Variable_Int * inttemp = dynamic_cast<Variable_Int *>(mvm["inttemp"]);
	if (!inttemp) {
      inttemp = new Variable_Int(false);
      inttemp->set_name("inttemp");
		mvm.register_new(*inttemp);
	}
   inttemp->set_value(count);

   return;
}

};
