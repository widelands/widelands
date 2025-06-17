/*
 * Copyright (C) 2023-2025 by the Widelands Development Team
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

#ifndef WL_ECONOMY_SOLDIER_REQUEST_H
#define WL_ECONOMY_SOLDIER_REQUEST_H

#include <functional>
#include <memory>
#include <vector>

#include "base/macros.h"
#include "economy/request.h"
#include "logic/widelands.h"

namespace Widelands {

class Soldier;

class SoldierRequest : public Request {
public:
	SoldierRequest(PlayerImmovable& target,
	               DescriptionIndex di,
	               CallbackFn cb,
	               WareWorker ww,
	               SoldierPreference pref)
	   : Request(target, di, cb, ww), preference_(pref) {
	}

	[[nodiscard]] SoldierPreference get_preference() const {
		return preference_;
	}
	void set_preference(SoldierPreference pref) {
		preference_ = pref;
	}

private:
	SoldierPreference preference_{SoldierPreference::kAny};
};

class SoldierRequestManager {
public:
	using DesiredCapacityFn = std::function<Quantity()>;
	using StationedSoldiersFn = std::function<std::vector<Soldier*>()>;

	SoldierRequestManager(PlayerImmovable& target,
	                      SoldierPreference pref,
	                      Request::CallbackFn callback,
	                      DesiredCapacityFn dcfn,
	                      StationedSoldiersFn ssfn)
	   : target_(target),
	     preference_(pref),
	     callback_(callback),
	     get_desired_capacity_(dcfn),
	     get_stationed_soldiers_(ssfn) {
	}

	void update();

	void set_economy(Economy* e, WareWorker type);
	void set_preference(SoldierPreference p) {
		preference_ = p;
	}

	[[nodiscard]] PlayerImmovable& get_target() const {
		return target_;
	}
	[[nodiscard]] SoldierPreference get_preference() const {
		return preference_;
	}

	[[nodiscard]] const SoldierRequest* get_request() const {
		return request_.get();
	}
	[[nodiscard]] SoldierRequest* get_request() {
		return request_.get();
	}

	void read(FileRead& fr, Game& game, MapObjectLoader& mol);
	void write(FileWrite& fw, Game& game, MapObjectSaver& mos) const;

private:
	void create_request();

	PlayerImmovable& target_;
	SoldierPreference preference_;
	Request::CallbackFn callback_;

	std::unique_ptr<SoldierRequest> request_;

	DesiredCapacityFn get_desired_capacity_;
	StationedSoldiersFn get_stationed_soldiers_;

	DISALLOW_COPY_AND_ASSIGN(SoldierRequestManager);
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_SOLDIER_REQUEST_H
