/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#ifndef TRIBE_H
#define TRIBE_H

#include "animation.h"
#include "bob.h"
#include "building.h"
#include "descr_maintainer.h"
#include "immovable.h"
#include "item_ware_descr.h"
#include "worker.h"
#include "HTMLReferences.h"

#include "filewrite.h"
#include "TribeBasicInfo.h"

#include <map>
#include <vector>

namespace Widelands {

struct Building_Descr;
struct Editor_Game_Base;
struct Item_Ware_Descr;
struct Resource_Descr;
class Worker_Descr;
class Warehouse;
struct World;

/*
Tribes
------

Every player chooses a tribe. A tribe has distinct properties such as the
buildings it can build and the associated graphics.
Two players can choose the same tribe.
*/
struct Tribe_Descr {
	enum {
		OK = 0,
		ERR_WRONGVERSION
	};

	Tribe_Descr(std::string const & name, Editor_Game_Base &);

	//  Static function to check for tribes.
	static bool exists_tribe
		(std::string const & name, TribeBasicInfo * info = 0);
	static void get_all_tribenames(std::vector<std::string> &);
	static void get_all_tribe_infos(std::vector<TribeBasicInfo> &);


	const std::string & name() const throw () {return m_name;}
	__attribute__ ((deprecated)) const std::string & get_name() const throw () {return m_name;}
	const World & world() const throw () {return m_world;}

	Ware_Index get_nrworkers() const {return m_workers.get_nitems();}
	Worker_Descr * get_worker_descr(Ware_Index const index) const {
		return m_workers.get(index);
	}
	Ware_Index worker_index(std::string const & workername) const {
		return m_workers.get_index(workername);
	}
	Ware_Index worker_index(char const * const workername) const {
		return m_workers.get_index(workername);
	}
	Ware_Index get_nrwares() const {return m_wares.get_nitems();}
	Ware_Index safe_ware_index(std::string const & warename) const;
	Ware_Index safe_ware_index(const char * const warename) const;
	Ware_Index ware_index(std::string const & warename) const {
		return m_wares.get_index(warename);
	}
	Ware_Index ware_index(char const * const warename) const {
		return m_wares.get_index(warename);
	}
	Item_Ware_Descr * get_ware_descr(Ware_Index const index) const {
		return m_wares.get(index);
	}
	Ware_Index safe_worker_index(std::string const & workername) const;
	Ware_Index safe_worker_index(const char * const workername) const;
	Building_Index get_nrbuildings() const {
		return m_buildings.get_nitems();
	}
	Building_Index safe_building_index(const char *name) const;
	Building_Descr * get_building_descr(Building_Index const index) const {
		return m_buildings.get(index);
	}
	Building_Index building_index(std::string const & buildingname) const {
		return m_buildings.get_index(buildingname);
	}
	Building_Index building_index(char const * const buildingname) const {
		return m_buildings.get_index(buildingname);
	}
	int32_t get_immovable_index(char const * l) const {return m_immovables.get_index(l);}
	int32_t get_nr_immovables() {return m_immovables.get_nitems();}
	Immovable_Descr * get_immovable_descr(int32_t const index) const {
		return m_immovables.get(index);
	}
	int32_t get_bob(char const * const l) const {return m_bobs.get_index(l);}
	Bob::Descr * get_bob_descr(uint16_t const index) const {
		return m_bobs.get(index);
	}
	int32_t get_nr_bobs() {return m_bobs.get_nitems();}

	uint32_t get_frontier_anim() const throw () {return m_anim_frontier;}
	uint32_t get_flag_anim    () const throw () {return m_anim_flag;}

	uint32_t get_bob_vision_range() const {return m_bob_vision_range;}

	uint32_t get_resource_indicator
		(const Resource_Descr * const res, const uint32_t amount) const;

	void postload(Editor_Game_Base &);
	void load_graphics();

	struct Initialization {
		std::string          name;
		std::string          descname;
		std::vector<Event *> events;
	};
	typedef std::vector<Initialization> Initializations;
	Initialization const & initialization(uint8_t const index) const {
		assert(index < m_initializations.size());
		return m_initializations[index];
	}

#ifdef WRITE_GAME_DATA_AS_HTML
	void referenceBuilding
		(::FileWrite &, std::string const &, HTMLReferences::Role,
		 Building_Index)
		const;
	void referenceWorker
		(::FileWrite &, std::string const &, HTMLReferences::Role,
		 Ware_Index,     uint8_t multiplicity = 1)
		const;
	void referenceWare
		(::FileWrite &, std::string const &, HTMLReferences::Role,
		 Ware_Index,     uint8_t multiplicity = 1)
		const;
#endif

private:
	const std::string m_name;
	const World & m_world;
	uint32_t m_anim_frontier;
	uint32_t m_anim_flag;
	uint32_t m_bob_vision_range;

	Indexed_Descr_Maintainer<Worker_Descr, Ware_Index>    m_workers;
	Indexed_Descr_Maintainer<Building_Descr, Building_Index>  m_buildings;
	Indexed_Descr_Maintainer<Item_Ware_Descr, Ware_Index> m_wares;
	Descr_Maintainer<Immovable_Descr> m_immovables;  // The player immovables
	Descr_Maintainer<Bob::Descr>      m_bobs;  // The player critters

	Initializations m_initializations;

	EncodeData m_default_encdata;

#ifdef WRITE_GAME_DATA_AS_HTML
	void writeHTMLBuildings(std::string const & directory);
	void writeHTMLWorkers  (std::string const & directory);
	void writeHTMLWares    (std::string const & directory);
	void writeHTMLBobs     (std::string const & directory);
	HTMLReferences * m_ware_references;
	HTMLReferences * m_worker_references;
	HTMLReferences * m_building_references;
#endif

	Tribe_Descr & operator= (Tribe_Descr const &);
	explicit Tribe_Descr    (Tribe_Descr const &);
};

};

#endif
