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

#ifndef WL_ECONOMY_REQUEST_H
#define WL_ECONOMY_REQUEST_H

#include "base/times.h"
#include "economy/trackptr.h"
#include "logic/map_objects/tribes/requirements.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/widelands.h"

namespace Widelands {

class Economy;
struct Flag;
class Game;
struct PlayerImmovable;
class RequestList;
struct Supply;
struct Transfer;
class Worker;
class Building;
class ProductionSite;
class Warehouse;
class ConstructionSite;

/**
 * A Request is issued whenever some object (road or building) needs a ware.
 *
 * Requests are always created and destroyed by their owner, i.e. the target
 * player immovable. The owner is also responsible for calling set_economy()
 * when its economy changes.
 *
 * Idle Requests need not be fulfilled; however, when there's a matching Supply
 * left, a transfer may be initiated.
 * The required time has no meaning for idle requests.
 */
class Request : public Trackable {
public:
	friend class Economy;
	friend class RequestList;

	using CallbackFn = void (*)(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);
	using TransferList = std::vector<Transfer*>;

	Request(PlayerImmovable& target, DescriptionIndex, CallbackFn, WareWorker);
	~Request() override;

	PlayerImmovable& target() const {
		return target_;
	}
	DescriptionIndex get_index() const {
		return index_;
	}
	WareWorker get_type() const {
		return type_;
	}
	Quantity get_count() const {
		return count_;
	}
	uint32_t get_open_count() const {
		return count_ - transfers_.size();
	}
	bool get_exact_match() const {
		return exact_match_;
	}
	bool is_open() const {
		return transfers_.size() < count_;
	}
	Economy* get_economy() const {
		return economy_;
	}
	Time get_required_time() const;
	const Time& get_last_request_time() const {
		return last_request_time_;
	}
	uint32_t get_priority(int32_t cost) const;
	uint32_t get_normalized_transfer_priority() const;
	uint32_t get_num_transfers() const {
		return transfers_.size();
	}
	const TransferList& get_transfers() const {
		return transfers_;
	}

	Flag& target_flag() const;

	void set_economy(Economy*);
	void set_count(Quantity);
	void set_exact_match(bool match);
	void set_required_time(const Time& time);
	void set_required_interval(const Duration& interval);

	void set_last_request_time(const Time& time) {
		last_request_time_ = time;
	}

	void start_transfer(Game&, Supply&);

	void read(FileRead&, Game&, MapObjectLoader&);
	void write(FileWrite&, Game&, MapObjectSaver&) const;
	Worker* get_transfer_worker();

	//  callbacks for WareInstance/Worker code
	void transfer_finish(Game&, Transfer&);
	void transfer_fail(Game&, Transfer&);
	void cancel_transfer(uint32_t idx);

	void set_requirements(const Requirements& r) {
		requirements_ = r;
	}
	const Requirements& get_requirements() const {
		return requirements_;
	}

private:
	Time get_base_required_time(const EditorGameBase&, uint32_t nr) const;
	void remove_transfer(uint32_t idx);
	uint32_t find_transfer(Transfer&);

	WareWorker type_;

	PlayerImmovable& target_;  //  who requested it?
	//  Copies of target_ of various pointer types, to avoid expensive
	//  dynamic casting at runtime. Variables with an incompatible type
	//  are filled with nulls.
	Building* target_building_;
	ProductionSite* target_productionsite_;
	Warehouse* target_warehouse_;
	ConstructionSite* target_constructionsite_;

	Economy* economy_;
	DescriptionIndex index_;  //  the index of the ware descr
	Quantity count_;          //  how many do we need in total
	bool exact_match_;        // Whether a worker supply has to match exactly
	                          // or if a can_act_as() comparison is good enough

	CallbackFn callbackfn_;  //  called on request success

	//  when do we need the first ware (can be in the past)
	Time required_time_;
	Duration required_interval_;  //  time between wares
	Time last_request_time_;

	TransferList transfers_;  //  maximum size is count_

	Requirements requirements_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_REQUEST_H
