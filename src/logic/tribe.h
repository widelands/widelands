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

#ifndef WL_LOGIC_TRIBE_H
#define WL_LOGIC_TRIBE_H

#include <map>
#include <memory>
#include <vector>

#include "base/macros.h"
#include "graphic/animation.h"
#include "graphic/texture.h"
#include "logic/bob.h"
#include "logic/building.h"
#include "logic/description_maintainer.h"
#include "logic/immovable.h"
#include "logic/road_textures.h"
#include "logic/roadtype.h"
#include "logic/tribe_basic_info.h"
#include "logic/ware_descr.h"
#include "logic/worker.h"

namespace Widelands {

class EditorGameBase;
class ResourceDescription;
class Warehouse;
class WorkerDescr;
class World;
struct BuildingDescr;
struct Event;
struct WareDescr;

/*
Tribes
------

Every player chooses a tribe. A tribe has distinct properties such as the
buildings it can build and the associated graphics.
Two players can choose the same tribe.
*/
class TribeDescr {
public:
	TribeDescr(const std::string & name, EditorGameBase &);

	//  Static function to check for tribes.
	static bool exists_tribe
		(const std::string & name, TribeBasicInfo * info = nullptr);
	static std::vector<std::string> get_all_tribenames();
	static std::vector<TribeBasicInfo> get_all_tribe_infos();


	const std::string& name() const {return m_name;}

	// A vector of all texture images that can be used for drawing a
	// (normal|busy) road. The images are guaranteed to exist.
	const std::vector<std::string>& normal_road_paths() const;
	const std::vector<std::string>& busy_road_paths() const;

	// Add the corresponding texture (which probably resides in a
	// texture atlas) for roads.
	void add_normal_road_texture(std::unique_ptr<Texture> texture);
	void add_busy_road_texture(std::unique_ptr<Texture> texture);

	// The road textures used for drawing roads.
	const RoadTextures& road_textures() const;

	WareIndex get_nrworkers() const {return m_workers.get_nitems();}
	WorkerDescr const * get_worker_descr(const WareIndex& index) const {
		return m_workers.get(index);
	}
	WareIndex worker_index(const std::string & workername) const {
		return m_workers.get_index(workername);
	}
	WareIndex worker_index(char const * const workername) const {
		return m_workers.get_index(workername);
	}
	WareIndex carrier2() const {
		if (m_carrier2.size())
			return worker_index(m_carrier2);
		return worker_index("carrier");
	}
	WareIndex get_nrwares() const {return m_wares.get_nitems();}
	WareIndex safe_ware_index(const std::string & warename) const;
	WareIndex ware_index(const std::string & warename) const;
	WareDescr const * get_ware_descr(const WareIndex& index) const {
		return m_wares.get(index);
	}
	void set_ware_type_has_demand_check(const WareIndex& index) const {
		m_wares.get(index)->set_has_demand_check();
	}
	void set_worker_type_has_demand_check(const WareIndex& index) const {
		m_workers.get(index)->set_has_demand_check();
	}
	WareIndex safe_worker_index(const std::string & workername) const;
	BuildingIndex get_nrbuildings() const {
		return m_buildings.get_nitems();
	}
	BuildingIndex safe_building_index(char const * name) const;
	BuildingDescr const * get_building_descr(const BuildingIndex& index) const
	{
		return m_buildings.get(index);
	}
	BuildingIndex building_index(const std::string & buildingname) const {
		return m_buildings.get_index(buildingname);
	}
	BuildingIndex building_index(char const * const buildingname) const {
		return m_buildings.get_index(buildingname);
	}
	int32_t get_immovable_index(char const * const l) const {
		return m_immovables.get_index(l);
	}
	int32_t get_immovable_index(const std::string & l) const {
		return m_immovables.get_index(l);
	}
	int32_t get_nr_immovables() {return m_immovables.get_nitems();}
	ImmovableDescr const * get_immovable_descr(int32_t const index) const {
		return m_immovables.get(index);
	}
	ImmovableDescr const * get_immovable_descr(const std::string & imm_name) const {
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

	const std::vector<WareIndex> & worker_types_without_cost() const {
		return m_worker_types_without_cost;
	}

	uint32_t frontier_animation() const {
		return m_frontier_animation_id;
	}

	uint32_t flag_animation() const {
		return m_flag_animation_id;
	}

	uint32_t get_bob_vision_range() const {return m_bob_vision_range;}

	uint32_t get_resource_indicator
		(const ResourceDescription * const res, const uint32_t amount) const;

	void postload(EditorGameBase &);
	void load_graphics();

	struct Initialization {
		std::string script;
		std::string descname;
	};

	// Returns the initalization at 'index' (which must not be out of bounds).
	const Initialization & initialization(const uint8_t index) const {
		return m_initializations.at(index);
	}

	using WaresOrder = std::vector<std::vector<Widelands::WareIndex>>;
	using WaresOrderCoords = std::vector<std::pair<uint32_t, uint32_t>>;
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

	std::vector<std::string> m_normal_road_paths;
	std::vector<std::string> m_busy_road_paths;
	RoadTextures m_road_textures;

	uint32_t m_frontier_animation_id;
	uint32_t m_flag_animation_id;
	uint32_t m_bob_vision_range;

	DescriptionMaintainer<WorkerDescr> m_workers;
	DescriptionMaintainer<BuildingDescr> m_buildings;
	DescriptionMaintainer<WareDescr> m_wares;
	DescriptionMaintainer<ImmovableDescr> m_immovables;  // The player immovables
	DescriptionMaintainer<BobDescr> m_bobs;
	std::string                       m_carrier2;
	// Order and positioning of wares in the warehouse display
	WaresOrder                        m_wares_order;
	WaresOrderCoords                  m_wares_order_coords;
	WaresOrder                        m_workers_order;
	WaresOrderCoords                  m_workers_order_coords;

	std::vector<WareIndex> m_worker_types_without_cost;

	std::vector<Initialization> m_initializations;

	DISALLOW_COPY_AND_ASSIGN(TribeDescr);
};

}

#endif  // end of include guard: WL_LOGIC_TRIBE_H
