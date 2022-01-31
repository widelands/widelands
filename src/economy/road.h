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

#ifndef WL_ECONOMY_ROAD_H
#define WL_ECONOMY_ROAD_H

#include "base/macros.h"
#include "economy/roadbase.h"
#include "logic/path.h"

namespace Widelands {
class Request;

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
/**
 * Every Road has one or more Carriers attached to it. Carriers are attached
 * when they arrive via the callback function passed to the request. The
 * callback then calls assign_carrier which incorporates this carrier on this
 * Road.
 */
struct Road : public RoadBase {
	friend class MapRoaddataPacket;  // For saving
	friend class MapRoadPacket;      // For init()

	static bool is_road_descr(MapObjectDescr const*);

	explicit Road();
	~Road() override;
	bool init(EditorGameBase&) override;

	static Road& create(EditorGameBase&, Flag& start, Flag& end, const Path&);

	// A CarrierSlot can store a carrier.
	struct CarrierSlot {
		CarrierSlot();

		OPtr<Carrier> carrier;
		Request* carrier_request;
		uint8_t carrier_type_id;
	};

	void postsplit(Game&, Flag&) override;

	void update_wallet_chargetime(const Game& game);
	void charge_wallet(Game& game);
	int32_t wallet() const;
	void add_to_wallet(int32_t sum);
	void pay_for_road(Game& game, uint8_t wares_count);
	void pay_for_building();

	void set_economy(Economy*, WareWorker) override;

	bool notify_ware(Game& game, FlagId flag) override;

	void remove_worker(Worker&) override;
	void assign_carrier(Carrier&, uint8_t) override;

	void log_general_info(const EditorGameBase&) const override;

	bool is_busy() const {
		return busy_;
	}

	// Use in the editor or from Lua scripting only!
	void set_busy(EditorGameBase& e, bool);

protected:
	bool is_bridge(const EditorGameBase&, const FCoords&, uint8_t) const override;
	RoadSegment road_type_for_drawing() const override;

private:
	void cleanup(EditorGameBase&) override;

	void link_into_flags(EditorGameBase&, bool = false) override;

	bool busy_;
	/// Counter that is incremented when a ware does not get a carrier for this
	/// road immediately and decremented over time.
	int32_t wallet_;

	/// holds the gametime when wallet_ was last charged
	Time last_wallet_charge_;

	void request_carrier(CarrierSlot&);
	static void
	request_carrier_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);

	uint8_t carriers_count() const;

	using SlotVector = std::vector<CarrierSlot>;
	SlotVector carrier_slots_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_ROAD_H
