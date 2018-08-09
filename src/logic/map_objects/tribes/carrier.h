/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_CARRIER_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_CARRIER_H

#include "base/macros.h"
#include "logic/map_objects/tribes/worker.h"

namespace Widelands {
class PendingWare;

class CarrierDescr : public WorkerDescr {
public:
	CarrierDescr(const std::string& init_descname,
	             const LuaTable& table,
	             const EditorGameBase& egbase);
	~CarrierDescr() override {
	}

protected:
	Bob& create_object() const override;

private:
	DISALLOW_COPY_AND_ASSIGN(CarrierDescr);
};

/**
 * Carrier is a worker who is employed by a Road.
 */
struct Carrier : public Worker {
	friend struct MapBobdataPacket;

	MO_DESCR(CarrierDescr)

	explicit Carrier(const CarrierDescr& carrier_descr)
	   : Worker(carrier_descr), operation_(NO_OPERATION) {
	}
	~Carrier() override {
	}

	bool notify_ware(Game&, int32_t flag);

	void start_task_road(Game&);
	void update_task_road(Game&);
	void start_task_transport(Game&, int32_t fromflag);
	bool start_task_walktoflag(Game&, int32_t flag, bool offset = false);

	void log_general_info(const EditorGameBase&) override;

	static Task const taskRoad;

private:
	int32_t find_source_flag(Game&);
	int32_t find_closest_flag(Game&);

	// internal task stuff
	void road_update(Game&, State&);
	void road_pop(Game&, State&);
	void transport_update(Game&, State&);

	static Task const taskTransport;

	void deliver_to_building(Game&, State&);

	// This should be an enum, but this clutters the code with too many casts
	static const int32_t INIT = -3;          // ready to undertake or resume operations
	static const int32_t WAIT = -2;          // waiting for flag capacity
	static const int32_t NO_OPERATION = -1;  // idling
	static const int32_t START_FLAG = 0;     // serving start flag of road
	static const int32_t END_FLAG = 1;       // serving end flag of road
	int32_t operation_;

	// saving and loading
protected:
	struct Loader : public Worker::Loader {
	public:
		Loader();

		void load(FileRead&) override;

	protected:
		const Task* get_task(const std::string& name) override;
	};

	Loader* create_loader() override;

public:
	void do_save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_CARRIER_H
