/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#include "logic/map_objects/tribes/worker.h"

#include <memory>
#include <tuple>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/road.h"
#include "economy/transfer.h"
#include "graphic/rendertarget.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/cmd_incorporate.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/terrain_affinity.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker_program.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/mapfringeregion.h"
#include "logic/mapregion.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "map_io/map_packet_versions.h"

namespace Widelands {

Worker::Worker(const WorkerDescr& worker_descr) : Bob(worker_descr) {
}

Worker::~Worker() {
	assert(!location_.is_set());
	assert(!transfer_);
}

/// Log basic information.
void Worker::log_general_info(const EditorGameBase& egbase) const {
	Bob::log_general_info(egbase);

	if (upcast(PlayerImmovable, loc, location_.get(egbase))) {
		FORMAT_WARNINGS_OFF
		molog(egbase.get_gametime(), "* Owner: (%p)\n", &loc->owner());
		FORMAT_WARNINGS_ON
		molog(egbase.get_gametime(), "** Owner (plrnr): %i\n", loc->owner().player_number());
		FORMAT_WARNINGS_OFF
		molog(egbase.get_gametime(), "* WorkerEconomy: %p\n", loc->get_economy(wwWORKER));
		molog(egbase.get_gametime(), "* WareEconomy: %p\n", loc->get_economy(wwWARE));
		FORMAT_WARNINGS_ON
	}

	PlayerImmovable* imm = location_.get(egbase);
	molog(egbase.get_gametime(), "location: %u\n", imm != nullptr ? imm->serial() : 0);
	FORMAT_WARNINGS_OFF
	molog(egbase.get_gametime(), "WorkerEconomy: %p\n", worker_economy_);
	molog(egbase.get_gametime(), "WareEconomy: %p\n", ware_economy_);
	molog(egbase.get_gametime(), "transfer: %p\n", transfer_);
	FORMAT_WARNINGS_ON

	if (upcast(WareInstance, ware, carried_ware_.get(egbase))) {
		molog(egbase.get_gametime(), "* carried_ware->get_ware() (id): %i\n", ware->descr_index());
		FORMAT_WARNINGS_OFF
		molog(egbase.get_gametime(), "* carried_ware->get_economy() (): %p\n", ware->get_economy());
		FORMAT_WARNINGS_ON
	}

	molog(egbase.get_gametime(), "current_exp: %i / %i\n", current_exp_,
	      descr().get_needed_experience());

	FORMAT_WARNINGS_OFF
	molog(egbase.get_gametime(), "supply: %p\n", supply_);
	FORMAT_WARNINGS_ON
}

/**
 * Change the location. This should be called in the following situations:
 * \li worker creation (usually, location is a warehouse)
 * \li worker moves along a route (location is a road and finally building)
 * \li current location is destroyed (building burnt down etc...)
 */
void Worker::set_location(PlayerImmovable* const location) {
	assert(!location || ObjectPointer(location).get(owner().egbase()));

	PlayerImmovable* const old_location = get_location(owner().egbase());
	if (old_location == location) {
		return;
	}

	if (old_location != nullptr) {
		// Note: even though we have an old location, economy_ may be zero
		// (old_location got deleted)
		old_location->remove_worker(*this);
	} else {
		if (!is_shipping()) {
			assert(!ware_economy_);
			assert(!worker_economy_);
		}
	}

	location_ = location;

	if (location != nullptr) {
		Economy* const eco_wo = location->get_economy(wwWORKER);
		Economy* const eco_wa = location->get_economy(wwWARE);

		if ((worker_economy_ == nullptr) || (descr().type() == MapObjectType::SOLDIER)) {
			set_economy(eco_wo, wwWORKER);
		} else if (worker_economy_ != eco_wo) {
			throw wexception("Worker::set_location changes worker_economy, but worker is no soldier");
		}
		if ((ware_economy_ == nullptr) || (descr().type() == MapObjectType::SOLDIER)) {
			set_economy(eco_wa, wwWARE);
		} else if (ware_economy_ != eco_wa) {
			throw wexception("Worker::set_location changes ware_economy, but worker is no soldier");
		}
		location->add_worker(*this);
	} else {
		if (!is_shipping()) {
			// Our location has been destroyed, we are now fugitives.
			// Interrupt whatever we've been doing.
			set_economy(nullptr, wwWARE);
			set_economy(nullptr, wwWORKER);

			EditorGameBase& egbase = get_owner()->egbase();
			if (upcast(Game, game, &egbase)) {
				send_signal(*game, "location");
			}
		}
	}
}

/**
 * Change the worker's current economy. This is called:
 * \li by set_location() when appropriate
 * \li by the current location, when the location's economy changes
 */
void Worker::set_economy(Economy* const economy, WareWorker type) {
	Economy* old = get_economy(type);
	if (economy == old) {
		return;
	}

	switch (type) {
	case wwWARE: {
		ware_economy_ = economy;
		if (WareInstance* const ware = get_carried_ware(get_owner()->egbase())) {
			ware->set_economy(ware_economy_);
		}
	} break;
	case wwWORKER: {
		worker_economy_ = economy;
		if (old != nullptr) {
			old->remove_wares_or_workers(owner().tribe().worker_index(descr().name()), 1);
		}
		if (supply_ != nullptr) {
			supply_->set_economy(worker_economy_);
		}
		if (worker_economy_ != nullptr) {
			worker_economy_->add_wares_or_workers(
			   owner().tribe().worker_index(descr().name()), 1, ware_economy_);
		}
	} break;
	default:
		NEVER_HERE();
	}
}

/**
 * Initialize the worker
 */
bool Worker::init(EditorGameBase& egbase) {
	Bob::init(egbase);

	// a worker should always start out at a fixed location
	// (this assert is not longer true for save games. Where it lives
	// is unknown to this worker till he is initialized
	//  assert(get_location(egbase));

	if (upcast(Game, game, &egbase)) {
		create_needed_experience(*game);
	}
	return true;
}

/**
 * Remove the worker.
 */
void Worker::cleanup(EditorGameBase& egbase) {
	WareInstance* const ware = get_carried_ware(egbase);

	if (supply_ != nullptr) {
		delete supply_;
		supply_ = nullptr;
	}

	if (ware != nullptr) {
		ware->destroy(egbase);
	}

	// We are destroyed, but we were maybe idling
	// or doing something else. Get Location might
	// init a gowarehouse task or something and this results
	// in a dirty stack. Nono, we do not want to end like this
	if (upcast(Game, game, &egbase)) {
		reset_tasks(*game);
	}

	if (get_location(egbase) != nullptr) {
		set_location(nullptr);
	}

	set_economy(nullptr, wwWARE);
	set_economy(nullptr, wwWORKER);

	Bob::cleanup(egbase);
}

/**
 * Set the ware we carry.
 * If we carry an ware right now, it will be destroyed (see
 * fetch_carried_ware()).
 */
void Worker::set_carried_ware(EditorGameBase& egbase, WareInstance* const ware) {
	if (WareInstance* const oldware = get_carried_ware(egbase)) {
		oldware->cleanup(egbase);
		delete oldware;
	}

	carried_ware_ = ware;
	ware->set_location(egbase, this);
	if (upcast(Game, game, &egbase)) {
		ware->update(*game);
	}
}

/**
 * Stop carrying the current ware, and return a pointer to it.
 */
WareInstance* Worker::fetch_carried_ware(EditorGameBase& game) {
	WareInstance* const ware = get_carried_ware(game);

	if (ware != nullptr) {
		ware->set_location(game, nullptr);
		carried_ware_ = nullptr;
	}

	return ware;
}

/**
 * Schedule an immediate CMD_INCORPORATE, which will integrate this worker into
 * the warehouse he is standing on.
 */
void Worker::schedule_incorporate(Game& game) {
	game.cmdqueue().enqueue(new CmdIncorporate(game.get_gametime(), this));
	return skip_act();
}

/**
 * Incorporate the worker into the warehouse it's standing on immediately.
 */
void Worker::incorporate(Game& game) {
	if (upcast(Warehouse, wh, get_location(game))) {
		wh->incorporate_worker(game, this);
		return;
	}

	// our location has been deleted from under us
	send_signal(game, "fail");
}

/**
 * Calculate needed experience.
 *
 * This sets the needed experience on a value between max and min
 */
void Worker::create_needed_experience(Game& /* game */) {
	if (descr().get_needed_experience() == INVALID_INDEX) {
		current_exp_ = INVALID_INDEX;
		return;
	}

	current_exp_ = 0;
}

void Worker::set_current_experience(const int32_t xp) {
	assert(needs_experience());
	assert(xp >= 0);
	assert(xp < descr().get_needed_experience());
	current_exp_ = xp;
}

/**
 * Gain experience
 *
 * This function increases the experience
 * of the worker by one, if he reaches
 * needed_experience he levels
 */
DescriptionIndex Worker::gain_experience(Game& game) {
	return (descr().get_needed_experience() == INVALID_INDEX ||
	        ++current_exp_ < descr().get_needed_experience()) ?
	          INVALID_INDEX :
	          level(game);
}

/**
 * Level this worker to the next higher level. this includes creating a
 * new worker with his propertys and removing this worker
 */
DescriptionIndex Worker::level(Game& game) {

	// We do not really remove this worker, all we do
	// is to overwrite his description with the new one and to
	// reset his needed experience. Congratulations to promotion!
	// This silently expects that the new worker is the same type as the old
	// worker and can fullfill the same jobs (which should be given in all
	// circumstances)
	assert(descr().becomes() != INVALID_INDEX);
	const TribeDescr& t = owner().tribe();
	DescriptionIndex const old_index = t.worker_index(descr().name());
	DescriptionIndex const new_index = descr().becomes();
	descr_ = t.get_worker_descr(new_index);
	assert(t.has_worker(new_index));

	// Inform the economy, that something has changed
	worker_economy_->remove_wares_or_workers(old_index, 1);
	worker_economy_->add_wares_or_workers(new_index, 1, ware_economy_);

	create_needed_experience(game);
	return old_index;  //  So that the caller knows what to replace him with.
}

/**
 * Set a fallback task.
 */
void Worker::init_auto_task(Game& game) {
	if (PlayerImmovable* location = get_location(game)) {
		if (!get_economy(wwWORKER)->warehouses().empty() ||
		    location->descr().type() >= MapObjectType::BUILDING) {
			return start_task_gowarehouse(game);
		}
		set_location(nullptr);
	}

	molog(game.get_gametime(), "init_auto_task: become fugitive\n");

	return start_task_fugitive(game);
}

/**
 * Immediately evict the worker from his current building, if allowed.
 */
void Worker::evict(Game& game) {
	if (!is_evict_allowed()) {
		verb_log_warn_time(game.get_gametime(), "Worker %s %u: evict not currently allowed",
		                   descr().name().c_str(), serial());
		return;
	}
	if (!is_employed()) {
		verb_log_warn_time(game.get_gametime(), "Trying to evict worker %s %u who is not employed",
		                   descr().name().c_str(), serial());
		return;
	}

	molog(game.get_gametime(), "Evicting!");
	upcast(Building, building, get_location(game));
	building->notify_worker_evicted(game, *this);
	reset_tasks(game);
	set_location(&building->base_flag());
	start_task_return(game, true);
}

bool Worker::is_evict_allowed() {
	return true;
}

/** Check if this worker is currently employed in a building. */
bool Worker::is_employed() {
	PlayerImmovable* loc = get_location(owner().egbase());
	if (loc == nullptr || loc->descr().type() < MapObjectType::BUILDING) {
		return false;
	}

	if (get_state(taskBuildingwork) != nullptr) {
		// Main worker has task buildingwork anywhere in the stack.
		return true;
	}

	// Additional workers have idle task and no other task.
	return get_stack_size() == 1 && is_idle();
}

void Worker::draw_inner(const EditorGameBase& game,
                        const Vector2f& point_on_dst,
                        const Coords& coords,
                        const float scale,
                        RenderTarget* dst) const {
	assert(get_owner() != nullptr);
	const RGBColor& player_color = get_owner()->get_playercolor();

	dst->blit_animation(point_on_dst, coords, scale, get_current_anim(),
	                    Time((game.get_gametime() - get_animstart()).get()), &player_color);

	if (WareInstance const* const carried_ware = get_carried_ware(game)) {
		const Vector2f hotspot = descr().ware_hotspot().cast<float>();
		const Vector2f location(
		   point_on_dst.x - hotspot.x * scale, point_on_dst.y - hotspot.y * scale);
		dst->blit_animation(location, Widelands::Coords::null(), scale,
		                    carried_ware->descr().get_animation("idle", this), Time(0),
		                    &player_color);
	}
}

/**
 * Draw the worker, taking the carried ware into account.
 */
void Worker::draw(const EditorGameBase& egbase,
                  const InfoToDraw& /*info_to_draw*/,
                  const Vector2f& field_on_dst,
                  const Widelands::Coords& coords,
                  const float scale,
                  RenderTarget* dst) const {
	if (get_current_anim() == 0u) {
		return;
	}
	draw_inner(egbase, calc_drawpos(egbase, field_on_dst, scale), coords, scale, dst);
}

/*
==============================

Load/save support

==============================
*/

constexpr uint8_t kCurrentPacketVersion = 3;

void Worker::Loader::load(FileRead& fr) {
	Bob::Loader::load(fr);
	try {
		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {

			Worker& worker = get<Worker>();
			location_ = fr.unsigned_32();
			carried_ware_ = fr.unsigned_32();
			worker.current_exp_ = fr.signed_32();

			if (fr.unsigned_8() != 0u) {
				worker.transfer_ = new Transfer(dynamic_cast<Game&>(egbase()), worker);
				worker.transfer_->read(fr, transfer_);
			}
			const unsigned veclen = fr.unsigned_8();
			for (unsigned q = 0; q < veclen; q++) {
				if (fr.unsigned_8() != 0u) {
					const PlaceToScout gsw;
					worker.scouts_worklist.push_back(gsw);
				} else {
					const int16_t x = fr.signed_16();
					const int16_t y = fr.signed_16();
					Coords peekpos = Coords(x, y);
					const PlaceToScout gtt(peekpos);
					worker.scouts_worklist.push_back(gtt);
				}
			}

		} else {
			throw UnhandledVersionError("Worker", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading worker: %s", e.what());
	}
}

void Worker::Loader::load_pointers() {
	Bob::Loader::load_pointers();

	Worker& worker = get<Worker>();

	if (location_ != 0u) {
		worker.set_location(&mol().get<PlayerImmovable>(location_));
	}
	if (carried_ware_ != 0u) {
		worker.carried_ware_ = &mol().get<WareInstance>(carried_ware_);
	}
	if (worker.transfer_ != nullptr) {
		worker.transfer_->read_pointers(mol(), transfer_);
	}
}

void Worker::Loader::load_finish() {
	Bob::Loader::load_finish();

	Worker& worker = get<Worker>();

	// If our economy is unclear because we have no location, it is wise to not
	// mess with it. For example ships will not be a location for Workers
	// (because they are no PlayerImmovable), but they will handle economies for
	// us and will do so on load too. To make the order at which we are loaded
	// not a factor, we do not overwrite the economy they might have set for us
	// already.
	if (PlayerImmovable* const location = worker.location_.get(egbase())) {
		worker.set_economy(location->get_economy(wwWARE), wwWARE);
		worker.set_economy(location->get_economy(wwWORKER), wwWORKER);
	}
}

const Bob::Task* Worker::Loader::get_task(const std::string& name) {
	if (name == "program") {
		return &taskProgram;
	}
	if (name == "transfer") {
		return &taskTransfer;
	}
	if (name == "shipping") {
		return &taskShipping;
	}
	if (name == "buildingwork") {
		return &taskBuildingwork;
	}
	if (name == "return") {
		return &taskReturn;
	}
	if (name == "gowarehouse") {
		return &taskGowarehouse;
	}
	if (name == "dropoff") {
		return &taskDropoff;
	}
	if (name == "releaserecruit") {
		return &taskReleaserecruit;
	}
	if (name == "fetchfromflag") {
		return &taskFetchfromflag;
	}
	if (name == "waitforcapacity") {
		return &taskWaitforcapacity;
	}
	if (name == "leavebuilding") {
		return &taskLeavebuilding;
	}
	if (name == "fugitive") {
		return &taskFugitive;
	}
	if (name == "geologist") {
		return &taskGeologist;
	}
	if (name == "scout") {
		return &taskScout;
	}
	return Bob::Loader::get_task(name);
}

const MapObjectProgram* Worker::Loader::get_program(const std::string& name) {
	const Worker& worker = get<Worker>();
	const WorkerProgram* program = worker.descr().get_program(name);
	return program;
}

Worker::Loader* Worker::create_loader() {
	return new Loader;
}

/**
 * Load function for all classes derived from \ref Worker
 *
 * Derived classes must override \ref create_loader to make sure
 * the appropriate actual load functions are called.
 */
MapObject::Loader*
Worker::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr, uint8_t packet_version) {
	if (packet_version == kCurrentPacketVersionMapObject) {
		try {
			// header has already been read by caller
			const WorkerDescr* descr = egbase.descriptions().get_worker_descr(
			   egbase.descriptions().safe_worker_index(fr.c_string()));

			Worker* worker = dynamic_cast<Worker*>(&descr->create_object());
			std::unique_ptr<Loader> loader(worker->create_loader());
			loader->init(egbase, mol, *worker);
			loader->load(fr);
			return loader.release();
		} catch (const std::exception& e) {
			throw wexception("loading worker: %s", e.what());
		}
	} else {
		throw UnhandledVersionError(
		   "MapObjectPacket::Worker", packet_version, kCurrentPacketVersionMapObject);
	}
}

/**
 * Save the \ref Worker specific header and version info.
 *
 * \warning Do not override this function, override \ref do_save instead.
 */
void Worker::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderWorker);
	fw.c_string(descr().name());

	do_save(egbase, mos, fw);
}

/**
 * Save the data fields of this worker.
 *
 * This is separate from \ref save because of the way data headers are treated.
 *
 * Override this function in derived classes.
 */
void Worker::do_save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	Bob::save(egbase, mos, fw);

	fw.unsigned_8(kCurrentPacketVersion);
	fw.unsigned_32(mos.get_object_file_index_or_zero(location_.get(egbase)));
	fw.unsigned_32(mos.get_object_file_index_or_zero(carried_ware_.get(egbase)));
	fw.signed_32(current_exp_);

	if (transfer_ != nullptr) {
		fw.unsigned_8(1);
		transfer_->write(mos, fw);
	} else {
		fw.unsigned_8(0);
	}

	fw.unsigned_8(scouts_worklist.size());
	for (auto p : scouts_worklist) {
		if (p.randomwalk) {
			fw.unsigned_8(1);
		} else {
			fw.unsigned_8(0);
			// Is there a better way to save Coords? This makes
			// unnecessary assumptions of the internals of Coords
			fw.signed_16(p.scoutme.x);
			fw.signed_16(p.scoutme.y);
		}
	}
}
}  // namespace Widelands
