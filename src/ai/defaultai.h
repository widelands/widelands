/*
 * Copyright (C) 2008-2010, 2012 by the Widelands Development Team
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

#ifndef DEFAULTAI_H
#define DEFAULTAI_H

#include <map>

#include "ai/ai_help_structs.h"
#include "computer_player.h"
#include "i18n.h"

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
 * the weapons made out of it's mined resources.
 * It does only construct buildable and allowed (scenario mode) buildings.
 * It behaves after preciousness of a ware, which can be defined in wares conf
 * file. The higher the preciousness, the more will defaultAI care for that ware
 * and will try to build up an infrastructure to create that ware.
 *
 * \NOTE Network safeness:
 * - The current implementation does not care about network safe randomness, as
 *   only the host is running the computer player code and sends it's player
 *   commands to all other players. If this network behaviour is changed,
 *   remember to change some time() in network save random functions.
 *
 * \TODO Improvements:
 * - Improve different initialization types (Aggressive, Normal, Defensive)
 * - Improve update code - currently the whole buildable area owned by defaultAI
 *   is rechecked after construction of a building or a road. Instead it would
 *   be better to write down the changed coordinates and only check those and
 *   surrounding ones. Same applies for other parts of the code:
 *   e.g. check_militarysite checks the whole visible area for enemy area, but
 *   it would already be enough, if it checks the outer circle ring.
 * - improvements and speedups in the whole defaultAI code.
 * - handling of trainingsites (if supply line is broken - send some soldiers
 *   out, to have some more forces. Reincrease the number of soldiers that
 *   should be trained if inputs get filled again.).
 *
 */
struct DefaultAI : Computer_Player {
	DefaultAI(Widelands::Game &, const Widelands::Player_Number, uint8_t);
	~DefaultAI();
	virtual void think () override;

	virtual void receive(const Widelands::NoteImmovable &) override;
	virtual void receive(const Widelands::NoteFieldPossession     &) override;

	enum {
		AGGRESSIVE = 2,
		NORMAL     = 1,
		DEFENSIVE  = 0,
	};

	/// Implementation for Aggressive
	struct AggressiveImpl : public Computer_Player::Implementation {
		AggressiveImpl() {name = _("Aggressive");}
		Computer_Player * instantiate
			(Widelands::Game & game, Widelands::Player_Number const p) const override
		{
			return new DefaultAI(game, p, AGGRESSIVE);
		}
	};

	struct NormalImpl : public Computer_Player::Implementation {
		NormalImpl() {name = _("Normal");}
		Computer_Player * instantiate
			(Widelands::Game & game, Widelands::Player_Number const p) const override
		{
			return new DefaultAI(game, p, NORMAL);
		}
	};

	struct DefensiveImpl : public Computer_Player::Implementation {
		DefensiveImpl() {name = _("Defensive");}
		Computer_Player * instantiate
			(Widelands::Game & game, Widelands::Player_Number const p) const override
		{
			return new DefaultAI(game, p, DEFENSIVE);
		}
	};

	static AggressiveImpl aggressiveImpl;
	static NormalImpl normalImpl;
	static DefensiveImpl defensiveImpl;

private:
	void late_initialization ();

	void update_all_buildable_fields     (int32_t);
	void update_all_mineable_fields      (int32_t);
	void update_all_not_buildable_fields ();

	void update_buildable_field(BuildableField &, uint16_t = 6, bool = false);
	void update_mineable_field (MineableField &);

	void update_productionsite_stats(int32_t);

	bool construct_building (int32_t);
	bool construct_roads    (int32_t);
	bool improve_roads      (int32_t);

	bool improve_transportation_ways (const Widelands::Flag &);
	bool connect_flag_to_another_economy (const Widelands::Flag &);

	bool check_economies       ();
	bool check_productionsites (int32_t);
	bool check_mines           (int32_t);
	bool check_militarysites   (int32_t);

	int32_t recalc_with_border_range(const BuildableField &, int32_t);
	int32_t calculate_need_for_ps(BuildingObserver &, int32_t);

	void consider_productionsite_influence
		(BuildableField &, Widelands::Coords, const BuildingObserver &);

	EconomyObserver  * get_economy_observer (Widelands::Economy &);
	BuildingObserver & get_building_observer(char const *);

	void gain_immovable (Widelands::PlayerImmovable       &);
	void lose_immovable (const Widelands::PlayerImmovable &);
	void gain_building  (Widelands::Building              &);
	void lose_building  (const Widelands::Building        &);

	bool check_supply (const BuildingObserver &);

	bool consider_attack (int32_t);


private:
	// Variables of default AI
	uint8_t type;

	bool m_buildable_changed;
	bool m_mineable_changed;

	Widelands::Player                * player;
	Widelands::Tribe_Descr const     * tribe;

	std::vector<BuildingObserver>      buildings;
	uint32_t                           total_constructionsites;

	std::list<Widelands::FCoords>      unusable_fields;
	std::list<BuildableField *>        buildable_fields;
	std::list<BlockedField>          blocked_fields;
	std::list<MineableField *>         mineable_fields;
	std::list<Widelands::Flag const *> new_flags;
	std::list<Widelands::Coords>       flags_to_be_removed;
	std::list<Widelands::Road const *> roads;
	std::list<EconomyObserver *>       economies;
	std::list<ProductionSiteObserver>  productionsites;
	std::list<ProductionSiteObserver>  mines;
	std::list<MilitarySiteObserver>    militarysites;

	std::vector<WareObserver>          wares;

	int32_t next_road_due;
	int32_t next_stats_update_due;
	int32_t next_construction_due;
	int32_t next_productionsite_check_due;
	int32_t next_mine_check_due;
	int32_t next_militarysite_check_due;
	int32_t next_attack_consideration_due;
	int32_t inhibit_road_building;
	int32_t time_of_last_construction;

	uint16_t numof_warehouses;
};

#endif
