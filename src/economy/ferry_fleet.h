/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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

#ifndef WL_ECONOMY_FERRY_FLEET_H
#define WL_ECONOMY_FERRY_FLEET_H

#include "base/macros.h"
#include "logic/map_objects/map_object.h"

namespace Widelands {

struct Ferry;
struct Waterway;

class FerryFleetDescr : public MapObjectDescr {
public:
	FerryFleetDescr(char const* const init_name, char const* const init_descname)
	   : MapObjectDescr(MapObjectType::FERRY_FLEET, init_name, init_descname) {
	}
	~FerryFleetDescr() override {
	}

private:
	DISALLOW_COPY_AND_ASSIGN(FerryFleetDescr);
};

/**
 * Manage all ferries and waterways of a player that are connected by ocean.
 *
 * @paragraph Lifetime
 *
 * Fleet objects are created on-the-fly by @ref Ferry and @ref Waterway,
 * and destroy themselves when they become empty.
 *
 * The intention is for fleet objects to merge automatically and separate
 * again in reaction to changes in the map. However, this may not work
 * properly at the moment.
 */
struct FerryFleet : MapObject {
	const FerryFleetDescr& descr() const;

	explicit FerryFleet(Player* player);

	bool active() const;

	bool init(EditorGameBase&) override;
	bool init(EditorGameBase&, Waterway*);
	void cleanup(EditorGameBase&) override;
	void update(EditorGameBase&, const Duration& tdelta = Duration(100));

	void add_ferry(Ferry* ferry);
	void remove_ferry(EditorGameBase& egbase, Ferry* ferry);

	void log_general_info(const EditorGameBase&) const override;

	uint32_t count_ferries() const;
	uint32_t count_unattended_waterways() const;
	bool has_ferry(const Waterway& ww) const;

	void request_ferry(const EditorGameBase& egbase, Waterway* waterway, const Time&);
	void reroute_ferry_request(Game& game, Waterway* oldww, Waterway* newww);
	void cancel_ferry_request(Game& game, Waterway* waterway);

	bool empty() const;

protected:
	void act(Game&, uint32_t data) override;

private:
	bool find_other_fleet(EditorGameBase& egbase);
	bool merge(EditorGameBase& egbase, FerryFleet* other);

	std::vector<Ferry*> ferries_;
	std::multimap<Time, Waterway*> pending_ferry_requests_;

	bool act_pending_;

	// saving and loading
protected:
	struct Loader : MapObject::Loader {
		Loader() = default;

		void load(FileRead&);
		void load_pointers() override;

	private:
		std::vector<uint32_t> ferries_;
		std::multimap<Time, uint32_t> pending_ferry_requests_;
	};

public:
	bool has_new_save_support() override {
		return true;
	}
	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;

	static MapObject::Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_FERRY_FLEET_H
