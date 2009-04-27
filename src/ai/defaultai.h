/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

#ifndef DEFAULTAI_H
#define DEFAULTAI_H

#include "ai/ai_help_structs.h"
#include "computer_player.h"

#include <map>

namespace Widelands {
struct Road;
struct Flag;
}

/**
 * Default Widelands Computer Player (defaultAI)
 *
 * The behaviour of defaultAI is controlled via \ref DefaultAI::think() and all
 * functions called by \ref DefaultAI::think().
 * At the moment defaultAI should be able to build up a basic infrastructure
 * including food, mining and smithing infrastructure and a basic street net.
 * It should be able to expand it's territory and to recruit some soldiers from
 * the weapons made out of it's mined ressources.
 * It does only construct buildable and allowed (scenario mode) buildings.
 * It behaves after preciousness of a ware, which can be defined in wares conf
 * file. The higher the preciousness, the more will defaultAI care for that ware
 * and will try to build up an infrastructure to create that ware.
 *
 * \ToDo What it does not do until now is:
 * - Building and using trainings sites
 * - Higher the military presence at the frontier to other players
 * - Attacking a near enemy, if strong enough
 * - Enhancing buildings
 *
 * \ToDo Other things left to implement / improve:
 * - remove cphints from tribes directory and instead read "requieres_object"
 *   and "mines" from buildings conf file.
 * - Improve update code - currently the whole buildable area owned by defaultAI
 *   is rechecked after construction of a building or a road. Instead it would
 *   be better to write down the changed coordinates and only check those and
 *   surrounding ones.
 *
 */
struct DefaultAI : Computer_Player {
	DefaultAI(Widelands::Game &, const Widelands::Player_Number);

	virtual void think ();

	virtual void receive(Widelands::NoteImmovable const &);
	virtual void receive(Widelands::NoteField     const &);

	struct Implementation : public Computer_Player::Implementation {
		Implementation() {name = "default";}
		Computer_Player * instantiate(Widelands::Game & g, const Widelands::Player_Number p) const
		{
			return new DefaultAI(g, p);
		}
	};
	static Implementation implementation;

private:
	void late_initialization ();

	void update_all_buildable_fields     (int32_t);
	void update_all_mineable_fields      (int32_t);
	void update_all_not_buildable_fields ();

	void update_buildable_field (BuildableField &);
	void update_mineable_field (MineableField &);

	bool construct_building ();
	bool construct_roads    (int32_t);
	bool improve_roads      (int32_t);

	bool improve_transportation_ways (const Widelands::Flag &);
	bool connect_flag_to_another_economy (const Widelands::Flag &);

	bool check_economies      ();
	bool check_productionsite (ProductionSiteObserver &);

	void consider_productionsite_influence
		(BuildableField &, Widelands::Coords, BuildingObserver const &);

	EconomyObserver  * get_economy_observer (Widelands::Economy &);
	BuildingObserver & get_building_observer(char const *);

	void gain_immovable (Widelands::PlayerImmovable       &);
	void lose_immovable (Widelands::PlayerImmovable const &);
	void gain_building  (Widelands::Building              &);
	void lose_building  (Widelands::Building        const &);

	bool check_supply(BuildingObserver const &);


private:
// Variables of default AI
	bool m_buildable_changed;
	bool m_mineable_changed;

	Widelands::Player               * player;
	Widelands::Tribe_Descr const    * tribe;

	std::list<BuildingObserver>       buildings;
	int32_t                           total_constructionsites;

	std::list<Widelands::FCoords>     unusable_fields;
	std::list<BuildableField *>       buildable_fields;
	std::list<MineableField *>        mineable_fields;
	std::list<Widelands::Flag const *> new_flags;
	std::list<Widelands::Road const *> roads;
	std::list<EconomyObserver *>      economies;
	std::list<ProductionSiteObserver> productionsites;

	std::vector<WareObserver>         wares;

	int32_t next_road_due;
	int32_t next_construction_due;
	int32_t next_productionsite_check_due;
	int32_t inhibit_road_building;
};

#endif // DEFAULTAI_H
