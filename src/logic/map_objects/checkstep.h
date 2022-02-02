/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_CHECKSTEP_H
#define WL_LOGIC_MAP_OBJECTS_CHECKSTEP_H

#include <memory>
#include <set>
#include <vector>

#include "logic/widelands_geometry.h"

namespace Widelands {

class EditorGameBase;
class Map;
class Player;

struct CheckStep {
	enum StepId {
		stepNormal,  //  normal step treatment
		stepFirst,   //  first step of a path
		stepLast,    //  last step of a path
	};

private:
	struct BaseCapsule {
		virtual ~BaseCapsule() {
		}
		virtual bool allowed(
		   const Map&, const FCoords& start, const FCoords& end, int32_t dir, StepId id) const = 0;
		virtual bool reachable_dest(const Map&, const FCoords& dest) const = 0;
	};
	template <typename T> struct Capsule : public BaseCapsule {
		Capsule(const T& init_op) : op(init_op) {
		}

		bool allowed(const Map& map,
		             const FCoords& start,
		             const FCoords& end,
		             int32_t const dir,
		             StepId const id) const override {
			return op.allowed(map, start, end, dir, id);
		}
		bool reachable_dest(const Map& map, const FCoords& dest) const override {
			return op.reachable_dest(map, dest);
		}

		const T op;
	};

	std::shared_ptr<BaseCapsule> capsule;

	static const CheckStep& always_false();

public:
	CheckStep();

	template <typename T> CheckStep(const T& op) : capsule(new Capsule<T>(op)) {
	}

	/**
	 * \return \c true true if moving from start to end (single step in the given
	 * direction) is allowed.
	 */
	bool allowed(const Map& map,
	             const FCoords& start,
	             const FCoords& end,
	             int32_t const dir,
	             StepId const id) const {
		return capsule->allowed(map, start, end, dir, id);
	}

	/**
	 * \return \c true if the destination field can be reached at all
	 * (e.g. return false for land-based bobs when dest is in water).
	 */
	bool reachable_dest(const Map& map, const FCoords& dest) const {
		return capsule->reachable_dest(map, dest);
	}
};

/**
 * CheckStep implementation that returns the logic and of all
 * sub-implementations that have been added via \ref add().
 */
struct CheckStepAnd {
	void add(const CheckStep& sub);

	bool allowed(const Map&,
	             const FCoords& start,
	             const FCoords& end,
	             int32_t dir,
	             CheckStep::StepId id) const;
	bool reachable_dest(const Map&, const FCoords& dest) const;

private:
	std::vector<CheckStep> subs;
};

/**
 * Implements the default step checking behaviours that should be used for all
 * normal bobs.
 *
 * Simply check whether the movecaps are matching (basic exceptions for water
 * bobs moving onto the shore).
 */
struct CheckStepDefault {
	explicit CheckStepDefault(uint8_t const movecaps) : movecaps_(movecaps) {
	}

	bool allowed(
	   const Map&, const FCoords& start, const FCoords& end, int32_t dir, CheckStep::StepId) const;
	bool reachable_dest(const Map&, const FCoords& dest) const;

private:
	uint8_t movecaps_;
};

/**
 * Implements the step checking behaviour for ferries.
 *
 * A ferry can travel on an edge if and only if both adjacent triangles are water.
 */
struct CheckStepFerry {
	explicit CheckStepFerry(const EditorGameBase& egbase) : egbase_(egbase) {
	}

	bool allowed(
	   const Map&, const FCoords& start, const FCoords& end, int32_t dir, CheckStep::StepId) const;
	bool reachable_dest(const Map&, const FCoords& dest) const;

private:
	const EditorGameBase& egbase_;
};

/**
 * Implements the default step checking behaviours with one exception: we can
 * move from a walkable field onto an unwalkable one.
 * If onlyend is true, we can only do this on the final step.
 */
struct CheckStepWalkOn {
	CheckStepWalkOn(uint8_t const movecaps, bool const onlyend)
	   : movecaps_(movecaps), onlyend_(onlyend) {
	}

	bool allowed(
	   const Map&, const FCoords& start, const FCoords& end, int32_t dir, CheckStep::StepId) const;
	bool reachable_dest(const Map&, FCoords dest) const;

private:
	uint8_t movecaps_;
	bool onlyend_;
};

/**
 * Implements the step checking behaviour for road building.
 *
 * player is the player who is building the road.
 * movecaps are the capabilities with which the road is to be built (swimming
 * for boats, walking for normal roads).
 */
struct CheckStepRoad {
	CheckStepRoad(const Player& player, uint8_t const movecaps)
	   : player_(player), movecaps_(movecaps) {
	}

	bool allowed(
	   const Map&, const FCoords& start, const FCoords& end, int32_t dir, CheckStep::StepId) const;
	bool reachable_dest(const Map&, const FCoords& dest) const;

private:
	const Player& player_;
	uint8_t movecaps_;
};

/**
 * A version of CheckStep that is limited to a set of allowed locations. It
 * only checks whether the target is an allowed location.
 */
struct CheckStepLimited {
	void add_allowed_location(const Coords& c) {
		allowed_locations_.insert(c);
	}
	bool allowed(
	   const Map&, const FCoords& start, const FCoords& end, int32_t dir, CheckStep::StepId) const;
	bool reachable_dest(const Map&, FCoords dest) const;

private:
	// The ordering of the set does not matter, as long as it is system
	// independent (for parallel simulation).
	// The only thing that matters is whether a location is in the set.
	std::set<Coords> allowed_locations_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_CHECKSTEP_H
