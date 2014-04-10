/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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

#ifndef TRIBE_H
#define TRIBE_H

#include <map>
#include <vector>

#include "TribeBasicInfo.h"
#include "description_maintainer.h"
#include "graphic/animation.h"
#include "io/filewrite.h"
#include "logic/bob.h"
#include "logic/building.h"
#include "logic/immovable.h"
#include "logic/military_data.h"
#include "logic/ware_descr.h"
#include "logic/worker.h"

namespace Widelands {

class Warehouse;
class Worker_Descr;
struct Building_Descr;
class Editor_Game_Base;
struct Event;
struct WareDescr;
struct Resource_Descr;
struct World;

/*
Tribes
------

Every player chooses a tribe. A tribe has distinct properties such as the
buildings it can build and the associated graphics.
Two players can choose the same tribe.
*/
struct Tribe_Descr : boost::noncopyable {
	enum {
		OK = 0,
		ERR_WRONGVERSION
	};

	Tribe_Descr(const std::string & name, Editor_Game_Base &);

	//  Static function to check for tribes.
	static bool exists_tribe
		(const std::string & name, TribeBasicInfo * info = nullptr);
	static std::vector<std::string> get_all_tribenames();
	static std::vector<TribeBasicInfo> get_all_tribe_infos();


	const std::string & name() const {return m_name;}
	const World & world() const {return m_world;}

	Ware_Index get_nrworkers() const {return m_workers.get_nitems();}
	Worker_Descr const * get_worker_descr(const Ware_Index& index) const {
		return m_workers.get(index);
	}
	Ware_Index worker_index(const std::string & workername) const {
		return m_workers.get_index(workername);
	}
	Ware_Index worker_index(char const * const workername) const {
		return m_workers.get_index(workername);
	}
	Ware_Index carrier2() const {
		if (m_carrier2.size())
			return worker_index(m_carrier2);
		return worker_index("carrier");
	}
	Ware_Index get_nrwares() const {return m_wares.get_nitems();}
	Ware_Index safe_ware_index(const std::string & warename) const;
	Ware_Index safe_ware_index(const char * const warename) const;
	Ware_Index ware_index(const std::string & warename) const;
	Ware_Index ware_index(char const * const warename) const;
	WareDescr const * get_ware_descr(const Ware_Index& index) const {
		return m_wares.get(index);
	}
	void set_ware_type_has_demand_check(const Ware_Index& index) const {
		m_wares.get(index)->set_has_demand_check();
	}
	void set_worker_type_has_demand_check(const Ware_Index& index) const {
		m_workers.get(index)->set_has_demand_check();
	}
	Ware_Index safe_worker_index(const std::string & workername) const;
	Ware_Index safe_worker_index(const char * const workername) const;
	Building_Index get_nrbuildings() const {
		return m_buildings.get_nitems();
	}
	Building_Index safe_building_index(char const * name) const;
	Building_Descr const * get_building_descr(const Building_Index& index) const
	{
		return m_buildings.get(index);
	}
	Building_Index building_index(const std::string & buildingname) const {
		return m_buildings.get_index(buildingname);
	}
	Building_Index building_index(char const * const buildingname) const {
		return m_buildings.get_index(buildingname);
	}
	int32_t get_immovable_index(char const * const l) const {
		return m_immovables.get_index(l);
	}
	int32_t get_immovable_index(const std::string & l) const {
		return m_immovables.get_index(l);
	}
	int32_t get_nr_immovables() {return m_immovables.get_nitems();}
	Immovable_Descr const * get_immovable_descr(int32_t const index) const {
		return m_immovables.get(index);
	}
	Immovable_Descr const * get_immovable_descr(const std::string & imm_name) const {
		return m_immovables.get(get_immovable_index(imm_name.c_str()));
	}
	int32_t get_bob(char const * const l) const {return m_bobs.get_index(l);}
	BobDescr const * get_bob_descr(uint16_t const index) const {
		return m_bobs.get(index);
	}
	BobDescr const * get_bob_descr(const std::string & bob_name) const {
		return m_bobs.exists(bob_name.c_str());
	}
	int32_t get_nr_bobs() {return m_bobs.get_nitems();}

	const std::vector<Ware_Index> & worker_types_without_cost() const {
		return m_worker_types_without_cost;
	}

	typedef std::vector<std::pair<std::string, uint32_t> > AnimationStyles;
	struct Nonexistent {};
	uint8_t frontier_style_index(const std::string & stylename) const {
		for (uint8_t result = m_anim_frontier.size();;)
			if (m_anim_frontier.at(--result).first == stylename)
				return result;
			else if (not result)
				throw Nonexistent();

		return 0;
	}
	uint8_t flag_style_index    (const std::string & stylename) const {
		for (uint8_t result = m_anim_flag.size();;)
			if (m_anim_flag.at(--result).first == stylename)
				return result;
			else if (not result)
				throw Nonexistent();

		return 0;
	}
	uint8_t frontier_style_index(char const * const stylename) const {
		for (uint8_t result = m_anim_frontier.size();;)
			if (m_anim_frontier.at(--result).first == stylename)
				return result;
			else if (not result)
				throw Nonexistent();

		return 0;
	}
	uint8_t flag_style_index    (char const * const stylename) const {
		for (uint8_t result = m_anim_flag.size();;)
			if (m_anim_flag.at(--result).first == stylename)
				return result;
			else if (not result)
				throw Nonexistent();

		return 0;
	}
	uint8_t next_frontier_style_index(uint8_t i) const {
		return ++i == m_anim_frontier.size() ? 0 : i;
	}
	uint8_t next_flag_style_index    (uint8_t i) const {
		return ++i == m_anim_flag    .size() ? 0 : i;
	}
	const std::string & frontier_style_name (uint8_t const i) const {
		return m_anim_frontier.at(i).first;
	}
	const std::string & flag_style_name     (uint8_t const i) const {
		return m_anim_flag    .at(i).first;
	}
	uint32_t frontier_animation  (uint8_t const i) const {
		return m_anim_frontier.at(i).second;
	}
	uint32_t flag_animation      (uint8_t const i) const {
		return m_anim_flag    .at(i).second;
	}

	uint32_t get_bob_vision_range() const {return m_bob_vision_range;}

	uint32_t get_resource_indicator
		(const Resource_Descr * const res, const uint32_t amount) const;

	void postload(Editor_Game_Base &);
	void load_graphics();

	Military_Data get_military_data() const {return m_military_data;}

	struct Initialization {
		std::string          script;
		std::string          descname;
	};
	typedef std::vector<Initialization> Initializations;
	const Initialization & initialization(uint8_t const index) const {
		if (m_initializations.size() <= index)
			throw Nonexistent();
		return m_initializations[index];
	}

	typedef std::vector<std::vector<Widelands::Ware_Index> > WaresOrder;
	typedef std::vector<std::pair<uint32_t, uint32_t> > WaresOrderCoords;
	const WaresOrder & wares_order() const {return m_wares_order;}
	const WaresOrderCoords & wares_order_coords() const {
		return m_wares_order_coords;
	}

	const WaresOrder & workers_order() const {return m_workers_order;}
	const WaresOrderCoords & workers_order_coords() const {
		return m_workers_order_coords;
	}

	void resize_ware_orders(size_t maxLength);

private:
	const std::string m_name;
	const World & m_world;
	AnimationStyles   m_anim_frontier;
	AnimationStyles   m_anim_flag;
	uint32_t m_bob_vision_range;

	IndexedDescriptionMaintainer<Worker_Descr, Ware_Index>    m_workers;
	IndexedDescriptionMaintainer<Building_Descr, Building_Index>  m_buildings;
	IndexedDescriptionMaintainer<WareDescr, Ware_Index> m_wares;
	DescriptionMaintainer<Immovable_Descr> m_immovables;  // The player immovables
	DescriptionMaintainer<BobDescr>      m_bobs;
	std::string                       m_carrier2;
	// Order and positioning of wares in the warehouse display
	WaresOrder                        m_wares_order;
	WaresOrderCoords                  m_wares_order_coords;
	WaresOrder                        m_workers_order;
	WaresOrderCoords                  m_workers_order_coords;

	std::vector<Ware_Index> m_worker_types_without_cost;

	Initializations m_initializations;

	Military_Data   m_military_data;
};

}

#endif
