/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#ifndef WL_ECONOMY_FLAG_H
#define WL_ECONOMY_FLAG_H

#include <algorithm>
#include <deque>
#include <iterator>

#include "base/macros.h"
#include "economy/flag_job.h"
#include "economy/routing_node.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/info_to_draw.h"
#include "logic/map_objects/walkingdir.h"

namespace Widelands {
struct RoadBase;
struct Road;
struct Waterway;
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
 * Flag represents a flag as you see it on the map.
 *
 * A flag itself doesn't do much. However, it can have up to 6 roads/waterways
 * attached to it. Instead of the WALK_NW road, it can also have a building
 * attached to it. It cannot have more than one waterway.
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

	/// Empty flag, for unit tests only.
	Flag();

	/// Create a new flag. Only specify an economy during saveloading.
	/// Otherwise, a new economy will be created automatically if needed.
	Flag(EditorGameBase&,
	     Player* owner,
	     const Coords&,
	     Economy* ware_economy = nullptr,
	     Economy* worker_economy = nullptr);
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

	void set_economy(Economy*, WareWorker) override;

	Building* get_building() const {
		return building_;
	}
	void attach_building(EditorGameBase&, Building&);
	void detach_building(EditorGameBase&);

	bool has_roadbase() const {
		return std::any_of(std::begin(roads_), std::end(roads_),
		                   [](const RoadBase* road) { return road != nullptr; });
	}
	bool has_waterway() const {
		return nr_of_waterways() > 0;
	}
	bool has_road() const {
		return nr_of_roads() > 0;
	}
	RoadBase* get_roadbase(uint8_t dir) const {
		return roads_[dir - 1];
	}
	Road* get_road(uint8_t dir) const;
	Waterway* get_waterway(uint8_t dir) const;
	uint8_t nr_of_roadbases() const;
	uint8_t nr_of_roads() const;
	uint8_t nr_of_waterways() const;
	void attach_road(int32_t dir, RoadBase*);
	void detach_road(int32_t dir);

	RoadBase* get_roadbase(Flag&);
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
	void propagate_promoted_road(Road* promoted_road);
	Wares get_wares();
	uint8_t count_wares_in_queue(PlayerImmovable& dest) const;

	void call_carrier(Game&, WareInstance&, PlayerImmovable* nextstep);
	void update_wares(Game&, Flag* other);

	void remove_ware(EditorGameBase&, WareInstance* const);

	void log_general_info(const EditorGameBase&) const override;

	/**
	 * Clamp the maximal value of \ref PendingWare::priority.
	 * After reaching this value, the pure FIFO approach is applied
	 */
	static constexpr uint8_t kMaxTransferPriority = 16;

	void add_flag_job(Game&, FlagJob::Type);

	void act(Game&, uint32_t) override;

	void receive_worker(Game&, Worker&) override;

protected:
	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

	void draw(const Time& gametime,
	          InfoToDraw info_to_draw,
	          const Vector2f& point_on_dst,
	          const Coords& coords,
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

	Coords position_;
	Time animstart_;

	Building* building_;  ///< attached building (replaces road WALK_NW)
	RoadBase* roads_[WalkingDir::LAST_DIRECTION];

	int32_t ware_capacity_;  ///< size of wares_ array
	int32_t ware_filled_;    ///< number of wares currently on the flag
	PendingWare* wares_;     ///< wares currently on the flag

	/// call_carrier() will always call a carrier when the destination is
	/// the given flag
	Flag* always_call_for_flag_;

	using CapacityWaitQueue = std::deque<OPtr<Worker>>;
	CapacityWaitQueue capacity_wait_;  ///< workers waiting for capacity

	using FlagJobs = std::list<FlagJob>;
	FlagJobs flag_jobs_;
	bool act_pending_;
	void do_schedule_act(Game&, const Duration&);
};

extern FlagDescr g_flag_descr;
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_FLAG_H
