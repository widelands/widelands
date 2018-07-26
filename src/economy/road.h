/*
 * Copyright (C) 2004-2018 by the Widelands Development Team
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
#include "economy/roadbase.h"
#include "logic/path.h"
#include "logic/roadtype.h"

namespace Widelands {

class RoadDescr : public RoadBaseDescr {
public:
	explicit RoadDescr(char const* const init_name, char const* const init_descname)
	   : RoadBaseDescr(init_name, init_descname, MapObjectType::ROAD) {
	}
	~RoadDescr() override {
	}

private:
	DISALLOW_COPY_AND_ASSIGN(RoadDescr);
};

// C++11 allows static constexpr members in the compiler, but the linker can't handle it.
// So, we don't have these in the class.
// https://stackoverflow.com/questions/40690260/undefined-reference-error-for-static-constexpr-member
constexpr int32_t kRoadAnimalPrice = 600;
constexpr int32_t kRoadMaxWallet = static_cast<int32_t>(2.5 * kRoadAnimalPrice);

struct Road : public RoadBase {
	friend class MapRoaddataPacket;  // For saving
	friend class MapRoadPacket;      // For init()

	static bool is_road_descr(MapObjectDescr const*);

	explicit Road();

	static Road& create(EditorGameBase&, Flag& start, Flag& end, const Path&);

	void presplit(Game&, Coords split);
	void postsplit(Game&, Flag&);

	void update_wallet_chargetime(Game& game);
	void charge_wallet(Game& game);
	int32_t wallet() const;
	void add_to_wallet(int32_t sum);
	void pay_for_road(Game& game, uint8_t wares_count);
	void pay_for_building();

	void log_general_info(const EditorGameBase&) override;

private:
	/// Counter that is incremented when a ware does not get a carrier for this
	/// road immediately and decremented over time.
	int32_t wallet_;

	/// holds the gametime when wallet_ was last charged
	uint32_t last_wallet_charge_;
};
}

#endif  // end of include guard: WL_ECONOMY_ROAD_H
