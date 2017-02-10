/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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

#ifndef WL_ECONOMY_ROAD_H
#define WL_ECONOMY_ROAD_H

#include <vector>

#include "base/macros.h"
#include "logic/map_objects/immovable.h"
#include "logic/path.h"
#include "logic/roadtype.h"

namespace Widelands {
struct Carrier;
class Request;

class RoadDescr : public MapObjectDescr {
public:
	RoadDescr(char const* const init_name, char const* const init_descname)
	   : MapObjectDescr(MapObjectType::ROAD, init_name, init_descname) {
	}
	~RoadDescr() override {
	}

private:
	DISALLOW_COPY_AND_ASSIGN(RoadDescr);
};

/**
 * Road is a special object which connects two flags.
 * The Road itself is never rendered; however, the appropriate Field::roads are
 * set to represent the road visually.
 * The actual steps involved in a road are stored as a Path from the starting
 * flag to the ending flag. Apart from that, however, the two flags are treated
 * exactly the same, as far as most transactions are concerned. There are minor
 * exceptions: placement of carriers if the path's length is odd, splitting
 * a road when a flag is inserted.
 *
 * Every road has one or more Carriers attached to it. Carriers are attached
 * when they arrive via the callback function passed to the request. The
 * callback then calls assign_carrier which incorporates this carrier on this
 * road.
 */
struct Road : public PlayerImmovable {
	friend class MapRoaddataPacket;  // For saving
	friend class MapRoadPacket;      // For init()

	const RoadDescr& descr() const;

	static bool is_road_descr(MapObjectDescr const*);

	enum FlagId { FlagStart = 0, FlagEnd = 1 };

	struct CarrierSlot {
		CarrierSlot();

		OPtr<Carrier> carrier;
		Request* carrier_request;
		uint8_t carrier_type;
	};

	Road();
	virtual ~Road();

	static Road& create(EditorGameBase&, Flag& start, Flag& end, const Path&);

	Flag& get_flag(FlagId const flag) const {
		return *flags_[flag];
	}

	uint8_t get_roadtype() const {
		return type_;
	}
	int32_t get_size() const override;
	bool get_passable() const override;
	PositionList get_positions(const EditorGameBase&) const override;

	Flag& base_flag() override;

	void set_economy(Economy*) override;

	int32_t get_cost(FlagId fromflag);
	const Path& get_path() const {
		return path_;
	}
	int32_t get_idle_index() const {
		return idle_index_;
	}

	void presplit(Game&, Coords split);
	void postsplit(Game&, Flag&);

	bool notify_ware(Game& game, FlagId flagid);

	void remove_worker(Worker&) override;
	void assign_carrier(Carrier&, uint8_t);

	void log_general_info(const EditorGameBase&) override;

protected:
	void init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

	void draw(uint32_t gametime,
	          TextToDraw draw_text,
	          const Vector2f& point_on_dst,
	          float scale,
	          RenderTarget* dst) override;

private:
	void set_path(EditorGameBase&, const Path&);

	void mark_map(EditorGameBase&);
	void unmark_map(EditorGameBase&);

	void link_into_flags(EditorGameBase&);

	void request_carrier(CarrierSlot&);
	static void
	request_carrier_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);

private:
	/// Counter that is incremented when a ware does not get a carrier for this
	/// road immediately and decremented over time.
	uint32_t busyness_;

	/// holds the gametime when busyness_ was last updated
	uint32_t busyness_last_update_;

	uint8_t type_;        ///< RoadType, 2 bits used
	Flag* flags_[2];      ///< start and end flag
	int32_t flagidx_[2];  ///< index of this road in the flag's road array

	/// cost for walking this road (0 = from start to end, 1 = from end to start)
	int32_t cost_[2];

	Path path_;            ///< path goes from start to end
	uint32_t idle_index_;  ///< index into path where carriers should idle

	using SlotVector = std::vector<CarrierSlot>;
	SlotVector carrier_slots_;
};
}

#endif  // end of include guard: WL_ECONOMY_ROAD_H
