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

#ifndef WL_ECONOMY_ROADBASE_H
#define WL_ECONOMY_ROADBASE_H

#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/road_segments.h"
#include "logic/map_objects/immovable.h"
#include "logic/path.h"

namespace Widelands {
struct Carrier;

class RoadBaseDescr : public MapObjectDescr {
public:
	RoadBaseDescr(char const* const init_name, char const* const init_descname, MapObjectType mot)
	   : MapObjectDescr(mot, init_name, init_descname) {
	}
	~RoadBaseDescr() override {
	}

private:
	DISALLOW_COPY_AND_ASSIGN(RoadBaseDescr);
};

/**
 * RoadBase is a special object which connects two flags.
 * The RoadBase itself is never rendered; however, the appropriate Field::roads are
 * set to represent the RoadBase visually.
 * The actual steps involved in a RoadBase are stored as a Path from the starting
 * flag to the ending flag. Apart from that, however, the two flags are treated
 * exactly the same, as far as most transactions are concerned. There are minor
 * exceptions: placement of carriers if the path's length is odd, splitting
 * a RoadBase when a flag is inserted.
 *
 */
struct RoadBase : public PlayerImmovable {

	enum FlagId { FlagStart = 0, FlagEnd = 1 };

	RoadBase(const RoadBaseDescr& d);

	Flag& get_flag(FlagId const flag) const {
		return *flags_[flag];
	}

	int32_t get_size() const override;
	bool get_passable() const override;
	PositionList get_positions(const EditorGameBase&) const override;

	Flag& base_flag() override;

	int32_t get_cost(FlagId fromflag);
	const Path& get_path() const {
		return path_;
	}
	int32_t get_idle_index() const {
		return idle_index_;
	}

	void presplit(Game&, Coords split);
	virtual void postsplit(Game&, Flag&);

	virtual bool notify_ware(Game&, FlagId);

	virtual void assign_carrier(Carrier&, uint8_t);

protected:
	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

	/** The road is drawn by the terrain renderer via marked fields. */
	void
	draw(const Time&, InfoToDraw, const Vector2f&, const Coords&, float, RenderTarget*) override {
	}

	void set_path(const EditorGameBase&, const Path&);

	void mark_map(EditorGameBase&);
	void unmark_map(EditorGameBase&);

	virtual void link_into_flags(EditorGameBase&, bool = false);

	virtual RoadSegment road_type_for_drawing() const = 0;
	void set_roadtype(EditorGameBase&, FCoords, uint8_t, RoadSegment) const;
	virtual bool is_bridge(const EditorGameBase&, const FCoords&, uint8_t) const {
		return false;
	}

	Flag* flags_[2];      ///< start and end flag
	int32_t flagidx_[2];  ///< index of this road in the flag's road array

	/// cost for walking this road (0 = from start to end, 1 = from end to start)
	int32_t cost_[2];

	Path path_;            ///< path goes from start to end
	uint32_t idle_index_;  ///< index into path where carriers should idle
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_ROADBASE_H
