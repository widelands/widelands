/*
 * Copyright (C) 2002-2004, 2006-2009, 2011 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_CONSTRUCTIONSITE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_CONSTRUCTIONSITE_H

#include <vector>

#include "base/macros.h"
#include "logic/map_objects/tribes/partially_finished_building.h"
#include "scripting/lua_table.h"

namespace Widelands {

class Building;
class Request;
class WaresQueue;

/// Per-player and per-field constructionsite information
struct ConstructionsiteInformation {
	ConstructionsiteInformation() : becomes(nullptr), was(nullptr), totaltime(0), completedtime(0) {
	}
	const BuildingDescr*
	   becomes;  // Also works as a marker telling whether there is a construction site.
	const BuildingDescr* was;  // only valid if "becomes" is an enhanced building.
	uint32_t totaltime;
	uint32_t completedtime;
};

/*
ConstructionSite
----------------
A "building" that acts as a placeholder during construction work.
Once the construction phase is finished, the ConstructionSite is removed
and replaced by the actual building.

A construction site can have a worker.
A construction site has one (or more) input wares types, each with an
  associated store.

Note that the ConstructionSiteDescr class is mostly a dummy class.
The ConstructionSite is derived from Building so that it fits in more cleanly
with the transport and Flag code.

Every tribe has exactly one ConstructionSiteDescr.
The ConstructionSite's idling animation is the basic construction site marker.
*/
class ConstructionSiteDescr : public BuildingDescr {
public:
	ConstructionSiteDescr(const std::string& init_descname,
	                      const LuaTable& t,
	                      const EditorGameBase& egbase);
	~ConstructionSiteDescr() override {
	}

	Building& create_object() const override;

private:
	DISALLOW_COPY_AND_ASSIGN(ConstructionSiteDescr);
};

class ConstructionSite : public PartiallyFinishedBuilding {
	friend class MapBuildingdataPacket;

	static const uint32_t CONSTRUCTIONSITE_STEP_TIME = 30000;

	MO_DESCR(ConstructionSiteDescr)

public:
	ConstructionSite(const ConstructionSiteDescr& descr);

	const ConstructionsiteInformation& get_info() {
		return info_;
	}

	WaresQueue& waresqueue(DescriptionIndex) override;

	void set_building(const BuildingDescr&) override;
	const BuildingDescr& building() const {
		return *building_;
	}

	void init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

	bool burn_on_destroy() override;

	bool fetch_from_flag(Game&) override;
	bool get_building_work(Game&, Worker&, bool success) override;

protected:
	void update_statistics_string(std::string* statistics_string) override;

	uint32_t build_step_time() const override {
		return CONSTRUCTIONSITE_STEP_TIME;
	}
	void create_options_window(InteractiveGameBase&, UI::Window*& registry) override;

	static void wares_queue_callback(Game&, WaresQueue*, DescriptionIndex, void* data);

	void draw(const EditorGameBase&, RenderTarget&, const FCoords&, const Point&) override;

private:
	int32_t fetchfromflag_;  // # of wares to fetch from flag

	bool builder_idle_;                 // used to determine whether the builder is idle
	ConstructionsiteInformation info_;  // asked for by player point of view for the gameview
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_CONSTRUCTIONSITE_H
