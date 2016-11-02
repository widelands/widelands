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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_DISMANTLESITE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_DISMANTLESITE_H

#include "base/macros.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/partially_finished_building.h"
#include "logic/player.h"
#include "scripting/lua_table.h"

namespace Widelands {

class Building;

/*
DismantleSite
-------------
The dismantlesite is a way to get some resources back when destroying buildings.
It has a builder as worker and will show the idle image of the original
building which gets smaller and smaller.

Every tribe has exactly one DismantleSite_Descr.
The DismantleSite_Descr's idling animation is remaining graphic that is shown under
the destructed building.
*/
class DismantleSite;

class DismantleSiteDescr : public BuildingDescr {
public:
	DismantleSiteDescr(const std::string& init_descname,
	                   const LuaTable& t,
	                   const EditorGameBase& egbase);
	~DismantleSiteDescr() override {
	}

	Building& create_object() const override;

private:
	DISALLOW_COPY_AND_ASSIGN(DismantleSiteDescr);
};

class DismantleSite : public PartiallyFinishedBuilding {
	friend class MapBuildingdataPacket;

	static const uint32_t DISMANTLESITE_STEP_TIME = 45000;

	MO_DESCR(DismantleSiteDescr)

public:
	DismantleSite(const DismantleSiteDescr& descr);
	DismantleSite(const DismantleSiteDescr& descr,
	              EditorGameBase&,
	              Coords const,
	              Player&,
	              bool,
	              Building::FormerBuildings& former_buildings);

	bool burn_on_destroy() override;
	void init(EditorGameBase&) override;

	bool get_building_work(Game&, Worker&, bool success) override;

	static const Buildcost count_returned_wares(Building* building);

protected:
	void update_statistics_string(std::string*) override;

	uint32_t build_step_time() const override {
		return DISMANTLESITE_STEP_TIME;
	}

	void draw(uint32_t gametime,
	          TextToDraw draw_text,
	          const Vector2f& point_on_dst,
				 float scale,
	          RenderTarget* dst) override;
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_DISMANTLESITE_H
