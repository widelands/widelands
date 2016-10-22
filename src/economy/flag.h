/*
 * Copyright (C) 2004, 2006-2013 by the Widelands Development Team
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

#ifndef WL_ECONOMY_FLAG_H
#define WL_ECONOMY_FLAG_H

#include <list>
#include <vector>

#include "base/macros.h"
#include "economy/routing_node.h"
#include "logic/map_objects/draw_text.h"
#include "logic/map_objects/immovable.h"

namespace Widelands {
class Building;
class Request;
struct Road;
class WareInstance;

class FlagDescr : public MapObjectDescr {
public:
	FlagDescr(char const* const init_name, char const* const init_descname)
	   : MapObjectDescr(MapObjectType::FLAG, init_name, init_descname) {
	}
	~FlagDescr() override {
	}

private:
	DISALLOW_COPY_AND_ASSIGN(FlagDescr);
};

/**
 * Flag represents a flag, obviously.
 * A flag itself doesn't do much. However, it can have up to 6 roads attached
 * to it. Instead of the WALK_NW road, it can also have a building attached to
 * it.
 * Flags also have a store of up to 8 wares.
 *
 * You can also assign an arbitrary number of "jobs" for a flag.
 * A job consists of a request for a worker, and the name of a program that the
 * worker is to execute. Once execution of the program has finished, the worker
 * will return to a warehouse.
 *
 * Important: Do not access roads_ directly. get_road() and others use
 * WALK_xx in all "direction" parameters.
 */
struct Flag : public PlayerImmovable, public RoutingNode {
	using Wares = std::vector<const WareInstance*>;

	friend class Economy;
	friend class FlagQueue;
	friend class MapFlagdataPacket;   // has to read/write this to a file
	friend struct MapWarePacket;      // has to look at pending wares
	friend struct MapWaredataPacket;  // has to look at pending wares
	friend struct Router;

	const FlagDescr& descr() const;

	Flag();                                               /// empty flag for savegame loading
	Flag(EditorGameBase&, Player& owner, const Coords&);  /// create a new flag
	~Flag() override;

	void load_finish(EditorGameBase&) override;
	void destroy(EditorGameBase&) override;

	int32_t get_size() const override;
	bool get_passable() const override;

	Flag& base_flag() override;

	const Coords& get_position() const override {
		return position_;
	}
	PositionList get_positions(const EditorGameBase&) const override;
	void get_neighbours(WareWorker type, RoutingNodeNeighbours&) override;
	int32_t get_waitcost() const {
		return ware_filled_;
	}

	void set_economy(Economy*) override;

	Building* get_building() const {
		return building_;
	}
	void attach_building(EditorGameBase&, Building&);
	void detach_building(EditorGameBase&);

	bool has_road() const {
		return roads_[0] || roads_[1] || roads_[2] || roads_[3] || roads_[4] || roads_[5];
	}
	Road* get_road(uint8_t const dir) const {
		return roads_[dir - 1];
	}
	uint8_t nr_of_roads() const;
	void attach_road(int32_t dir, Road*);
	void detach_road(int32_t dir);

	Road* get_road(Flag&);

	bool is_dead_end() const;

	bool has_capacity() const;
	uint32_t total_capacity() {
		return ware_capacity_;
	}
	uint32_t current_wares() const {
		return ware_filled_;
	}
	void wait_for_capacity(Game&, Worker&);
	void skip_wait_for_capacity(Game&, Worker&);
	void add_ware(EditorGameBase&, WareInstance&);
	bool has_pending_ware(Game&, Flag& destflag);
	bool ack_pickup(Game&, Flag& destflag);
	bool cancel_pickup(Game&, Flag& destflag);
	WareInstance* fetch_pending_ware(Game&, PlayerImmovable& dest);
	Wares get_wares();

	void call_carrier(Game&, WareInstance&, PlayerImmovable* nextstep);
	void update_wares(Game&, Flag* other);

	void remove_ware(EditorGameBase&, WareInstance* const);

	void add_flag_job(Game&, DescriptionIndex workerware, const std::string& programname);

	void log_general_info(const EditorGameBase&) override;

protected:
	void init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

	void draw(uint32_t gametime,
	          DrawText draw_text,
	          const Vector2f& point_on_dst,
				 float scale,
	          RenderTarget* dst) override;

	void wake_up_capacity_queue(Game&);

	static void
	flag_job_request_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);

	void set_flag_position(Coords coords);

private:
	struct PendingWare {
		WareInstance* ware;              ///< the ware itself
		bool pending;                    ///< if the ware is pending
		int32_t priority;                ///< carrier prefers the ware with highest priority
		OPtr<PlayerImmovable> nextstep;  ///< next step that this ware is sent to
	};

	struct FlagJob {
		Request* request;
		std::string program;
	};

	Coords position_;
	int32_t animstart_;

	Building* building_;  ///< attached building (replaces road WALK_NW)
	Road* roads_[6];      ///< WALK_xx - 1 as index

	int32_t ware_capacity_;  ///< size of wares_ array
	int32_t ware_filled_;    ///< number of wares currently on the flag
	PendingWare* wares_;     ///< wares currently on the flag

	/// call_carrier() will always call a carrier when the destination is
	/// the given flag
	Flag* always_call_for_flag_;

	using CapacityWaitQueue = std::vector<OPtr<Worker>>;
	CapacityWaitQueue capacity_wait_;  ///< workers waiting for capacity

	using FlagJobs = std::list<FlagJob>;
	FlagJobs flag_jobs_;
};

extern FlagDescr g_flag_descr;
}

#endif  // end of include guard: WL_ECONOMY_FLAG_H
