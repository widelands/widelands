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
#include "map_variable_manager.h"
#include "player.h"
#include "profile.h"
#include "wexception.h"

#include "upcast.h"

namespace Widelands {

static const int32_t TRIGGER_VERSION = 2;

Trigger_Building::Trigger_Building()
:
Trigger      (_("Building Trigger")),
m_player_area(0, Area<FCoords>(FCoords(Coords(0, 0)), 0)),
m_building   (_("<unset>")),
m_count      (0)
{set_trigger(false);}

Trigger_Building::~Trigger_Building() {}

void Trigger_Building::Read(Section* s, Editor_Game_Base* egbase) {
	const int32_t trigger_version= s->get_safe_int("version");
	if (1 <= trigger_version and trigger_version <= TRIGGER_VERSION) {
		m_player_area = Player_Area<Area<FCoords> >
			(s->get_safe_int("player"),
			 Area<FCoords>
			 (egbase->map().get_fcoords
			  (trigger_version == 1
			   ?
			   (Coords(s->get_safe_int("point_x"), s->get_safe_int("point_y")))
			   :
			   s->get_safe_Coords("point")),
			  s->get_safe_int("area")));
		egbase->get_iabase()
			->reference_player_tribe(m_player_area.player_number, this);
      set_building_count(s->get_int("count"));
      set_building(s->get_safe_string("building"));
	} else
		throw wexception
			("Building Trigger with unknown/unhandled version %u in map!",
			 trigger_version);
}

void Trigger_Building::Write(Section & s) const {
	s.set_int   ("version",  TRIGGER_VERSION);
	s.set_Coords("point",    m_player_area);
	s.set_int   ("area",     m_player_area.radius);
	s.set_int   ("player",   m_player_area.player_number);
	s.set_int   ("count",    get_building_count());
	s.set_string("building", m_building.c_str());
}

/*
 * check if trigger conditions are done
 */
void Trigger_Building::check_set_conditions(Game* game) {
	const Map & map = game->map();
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

   if (count>=m_count) set_trigger(true);

   // Set MapVariable inttemp
	MapVariableManager & mvm = game->map().get_mvm();
	Int_MapVariable * inttemp = mvm.get_int_variable("inttemp");
   if (!inttemp) {
      inttemp = new Int_MapVariable(false);
      inttemp->set_name("inttemp");
      mvm.register_new_variable(inttemp);
	}
   inttemp->set_value(count);

   return;
}

/*
 * Reset this trigger. This is only valid for non one timers
 */
void Trigger_Building::reset_trigger(Game *) {}

};
