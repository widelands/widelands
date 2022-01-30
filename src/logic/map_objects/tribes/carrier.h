/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_CARRIER_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_CARRIER_H

#include "base/macros.h"
#include "logic/map_objects/tribes/worker.h"

namespace Widelands {

class CarrierDescr : public WorkerDescr {
public:
	CarrierDescr(const std::string& init_descname,
	             const LuaTable& table,
	             Descriptions& descriptions,
	             MapObjectType t = MapObjectType::CARRIER);
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
	   : Worker(carrier_descr), promised_pickup_to_(NOONE) {
	}
	~Carrier() override {
	}

	bool notify_ware(Game&, int32_t flag);

	void start_task_road(Game&);
	void update_task_road(Game&);
	void start_task_transport(Game&, int32_t fromflag);
	bool start_task_walktoflag(Game&, int32_t flag, bool offset = false);

	void log_general_info(const EditorGameBase&) const override;

	static Task const taskRoad;

private:
	void find_pending_ware(Game&);
	int32_t find_closest_flag(const Game&);

	// internal task stuff
	void road_update(Game&, State&);
	void road_pop(Game&, State&);
	void transport_update(Game&, State&);

	static Task const taskTransport;

	void deliver_to_building(Game&, State&);
	void pickup_from_flag(Game&, const State&);
	void drop_ware(Game&, State&);
	void enter_building(Game&, State&);
	bool swap_or_wait(Game&, State&);

	/// -1: no ware acked; 0/1: acked ware for start/end flag of road
	// This should be an enum, but this clutters the code with too many casts
	static const int32_t NOONE = -1;
	static const int32_t START_FLAG = 0;
	static const int32_t END_FLAG = 1;
	int32_t promised_pickup_to_;

	// saving and loading
protected:
	struct Loader : public Worker::Loader {
	public:
		Loader() = default;

		void load(FileRead&) override;

	protected:
		const Task* get_task(const std::string& name) override;
	};

	Loader* create_loader() override;

public:
	void do_save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_CARRIER_H
