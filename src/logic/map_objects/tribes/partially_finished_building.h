/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_PARTIALLY_FINISHED_BUILDING_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_PARTIALLY_FINISHED_BUILDING_H

#include "logic/map_objects/tribes/building.h"

namespace Widelands {

class Request;
class WaresQueue;

/*
PartiallyFinishedBuilding
---------------------------

This class is the base for constructionsites and
dismantlesites.
*/
class PartiallyFinishedBuilding : public Building {
	friend class MapBuildingdataPacket;
	friend struct MapBuildingPacket;

public:
	explicit PartiallyFinishedBuilding(const BuildingDescr& building_descr);

	virtual void set_building(const BuildingDescr&);

	int32_t get_size() const override;
	uint32_t get_playercaps() const override;
	const Image* representative_image() const override;
	void cleanup(EditorGameBase&) override;
	bool init(EditorGameBase&) override;
	void set_economy(Economy*, WareWorker) override;

	uint32_t get_nrwaresqueues() {
		return wares_.size();
	}
	WaresQueue* get_waresqueue(uint32_t const idx) {
		return wares_[idx];
	}

	uint32_t get_built_per64k() const;
	Request* get_builder_request() {
		return builder_request_;
	}
	static void
	request_builder_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);

private:
	void request_builder(Game&);

	virtual uint32_t build_step_time() const = 0;

protected:
	const BuildingDescr* building_;  // type of building that was or will become

	Request* builder_request_;
	OPtr<Worker> builder_;

	using Wares = std::vector<WaresQueue*>;
	Wares wares_;

	bool working_;             // true if the builder is currently working
	uint32_t work_steptime_;   // time when next step is completed
	uint32_t work_completed_;  // how many steps have we done so far?
	uint32_t work_steps_;      // how many steps (= wares) until we're done?
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_PARTIALLY_FINISHED_BUILDING_H
