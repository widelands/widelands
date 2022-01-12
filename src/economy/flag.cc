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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "economy/flag.h"

#include <algorithm>
#include <iterator>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/road.h"
#include "economy/ware_instance.h"
#include "economy/waterway.h"
#include "graphic/rendertarget.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"

namespace Widelands {

FlagDescr g_flag_descr("flag", "Flag");

const FlagDescr& Flag::descr() const {
	return g_flag_descr;
}

/**
 * A bare flag, used for testing only.
 */
Flag::Flag()
   : PlayerImmovable(g_flag_descr),
     animstart_(0),
     building_(nullptr),
     ware_capacity_(8),
     ware_filled_(0),
     wares_(new PendingWare[ware_capacity_]),
     always_call_for_flag_(nullptr),
     act_pending_(false) {
	std::fill(std::begin(roads_), std::end(roads_), nullptr);
}

/**
 * Shouldn't be necessary to do anything,
 * since die() always calls cleanup() first.
 */
Flag::~Flag() {
	if (ware_filled_) {
		log_warn("Flag: ouch! wares left\n");
	}
	delete[] wares_;

	if (building_) {
		log_warn("Flag: ouch! building left\n");
	}

	if (!flag_jobs_.empty()) {
		log_warn("Flag: ouch! flagjobs left\n");
	}

	for (const RoadBase* const road : roads_) {
		if (road) {
			log_warn("Flag: ouch! road left\n");
		}
	}
}

void Flag::load_finish(EditorGameBase& egbase) {
	auto should_be_deleted = [&egbase, this](const OPtr<Worker>& r) {
		Worker& worker = *r.get(egbase);
		Bob::State const* const state = worker.get_state(Worker::taskWaitforcapacity);
		if (state == nullptr) {
			log_warn("worker %u is in the capacity wait queue of flag %u but "
			         "does not have a waitforcapacity task! Removing from queue.\n",
			         worker.serial(), serial());
			return true;
		}
		if (state->objvar1 != this) {
			log_warn("worker %u is in the capacity wait queue of flag %u but "
			         "its waitforcapacity task is for map object %u! Removing from "
			         "queue.\n",
			         worker.serial(), serial(), state->objvar1.serial());  // NOLINT
			return true;
		}
		return false;
	};

	capacity_wait_.erase(
	   std::remove_if(capacity_wait_.begin(), capacity_wait_.end(), should_be_deleted),
	   capacity_wait_.end());
}

/**
 * Creates a flag at the given location.
 */
Flag::Flag(EditorGameBase& egbase,
           Player* owning_player,
           const Coords& coords,
           Economy* ware_eco,
           Economy* worker_eco)
   : PlayerImmovable(g_flag_descr),
     building_(nullptr),
     ware_capacity_(8),
     ware_filled_(0),
     wares_(new PendingWare[ware_capacity_]),
     always_call_for_flag_(nullptr),
     act_pending_(false) {
	MutexLock m(MutexLock::ID::kObjects);

	std::fill(std::begin(roads_), std::end(roads_), nullptr);

	set_owner(owning_player);

	set_flag_position(coords);

	upcast(RoadBase, road, egbase.map().get_immovable(coords));
	upcast(Game, game, &egbase);

	if (game) {
		if (ware_eco) {
			// We're saveloading
			ware_eco->add_flag(*this);
		} else {
			//  we split a road, or a new, standalone flag is created
			(road ? road->get_economy(wwWARE) : owning_player->create_economy(wwWARE))
			   ->add_flag(*this);
		}
		if (worker_eco) {
			// We're saveloading
			worker_eco->add_flag(*this);
		} else {
			//  we split a road, or a new, standalone flag is created
			(road ? road->get_economy(wwWORKER) : owning_player->create_economy(wwWORKER))
			   ->add_flag(*this);
		}
		if (road && !ware_eco && !worker_eco) {
			road->presplit(*game, coords);
		}
	}

	init(egbase);

	if (!ware_eco && !worker_eco && road && game) {
		road->postsplit(*game, *this);
	}
}

void Flag::set_flag_position(Coords coords) {
	position_ = coords;
}

int32_t Flag::get_size() const {
	return SMALL;
}

bool Flag::get_passable() const {
	return true;
}

Flag& Flag::base_flag() {
	return *this;
}

/**
 * Call this only from Economy code!
 */
void Flag::set_economy(Economy* const e, WareWorker type) {
	Economy* const old = get_economy(type);

	if (old == e) {
		return;
	}

	PlayerImmovable::set_economy(e, type);

	if (type == wwWARE) {
		for (int32_t i = 0; i < ware_filled_; ++i) {
			wares_[i].ware->set_economy(e);
		}
	}

	if (building_) {
		building_->set_economy(e, type);
	}

	for (const FlagJob& temp_job : flag_jobs_) {
		if (temp_job.request && temp_job.request->get_type() == type) {
			temp_job.request->set_economy(e);
		}
	}

	for (RoadBase* const road : roads_) {
		if (road) {
			road->set_economy(e, type);
		}
	}
}

/**
 * Call this only from the Building init!
 */
void Flag::attach_building(EditorGameBase& egbase, Building& building) {
	assert(!building_ || building_ == &building);

	building_ = &building;

	const Map& map = egbase.map();
	egbase.set_road(
	   map.get_fcoords(map.tl_n(position_)), WALK_SE,
	   building_->get_size() == BaseImmovable::SMALL ? RoadSegment::kNormal : RoadSegment::kBusy);

	building.set_economy(get_economy(wwWARE), wwWARE);
	building.set_economy(get_economy(wwWORKER), wwWORKER);
}

/**
 * Call this only from the Building cleanup!
 */
void Flag::detach_building(EditorGameBase& egbase) {
	assert(building_);

	building_->set_economy(nullptr, wwWARE);
	building_->set_economy(nullptr, wwWORKER);

	const Map& map = egbase.map();
	egbase.set_road(map.get_fcoords(map.tl_n(position_)), WALK_SE, RoadSegment::kNone);

	building_ = nullptr;
}

/**
 * Call this only from the RoadBase init!
 */
void Flag::attach_road(int32_t const dir, RoadBase* const road) {
	assert(!roads_[dir - 1] || roads_[dir - 1] == road);

	roads_[dir - 1] = road;
	roads_[dir - 1]->set_economy(get_economy(wwWARE), wwWARE);
	roads_[dir - 1]->set_economy(get_economy(wwWORKER), wwWORKER);
}

/**
 * Call this only from the RoadBase init!
 */
void Flag::detach_road(int32_t const dir) {
	assert(roads_[dir - 1]);

	roads_[dir - 1]->set_economy(nullptr, wwWARE);
	roads_[dir - 1]->set_economy(nullptr, wwWORKER);
	roads_[dir - 1] = nullptr;
}

/**
 * \return all positions we occupy on the map. For a Flag, this is only one.
 */
BaseImmovable::PositionList Flag::get_positions(const EditorGameBase&) const {
	PositionList rv;
	rv.push_back(position_);
	return rv;
}

/**
 * \return neighbouring flags.
 */
void Flag::get_neighbours(WareWorker type, RoutingNodeNeighbours& neighbours) {
	for (RoadBase* const road : roads_) {
		if (!road) {
			continue;
		}

		// Only wares, workers cannot use ferries
		if (Waterway::is_waterway_descr(&road->descr()) && type == wwWORKER) {
			continue;
		}

		Flag* f = &road->get_flag(RoadBase::FlagEnd);
		int32_t nb_cost;
		if (f != this) {
			nb_cost = road->get_cost(RoadBase::FlagStart);
		} else {
			f = &road->get_flag(RoadBase::FlagStart);
			nb_cost = road->get_cost(RoadBase::FlagEnd);
		}
		if (type == wwWARE) {
			nb_cost += nb_cost * (get_waitcost() + f->get_waitcost()) / 2;
		}
		RoutingNodeNeighbour n(f, nb_cost);

		assert(n.get_neighbour() != this);
		neighbours.push_back(n);
	}

	if (building_ && building_->descr().get_isport()) {
		Warehouse* wh = dynamic_cast<Warehouse*>(building_);
		if (PortDock* pd = wh->get_portdock()) {
			pd->add_neighbours(neighbours);
		}
	}
}

/**
 * \return the road that leads to the given flag.
 */
RoadBase* Flag::get_roadbase(Flag& flag) {
	for (RoadBase* const road : roads_) {
		if (road) {
			if (&road->get_flag(RoadBase::FlagStart) == &flag ||
			    &road->get_flag(RoadBase::FlagEnd) == &flag) {
				return road;
			}
		}
	}
	return nullptr;
}
Road* Flag::get_road(Flag& flag) {
	for (int8_t i = WalkingDir::FIRST_DIRECTION; i <= WalkingDir::LAST_DIRECTION; ++i) {
		if (Road* const road = get_road(i)) {
			if (&road->get_flag(RoadBase::FlagStart) == &flag ||
			    &road->get_flag(RoadBase::FlagEnd) == &flag) {
				return road;
			}
		}
	}
	return nullptr;
}

Road* Flag::get_road(uint8_t const dir) const {
	if (roads_[dir - 1] && Road::is_road_descr(&roads_[dir - 1]->descr())) {
		return dynamic_cast<Road*>(roads_[dir - 1]);
	}
	return nullptr;
}
Waterway* Flag::get_waterway(uint8_t const dir) const {
	if (roads_[dir - 1] && Waterway::is_waterway_descr(&roads_[dir - 1]->descr())) {
		return dynamic_cast<Waterway*>(roads_[dir - 1]);
	}
	return nullptr;
}

/// \return the number of RoadBases connected to the flag
uint8_t Flag::nr_of_roadbases() const {
	uint8_t counter = 0;
	for (uint8_t road_id = WalkingDir::LAST_DIRECTION; road_id >= WalkingDir::FIRST_DIRECTION;
	     --road_id) {
		if (get_roadbase(road_id) != nullptr) {
			++counter;
		}
	}
	return counter;
}

/// \return the number of roads connected to the flag.
uint8_t Flag::nr_of_roads() const {
	uint8_t counter = 0;
	for (uint8_t road_id = WalkingDir::LAST_DIRECTION; road_id >= WalkingDir::FIRST_DIRECTION;
	     --road_id) {
		if (get_roadbase(road_id) != nullptr) {
			++counter;
		}
	}
	return counter;
}

/// \return the number of waterways connected to the flag.
uint8_t Flag::nr_of_waterways() const {
	uint8_t counter = 0;
	for (uint8_t road_id = WalkingDir::LAST_DIRECTION; road_id >= WalkingDir::FIRST_DIRECTION;
	     --road_id) {
		if (get_waterway(road_id) != nullptr) {
			++counter;
		}
	}
	return counter;
}

bool Flag::is_dead_end() const {
	if (get_building()) {
		return false;
	}
	Flag const* first_other_flag = nullptr;
	for (uint8_t road_id = WalkingDir::LAST_DIRECTION; road_id >= WalkingDir::FIRST_DIRECTION;
	     --road_id) {
		if (RoadBase* const road = get_roadbase(road_id)) {
			Flag& start = road->get_flag(RoadBase::FlagStart);
			Flag& other = this == &start ? road->get_flag(RoadBase::FlagEnd) : start;
			if (first_other_flag) {
				if (&other != first_other_flag) {
					return false;
				}
			} else {
				first_other_flag = &other;
			}
		}
	}
	return true;
}

/**
 * Returns true if the flag can hold more wares.
 */
bool Flag::has_capacity() const {
	return (ware_filled_ < ware_capacity_);
}

/**
 * Signal the given bob by interrupting its task as soon as capacity becomes
 * free.
 *
 * The capacity queue is a simple FIFO queue.
 */
void Flag::wait_for_capacity(Game&, Worker& bob) {
	capacity_wait_.push_back(&bob);
}

/**
 * Remove the worker from the list of workers waiting for free capacity.
 */
void Flag::skip_wait_for_capacity(Game&, Worker& w) {
	CapacityWaitQueue::iterator const it =
	   std::find(capacity_wait_.begin(), capacity_wait_.end(), &w);
	if (it != capacity_wait_.end()) {
		capacity_wait_.erase(it);
	}
}

void Flag::add_ware(EditorGameBase& egbase, WareInstance& ware) {

	assert(ware_filled_ < ware_capacity_);

	PendingWare& pi = wares_[ware_filled_++];
	pi.ware = &ware;
	pi.pending = false;
	pi.nextstep = nullptr;
	pi.priority = 0;

	Transfer* trans = ware.get_transfer();
	if (trans) {
		uint32_t trans_steps = trans->get_steps_left();
		if (trans_steps < 3) {
			pi.priority = 2;
		} else if (trans_steps == 3) {
			pi.priority = 1;
		}

		Request* req = trans->get_request();
		if (req) {
			pi.priority = pi.priority + req->get_normalized_transfer_priority();
		}
	}

	ware.set_location(egbase, this);

	if (upcast(Game, game, &egbase)) {
		ware.update(*game);  //  will call call_carrier() if necessary
	}
}

/**
 * \return true if a ware is currently waiting for a carrier to the given Flag.
 *
 * \note Due to fetch_from_flag() semantics, this function makes no sense
 * for a  building destination.
 */
bool Flag::has_pending_ware(Game&, Flag& dest) {
	for (int32_t i = 0; i < ware_filled_; ++i) {
		if (!wares_[i].pending) {
			continue;
		}

		if (wares_[i].nextstep != &dest) {
			continue;
		}

		return true;
	}

	return false;
}

/**
 * Called by carrier code to indicate that the carrier is moving to pick up an
 * ware. Ware with highest transfer priority is chosen.
 * \return true if an ware is actually waiting for the carrier.
 */
bool Flag::ack_pickup(Game&, Flag& destflag) {
	int32_t highest_pri = -1;
	int32_t i_pri = -1;

	for (int32_t i = 0; i < ware_filled_; ++i) {
		if (!wares_[i].pending) {
			continue;
		}

		if (wares_[i].nextstep != &destflag) {
			continue;
		}

		if (wares_[i].priority > highest_pri) {
			highest_pri = wares_[i].priority;
			i_pri = i;

			// Increase ware priority, it matters only if the ware has to wait.
			if (wares_[i].priority < kMaxTransferPriority) {
				wares_[i].priority++;
			}
		}
	}

	if (i_pri >= 0) {
		wares_[i_pri].pending = false;
		return true;
	}

	return false;
}
/**
 * Called by carrier code to find the best among the wares on this flag
 * that are meant for the provided dest.
 * \return index of found ware (carrier will take it)
 * or kNotFoundAppropriate (carrier will leave empty-handed)
 */
bool Flag::cancel_pickup(Game& game, Flag& destflag) {
	int32_t lowest_prio = kMaxTransferPriority + 1;
	int32_t i_pri = -1;

	for (int32_t i = 0; i < ware_filled_; ++i) {
		if (wares_[i].pending) {
			continue;
		}

		if (wares_[i].nextstep != &destflag) {
			continue;
		}

		if (wares_[i].priority < lowest_prio) {
			lowest_prio = wares_[i].priority;
			i_pri = i;
		}
	}

	if (i_pri >= 0) {
		wares_[i_pri].pending = true;
		wares_[i_pri].ware->update(game);  //  will call call_carrier() if necessary
		return true;
	}

	return false;
}

/**
 * Wake one sleeper from the capacity queue.
 */
void Flag::wake_up_capacity_queue(Game& game) {
	while (!capacity_wait_.empty()) {
		Worker* const w = capacity_wait_.front().get(game);
		capacity_wait_.pop_front();
		if (w && w->wakeup_flag_capacity(game, *this)) {
			break;
		}
	}
}

/**
 * Called by carrier code to retrieve one of the wares on the flag that is meant
 * for that carrier.
 *
 * This function may return 0 even if \ref ack_pickup() has already been
 * called successfully.
 */
WareInstance* Flag::fetch_pending_ware(Game& game, PlayerImmovable& dest) {
	int32_t best_index = -1;

	for (int32_t i = 0; i < ware_filled_; ++i) {
		if (wares_[i].nextstep != &dest) {
			continue;
		}

		// We prefer to retrieve wares that have already been acked
		if (best_index < 0 || !wares_[i].pending) {
			best_index = i;
		}
	}

	if (best_index < 0) {
		return nullptr;
	}

	// move the other wares up the list and return this one
	WareInstance* const ware = wares_[best_index].ware;
	--ware_filled_;
	memmove(&wares_[best_index], &wares_[best_index + 1],
	        sizeof(wares_[0]) * (ware_filled_ - best_index));

	ware->set_location(game, nullptr);  // Ware has no location while in transit

	// wake up capacity wait queue
	wake_up_capacity_queue(game);

	return ware;
}

/**
 * Accelerate potential promotion of roads adjacent to a newly promoted road.
 */
void Flag::propagate_promoted_road(Road* const promoted_road) {
	// Abort if flag has a building attached to it
	if (building_) {
		return;
	}

	// Calculate the sum of the involved wallets' adjusted value
	int32_t sum = 0;
	for (int8_t i = WalkingDir::FIRST_DIRECTION; i <= WalkingDir::LAST_DIRECTION; ++i) {
		Road* const road = get_road(i);
		if (road && road != promoted_road) {
			sum += kRoadMaxWallet + road->wallet() * road->wallet();
		}
	}

	// Distribute propagation coins in a smart way
	for (int8_t i = WalkingDir::FIRST_DIRECTION; i <= WalkingDir::LAST_DIRECTION; ++i) {
		Road* const road = get_road(i);
		if (road && !road->is_busy()) {
			road->add_to_wallet(0.5 * (kRoadMaxWallet - road->wallet()) *
			                    (kRoadMaxWallet + road->wallet() * road->wallet()) / sum);
		}
	}
}

/**
 * Count only those wares which are awaiting to be carried along the same road.
 */
uint8_t Flag::count_wares_in_queue(PlayerImmovable& dest) const {
	uint8_t n = 0;
	for (int32_t i = 0; i < ware_filled_; ++i) {
		if (wares_[i].nextstep == &dest) {
			++n;
		}
	}
	return n;
}

/**
 * Return a List of all the wares currently on this Flag.
 * Do not rely the result value to stay valid and do not change them.
 */
Flag::Wares Flag::get_wares() {
	Wares rv;

	for (int32_t i = 0; i < ware_filled_; ++i) {
		rv.push_back(wares_[i].ware);
	}

	return rv;
}

/**
 * Force a removal of the given ware from this flag.
 * Called by \ref WareInstance::cleanup()
 */
void Flag::remove_ware(EditorGameBase& egbase, WareInstance* const ware) {
	for (int32_t i = 0; i < ware_filled_; ++i) {
		if (wares_[i].ware != ware) {
			continue;
		}

		--ware_filled_;
		memmove(&wares_[i], &wares_[i + 1], sizeof(wares_[0]) * (ware_filled_ - i));

		if (upcast(Game, game, &egbase)) {
			wake_up_capacity_queue(*game);
		}

		return;
	}

	throw wexception("MO(%u): Flag::remove_ware: ware %u not on flag", serial(), ware->serial());
}

/**
 * If nextstep is not null, a carrier will be called to move this ware to
 * the given flag or building.
 *
 * If nextstep is null, the internal data will be reset to indicate that the
 * ware isn't going anywhere right now.
 *
 * nextstep is compared with the cached data, and a new carrier is only called
 * if that data hasn't changed.
 *
 * This behaviour is overridden by always_call_for_step_, which is set by
 * update_wares() to ensure that new carriers are called when roads are
 * split, for example.
 */
void Flag::call_carrier(Game& game, WareInstance& ware, PlayerImmovable* const nextstep) {
	PendingWare* pi = nullptr;
	int32_t i = 0;

	// Find the PendingWare entry
	for (; i < ware_filled_; ++i) {
		// Find pending ware
		if (wares_[i].ware != &ware) {
			continue;
		}

		// Found; handle it
		pi = &wares_[i];

		// Deal with the non-moving case quickly
		if (!nextstep) {
			pi->nextstep = nullptr;
			pi->pending = true;
			return;
		}

		// Find out whether we need to do anything
		if (pi->nextstep == nextstep && pi->nextstep != always_call_for_flag_) {
			return;  // no update needed
		}

		pi->nextstep = nextstep;
		pi->pending = false;

		// Deal with the building case
		if (nextstep == get_building()) {
			molog(game.get_gametime(), "Flag::call_carrier(%u): Tell building to fetch this ware\n",
			      ware.serial());

			if (!get_building()->fetch_from_flag(game)) {
				pi->ware->cancel_moving();
				pi->ware->update(game);
			}

			return;
		}

		// Deal with the normal (flag) case
		const Flag& nextflag = dynamic_cast<const Flag&>(*nextstep);

		for (int32_t dir = WalkingDir::FIRST_DIRECTION; dir <= WalkingDir::LAST_DIRECTION; ++dir) {
			RoadBase* const road = get_roadbase(dir);
			Flag* other;
			RoadBase::FlagId flagid;

			if (!road) {
				continue;
			}

			if (&road->get_flag(RoadBase::FlagStart) == this) {
				flagid = RoadBase::FlagStart;
				other = &road->get_flag(RoadBase::FlagEnd);
			} else {
				flagid = RoadBase::FlagEnd;
				other = &road->get_flag(RoadBase::FlagStart);
			}

			if (other != &nextflag) {
				continue;
			}

			// Yes, this is the road we want; inform it
			if (road->notify_ware(game, flagid)) {
				return;
			}

			// If the road doesn't react to the ware immediately, we try other roads:
			// They might lead to the same flag!
		}

		// Nothing found, just let it be picked up by somebody
		pi->pending = true;
		return;
	}
	NEVER_HERE();
}

/**
 * Called whenever a road gets broken or split.
 * Make sure all wares on this flag are rerouted if necessary.
 *
 * \note When two roads connect the same two flags, and one of these roads
 * is removed, this might cause the carrier(s) on the other road to
 * move unnecessarily. Fixing this could potentially be very expensive and
 * fragile.
 * A similar thing can happen when a road is split.
 */
void Flag::update_wares(Game& game, Flag* const other) {
	always_call_for_flag_ = other;

	for (int32_t i = 0; i < ware_filled_; ++i) {
		wares_[i].ware->update(game);
	}

	always_call_for_flag_ = nullptr;
}

bool Flag::init(EditorGameBase& egbase) {
	PlayerImmovable::init(egbase);

	set_position(egbase, position_);

	animstart_ = egbase.get_gametime();
	return true;
}

/**
 * Detach building and free roads.
 */
void Flag::cleanup(EditorGameBase& egbase) {
	while (!flag_jobs_.empty()) {
		delete flag_jobs_.begin()->request;
		flag_jobs_.erase(flag_jobs_.begin());
	}

	while (ware_filled_) {
		WareInstance& ware = *wares_[--ware_filled_].ware;

		ware.set_location(egbase, nullptr);
		ware.destroy(egbase);
	}

	if (building_) {
		building_->remove(egbase);  //  immediate death
		assert(!building_);
	}

	for (RoadBase* rb : roads_) {
		if (rb) {
			rb->remove(egbase);  //  immediate death
		}
	}
#ifndef NDEBUG
	for (RoadBase* rb : roads_) {
		assert(!rb);
	}
#endif

	if (Economy* e = get_economy(wwWARE)) {
		e->remove_flag(*this);
	}
	if (Economy* e = get_economy(wwWORKER)) {
		e->remove_flag(*this);
	}

	unset_position(egbase, position_);

	PlayerImmovable::cleanup(egbase);
}

void Flag::draw(const Time& gametime,
                const InfoToDraw,
                const Vector2f& field_on_dst,
                const Coords& coords,
                float scale,
                RenderTarget* dst) {
	static struct {
		float x, y;
	} ware_offsets[8] = {{-5.f, 1.f},  {-1.f, 3.f},  {3.f, 3.f},  {7.f, 1.f},
	                     {-6.f, -3.f}, {-1.f, -2.f}, {3.f, -2.f}, {8.f, -3.f}};

	const RGBColor& player_color = owner().get_playercolor();
	dst->blit_animation(field_on_dst, coords, scale, owner().tribe().flag_animation(),
	                    Time((gametime - animstart_).get()), &player_color);

	for (int32_t i = 0; i < ware_filled_; ++i) {  //  draw wares
		Vector2f warepos = field_on_dst;
		if (i < 8) {
			warepos.x += ware_offsets[i].x * scale;
			warepos.y += ware_offsets[i].y * scale;
		} else {
			warepos.y -= (6.f + (i - 8.f) * 3.f) * scale;
		}
		dst->blit_animation(warepos, Widelands::Coords::null(), scale,
		                    wares_[i].ware->descr().get_animation("idle", wares_[i].ware), Time(0),
		                    &player_color);
	}
}

/**
 * Destroy the building as well.
 *
 * \note This is needed in addition to the call to building_->remove() in
 * \ref Flag::cleanup(). This function is needed to ensure a fire is created
 * when a player removes a flag.
 */
void Flag::destroy(EditorGameBase& egbase) {
	if (building_) {
		building_->destroy(egbase);
		assert(!building_);
	}

	PlayerImmovable::destroy(egbase);
}

void Flag::receive_worker(Game&, Worker&) {
	// Callback when a requested scout arrives.
	// He knows what to do next by himself, nothing to do for us currently.
}

void Flag::do_schedule_act(Game& game, const Duration& d) {
	if (act_pending_) {
		return;
	}
	act_pending_ = true;
	schedule_act(game, d);
}

void Flag::act(Game& game, uint32_t) {
	assert(act_pending_);
	act_pending_ = false;

	bool need_act = false;
	for (auto it = flag_jobs_.begin(); it != flag_jobs_.end();) {
		bool erase = false;
		if (it->type == FlagJob::Type::kScout) {
			ProductionSite* ps = get_economy(wwWORKER)->find_closest_occupied_productionsite(
			   *this, owner().tribe().scouts_house(), true);
			if (ps) {
				Worker* worker = ps->working_positions()->at(0).worker.get(game);
				assert(worker);
				if (!worker->top_state().objvar1.is_set() && worker->get_location(game) == ps &&
				    !ps->has_forced_state()) {
					// Success! Tell the productionsite to instruct its worker
					// to come and scout here the next time he goes to work.
					ps->set_next_program_override(game, "targeted_scouting", this);
					get_owner()->show_watch_window(game, *worker);
					erase = true;
				} else {
					// No scout is in the building just now. Try again a bit later
					need_act = true;
				}
			} else {
				// No scout's house found
				need_act = true;
			}
		}
		if (erase) {
			it = flag_jobs_.erase(it);
		} else {
			++it;
		}
	}

	if (need_act) {
		do_schedule_act(game, Duration(1000));
	}
}

/**
 * Add a new flag job to request the worker with the given ID,
 * and to execute the given program once it's completed.
 */
void Flag::add_flag_job(Game& game, const FlagJob::Type t) {
	FlagJob j;

	j.request = nullptr;
	j.type = t;
	switch (t) {
	case FlagJob::Type::kGeologist:
		j.program = "expedition";
		j.request =
		   new Request(*this, owner().tribe().geologist(), Flag::flag_job_request_callback, wwWORKER);
		break;
	case FlagJob::Type::kScout:
		do_schedule_act(game, Duration(10));
		break;
	}

	flag_jobs_.push_back(j);
}

/**
 * This function is called when one of the flag job workers arrives on
 * the flag. Give him his job.
 */
void Flag::flag_job_request_callback(
   Game& game, Request& rq, DescriptionIndex, Worker* const w, PlayerImmovable& target) {
	Flag& flag = dynamic_cast<Flag&>(target);

	assert(w);

	for (FlagJobs::iterator flag_iter = flag.flag_jobs_.begin(); flag_iter != flag.flag_jobs_.end();
	     ++flag_iter) {
		if (flag_iter->request == &rq) {
			delete &rq;

			w->start_task_program(game, flag_iter->program);

			flag.flag_jobs_.erase(flag_iter);
			return;
		}
	}

	flag.molog(game.get_gametime(), "BUG: flag_job_request_callback: worker not found in list\n");
}

void Flag::log_general_info(const Widelands::EditorGameBase& egbase) const {
	molog(egbase.get_gametime(), "Flag at %i,%i\n", position_.x, position_.y);

	Widelands::PlayerImmovable::log_general_info(egbase);

	if (ware_filled_) {
		molog(egbase.get_gametime(), "Wares at flag:\n");
		for (int i = 0; i < ware_filled_; ++i) {
			PendingWare& pi = wares_[i];
			molog(egbase.get_gametime(), " %i/%i: %s(%i), nextstep %i, %s\n", i + 1, ware_capacity_,
			      pi.ware->descr().name().c_str(), pi.ware->serial(), pi.nextstep.serial(),
			      pi.pending ? "pending" : "acked by carrier");
		}
	} else {
		molog(egbase.get_gametime(), "No wares at flag.\n");
	}
}
}  // namespace Widelands
