/*
 * Copyright (C) 2011-2021 by the Widelands Development Team
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

#include "economy/ferry_fleet.h"

#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/waterway.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/tribes/ferry.h"
#include "logic/mapastar.h"
#include "logic/path.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

namespace {
// Every MapObject() needs to have a description. So we make a dummy one for
// Fleet.
FerryFleetDescr g_ferry_fleet_descr("ferry_fleet", "Ferry Fleet");
}  // namespace

const FerryFleetDescr& FerryFleet::descr() const {
	return g_ferry_fleet_descr;
}

/**
 * Fleets are initialized empty.
 *
 * Intended use: @ref Ferry and @ref Waterway, when created, create a new @ref FerryFleet
 * instance, then add themselves \em before calling the \ref init function.
 * The FerryFleet takes care of merging with existing fleets, if any.
 */
FerryFleet::FerryFleet(Player* player) : MapObject(&g_ferry_fleet_descr), act_pending_(false) {
	owner_ = player;
}

/**
 * Whether the fleet is in fact useful for transporting wares.
 * This is the case if we have a ferry and a request for a ferry.
 */
bool FerryFleet::active() const {
	return !ferries_.empty() && !pending_ferry_requests_.empty();
}

/**
 * Initialize the fleet, including a search through the map
 * to rejoin with the next other fleet we can find.
 */
bool FerryFleet::init(EditorGameBase& egbase) {
	MapObject::init(egbase);

	if (empty()) {
		molog(egbase.get_gametime(), "Empty fleet initialized; disband immediately\n");
		remove(egbase);
		return false;
	}

	return find_other_fleet(egbase);
}

bool FerryFleet::init(EditorGameBase& egbase, Waterway* waterway) {
	MapObject::init(egbase);

	request_ferry(egbase, waterway, egbase.get_gametime());

	return find_other_fleet(egbase);
}

struct StepEvalFindFerryFleet {
	explicit StepEvalFindFerryFleet(const EditorGameBase& egbase)
	   : checkstep_(new CheckStepFerry(egbase)) {
	}

	int32_t estimate(Map& /* map */, FCoords /* pos */) const {
		return 0;
	}
	int32_t stepcost(
	   const Map& map, FCoords from, int32_t /* fromcost */, WalkingDir dir, FCoords to) const {
		return checkstep_->allowed(map, from, to, dir, CheckStep::StepId::stepNormal) ? 1 : -1;
	}

private:
	std::unique_ptr<CheckStepFerry> checkstep_;
};

/**
 * Search the map, starting at our ferries and waterways, for another fleet
 * of the same player.
 */
bool FerryFleet::find_other_fleet(EditorGameBase& egbase) {
	StepEvalFindFerryFleet stepeval(egbase);
	MapAStar<StepEvalFindFerryFleet> astar(*egbase.mutable_map(), stepeval, wwWORKER);
	for (const Ferry* temp_ferry : ferries_) {
		astar.push(temp_ferry->get_position());
	}
	for (const auto& temp_ww : pending_ferry_requests_) {
		for (Coords& c : temp_ww.second->get_positions(egbase)) {
			astar.push(c);
		}
	}

	int32_t cost;
	FCoords cur;
	while (astar.step(cur, cost)) {
		if (BaseImmovable* imm = cur.field->get_immovable()) {
			const MapObjectType type = imm->descr().type();
			if (type == MapObjectType::WATERWAY) {
				upcast(Waterway, ww, imm);
				if (ww->get_fleet() != this && ww->get_owner() == get_owner()) {
					return ww->get_fleet().get(egbase)->merge(egbase, this);
				}
			}
		}

		for (Bob* bob = cur.field->get_first_bob(); bob != nullptr; bob = bob->get_next_bob()) {
			const MapObjectType type = bob->descr().type();
			if (type == MapObjectType::FERRY) {
				upcast(Ferry, ferry, bob);
				if (ferry->get_fleet() != nullptr && ferry->get_fleet() != this &&
				    ferry->get_owner() == get_owner()) {
					return ferry->get_fleet()->merge(egbase, this);
				}
			}
		}
	}

	if (active()) {
		update(egbase);
		return true;
	}
	return false;
}

/**
 * Merge the @p other fleet into this fleet, and remove the other fleet.
 *
 * Returns true if 'other' is the resulting fleet and "false" if 'this' is
 * the resulting fleet. The values are reversed because we originally call this from
 * another 'other' for efficiency reasons.
 */
bool FerryFleet::merge(EditorGameBase& egbase, FerryFleet* other) {
	if (ferries_.empty() && !other->ferries_.empty()) {
		other->merge(egbase, this);
		return true;
	}

	while (!other->ferries_.empty()) {
		Ferry* ferry = other->ferries_.back();
		other->ferries_.pop_back();
		add_ferry(ferry);
	}

	while (!other->pending_ferry_requests_.empty()) {
		auto pair = other->pending_ferry_requests_.begin();
		Time time = pair->first;
		Waterway* ww = pair->second;
		assert(ww->get_fleet() == other);
		// set_fleet() tells the associated waterway to remove this request from the other fleet
		ww->set_fleet(this);
		pending_ferry_requests_.emplace(time, ww);
	}

	other->remove(egbase);

	update(egbase);
	return false;
}

void FerryFleet::cleanup(EditorGameBase& egbase) {
	while (!ferries_.empty()) {
		Ferry* ferry = ferries_.back();
		ferry->set_fleet(nullptr);
		ferries_.pop_back();
	}
	while (!pending_ferry_requests_.empty()) {
		auto pair = pending_ferry_requests_.begin();
		assert(pair->second->get_fleet() == this);
		pair->second->set_fleet(nullptr);
		pending_ferry_requests_.erase(pair);
	}

	MapObject::cleanup(egbase);
}

uint32_t FerryFleet::count_ferries() const {
	return ferries_.size();
}

uint32_t FerryFleet::count_unattended_waterways() const {
	return pending_ferry_requests_.size();
}

// Returns true of this waterway has a ferry or a ferry is on the way there
bool FerryFleet::has_ferry(const Waterway& ww) const {
	if (ww.get_ferry().get(owner().egbase())) {
		return true;
	}
	assert(ww.get_fleet().get(owner().egbase()) == this);
	for (const auto& pair : pending_ferry_requests_) {
		if (pair.second == &ww) {
			return false;
		}
	}
	return true;
}

void FerryFleet::add_ferry(Ferry* ferry) {
	ferries_.push_back(ferry);
	assert(std::count(ferries_.begin(), ferries_.end(), ferry) == 1);
	ferry->set_fleet(this);
}

void FerryFleet::remove_ferry(EditorGameBase& egbase, Ferry* ferry) {
	auto it = std::find(ferries_.begin(), ferries_.end(), ferry);
	if (it == ferries_.end()) {
		log_warn_time(egbase.get_gametime(),
		              "FerryFleet %u: Requested to remove ferry %u which is not in this fleet\n",
		              serial(), ferry ? ferry->serial() : 0);
		return;
	}
	while (it != ferries_.end()) {
		ferries_.erase(it);
		it = std::find(ferries_.begin(), ferries_.end(), ferry);
		if (it != ferries_.end()) {
			log_warn_time(egbase.get_gametime(),
			              "FerryFleet %u: Multiple instances of ferry %u were in the ferry fleet",
			              serial(), ferry->serial());
		}
	}
	assert(std::count(ferries_.begin(), ferries_.end(), ferry) == 0);
	ferry->set_fleet(nullptr);

	if (ferry->get_location(egbase)) {
		update(egbase);
	}

	if (empty()) {
		remove(egbase);
	}
}

/**
 * Adds a request for a ferry. The request will be fulfilled as soon as possible
 * in the next call to act(). When a ferry is found, its destination will be set to the waterway.
 * Multiple requests will be treated first come first served.
 */
void FerryFleet::request_ferry(const EditorGameBase& egbase,
                               Waterway* waterway,
                               const Time& gametime) {
	for (const auto& pair : pending_ferry_requests_) {
		if (pair.second == waterway) {
			if (waterway->get_fleet() != this) {
				waterway->set_fleet(this);
			}
			// One and the same request may be issued twice, e.g. when splitting a waterway – ignore
			return;
		}
	}
	pending_ferry_requests_.emplace(
	   gametime.is_invalid() ? egbase.get_gametime() : gametime, waterway);
	waterway->set_fleet(this);
}

void FerryFleet::cancel_ferry_request(Game& game, Waterway* waterway) {
	for (Ferry* ferry : ferries_) {
		if (ferry->get_destination(game) == waterway) {
			ferry->set_destination(game, nullptr);
			break;
		}
	}
	for (auto it = pending_ferry_requests_.begin(); it != pending_ferry_requests_.end(); ++it) {
		if (it->second == waterway) {

			pending_ferry_requests_.erase(it);
			if (empty()) {
				// We're no longer needed, act() will destroy us soon
				update(game);
			}
			return;
		}
	}
}

void FerryFleet::reroute_ferry_request(Game& game, Waterway* oldww, Waterway* newww) {
	for (Ferry* ferry : ferries_) {
		if (ferry->get_destination(game) == oldww) {
			ferry->set_destination(game, newww);
			return;
		}
	}
	for (auto& pair : pending_ferry_requests_) {
		if (pair.second == oldww) {
			pair.second = newww;
			return;
		}
	}
	log_warn_time(
	   game.get_gametime(),
	   "FerryFleet::reroute_ferry_request: received order to reroute inexistent request\n");
}

bool FerryFleet::empty() const {
	return ferries_.empty() && pending_ferry_requests_.empty();
}

/**
 * Trigger an update of ferry scheduling
 */
void FerryFleet::update(EditorGameBase& egbase, const Duration& tdelta) {
	if (act_pending_) {
		return;
	}

	if (upcast(Game, game, &egbase)) {
		schedule_act(*game, tdelta);
		act_pending_ = true;
	}
}

/**
 * Act callback updates ferry assigning. All decisions about which ferry to assign to which waterway
 * are supposed to be made by this function.
 *
 * @note Do not call this directly; instead, trigger it via @ref update
 */
void FerryFleet::act(Game& game, uint32_t /* data */) {
	assert(act_pending_);
	act_pending_ = false;

	if (empty()) {
		molog(game.get_gametime(), "FerryFleet::act: remove empty fleet\n");
		remove(game);
		return;
	}

	if (!active()) {
		// If we are here, most likely update() was called by a pending ferry request
		// when there are no ferries yet or by a new ferry when we can't offer
		// employment yet. We can't handle it now, so we reschedule the act()
		molog(game.get_gametime(), "FerryFleet::act: inactive, retry later\n");
		return update(game, Duration(5000));
	}

	molog(game.get_gametime(), "FerryFleet::act\n");

	std::vector<Ferry*> idle_ferries;
	for (Ferry* f : ferries_) {
		if (f->unemployed()) {
			idle_ferries.push_back(f);
		}
	}
	while (!pending_ferry_requests_.empty() && !idle_ferries.empty()) {
		// The map is sorted by ascending gametime
		Waterway& ww = *pending_ferry_requests_.begin()->second;

		Ferry* ferry = idle_ferries.front();
		int32_t shortest_distance = std::numeric_limits<int32_t>::max();
		for (Ferry* temp_ferry : idle_ferries) {
			// Decide how far this ferry is from the waterway
			Path path;
			int32_t f_distance =
			   game.map().findpath(temp_ferry->get_position(), ww.base_flag().get_position(), 0, path,
			                       CheckStepFerry(game));
			if (f_distance < 0) {
				log_err_time(game.get_gametime(),
				             "FerryFleet(%u)::act: We have a ferry (%u at %dx%d) "
				             "that can't reach one of our waterways (%u at %dx%d)!\n",
				             serial_, temp_ferry->serial(), temp_ferry->get_position().x,
				             temp_ferry->get_position().y, ww.serial(), ww.base_flag().get_position().x,
				             ww.base_flag().get_position().y);
				continue;
			}

			if (f_distance < shortest_distance) {
				ferry = temp_ferry;
				shortest_distance = f_distance;
			}
		}
		assert(ferry);

		idle_ferries.erase(std::find(idle_ferries.begin(), idle_ferries.end(), ferry));
		pending_ferry_requests_.erase(pending_ferry_requests_.begin());

		ferry->start_task_row(game, ww);
	}

	if (!pending_ferry_requests_.empty()) {
		molog(game.get_gametime(),
		      "... there are %" PRIuS " waterways requesting a ferry we cannot satisfy yet\n",
		      pending_ferry_requests_.size());
		// try again later
		return update(game, Duration(5000));
	}
}

void FerryFleet::log_general_info(const EditorGameBase& egbase) const {
	MapObject::log_general_info(egbase);

	molog(egbase.get_gametime(), "%" PRIuS " ferries and %" PRIuS " waterways\n", ferries_.size(),
	      pending_ferry_requests_.size());
	for (const Ferry* f : ferries_) {
		molog(egbase.get_gametime(), "* Ferry %u\n", f->serial());
	}
	for (const auto& pair : pending_ferry_requests_) {
		molog(egbase.get_gametime(), "* Waterway %u (requested at %u)\n", pair.second->serial(),
		      pair.first.get());
	}
}

constexpr uint8_t kCurrentPacketVersion = 1;

void FerryFleet::Loader::load(FileRead& fr) {
	MapObject::Loader::load(fr);

	FerryFleet& fleet = get<FerryFleet>();

	fleet.act_pending_ = fr.unsigned_8();

	const uint32_t nrferries = fr.unsigned_32();
	ferries_.resize(nrferries);
	for (uint32_t i = 0; i < nrferries; ++i) {
		ferries_[i] = fr.unsigned_32();
	}

	const uint32_t nrww = fr.unsigned_32();
	for (uint32_t i = 0; i < nrww; ++i) {
		const Time gametime(fr);
		const uint32_t serial = fr.unsigned_32();
		pending_ferry_requests_.emplace(gametime, serial);
	}
}

void FerryFleet::Loader::load_pointers() {
	MapObject::Loader::load_pointers();

	FerryFleet& fleet = get<FerryFleet>();

	// Act commands created during loading are not persistent, so we need to undo any
	// changes to the pending state.
	bool save_act_pending = fleet.act_pending_;

	for (const uint32_t& temp_ferry : ferries_) {
		fleet.ferries_.push_back(&mol().get<Ferry>(temp_ferry));
		fleet.ferries_.back()->set_fleet(&fleet);
	}
	for (const auto& temp_ww : pending_ferry_requests_) {
		Waterway& ww = mol().get<Waterway>(temp_ww.second);
		fleet.pending_ferry_requests_.emplace(temp_ww.first, &ww);
		ww.set_fleet(&fleet);
	}

	fleet.act_pending_ = save_act_pending;
}

MapObject::Loader* FerryFleet::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller
		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			PlayerNumber owner_number = fr.unsigned_8();
			if (!owner_number || owner_number > egbase.map().get_nrplayers()) {
				throw GameDataError("owner number is %u but there are only %u players", owner_number,
				                    egbase.map().get_nrplayers());
			}

			Player* owner = egbase.get_player(owner_number);
			if (!owner) {
				throw GameDataError("owning player %u does not exist", owner_number);
			}

			loader->init(egbase, mol, *(new FerryFleet(owner)));
			loader->load(fr);
		} else {
			throw UnhandledVersionError("FerryFleet", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading ferry fleet: %s", e.what());
	}

	return loader.release();
}

void FerryFleet::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderFerryFleet);
	fw.unsigned_8(kCurrentPacketVersion);

	fw.unsigned_8(owner_.load()->player_number());

	MapObject::save(egbase, mos, fw);

	fw.unsigned_8(act_pending_ ? 1 : 0);

	fw.unsigned_32(ferries_.size());
	for (const Ferry* temp_ferry : ferries_) {
		fw.unsigned_32(mos.get_object_file_index(*temp_ferry));
	}
	fw.unsigned_32(pending_ferry_requests_.size());
	for (const auto& temp_ww : pending_ferry_requests_) {
		temp_ww.first.save(fw);
		fw.unsigned_32(mos.get_object_file_index(*temp_ww.second));
	}
}

}  // namespace Widelands
