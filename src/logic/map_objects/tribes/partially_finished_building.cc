/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "logic/map_objects/tribes/partially_finished_building.h"

#include "base/macros.h"
#include "economy/request.h"
#include "economy/wares_queue.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"

namespace Widelands {

PartiallyFinishedBuilding::PartiallyFinishedBuilding(const BuildingDescr& gdescr)
   : Building(gdescr),
     building_(nullptr),
     builder_request_(nullptr),
     working_(false),
     work_steptime_(0),
     work_completed_(0),
     work_steps_(0) {
}

/*
===============
Set the type of building we're going to build
===============
*/
void PartiallyFinishedBuilding::set_building(const BuildingDescr& building_descr) {
	assert(!building_);

	building_ = &building_descr;
}

void PartiallyFinishedBuilding::cleanup(EditorGameBase& egbase) {
	if (builder_request_) {
		delete builder_request_;
		builder_request_ = nullptr;
	}

	for (WaresQueue* temp_ware : consume_wares_) {
		temp_ware->cleanup();
		delete temp_ware;
	}
	for (WaresQueue* temp_ware : dropout_wares_) {
		temp_ware->cleanup();
		delete temp_ware;
	}
	dropout_wares_.clear();
	consume_wares_.clear();

	Building::cleanup(egbase);
}

bool PartiallyFinishedBuilding::init(EditorGameBase& egbase) {
	Building::init(egbase);

	if (upcast(Game, game, &egbase)) {
		request_builder(*game);
	}

	return true;
}

/*
===============
Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void PartiallyFinishedBuilding::set_economy(Economy* const e, WareWorker type) {
	if (type == wwWARE) {
		if (Economy* const old = get_economy(type)) {
			for (WaresQueue* temp_ware : dropout_wares_) {
				temp_ware->remove_from_economy(*old);
			}
			for (WaresQueue* temp_ware : consume_wares_) {
				temp_ware->remove_from_economy(*old);
			}
		}
	}
	Building::set_economy(e, type);
	if (builder_request_ && type == builder_request_->get_type()) {
		builder_request_->set_economy(e);
	}

	if (e && type == wwWARE) {
		for (WaresQueue* temp_ware : dropout_wares_) {
			temp_ware->add_to_economy(*e);
		}
		for (WaresQueue* temp_ware : consume_wares_) {
			temp_ware->add_to_economy(*e);
		}
	}
}

/*
===============
Issue a request for the builder.
===============
*/
void PartiallyFinishedBuilding::request_builder(Game&) {
	assert(!builder_.is_set() && !builder_request_);

	builder_request_ = new Request(*this, owner().tribe().builder(),
	                               PartiallyFinishedBuilding::request_builder_callback, wwWORKER);
}

/*
===============
Override: construction size is always the same size as the building
===============
*/
int32_t PartiallyFinishedBuilding::get_size() const {
	return building_->get_size();
}

/*
===============
Override: Even though construction sites cannot be built themselves, you can
bulldoze them.
===============
*/
uint32_t PartiallyFinishedBuilding::get_playercaps() const {
	uint32_t caps = Building::get_playercaps();

	if (!is_destruction_blocked()) {
		caps |= PCap_Bulldoze;
	}
	caps &= ~PCap_Dismantle;

	return caps;
}

/*
===============
Return the animation for the building that is in construction, as this
should be more useful to the player.
===============
*/
const Image* PartiallyFinishedBuilding::representative_image() const {
	return building_->representative_image(&owner().get_playercolor());
}

/*
===============
Return the completion "percentage", where 2^16 = completely built,
0 = nothing built.
===============
*/
// TODO(unknown): should take gametime or so
uint32_t PartiallyFinishedBuilding::get_built_per64k() const {
	const uint32_t time = owner().egbase().get_gametime().get();
	uint32_t thisstep = 0;

	uint32_t ts = build_step_time().get();
	if (working_) {
		thisstep = ts - (work_steptime_.get() - time);
		// The check below is necessary because we drive construction via
		// the construction worker in get_building_work(), and there can be
		// a small delay between the worker completing his job and requesting
		// new work.
		if (thisstep > ts) {
			thisstep = ts;
		}
	}
	thisstep = (thisstep << 16) / ts;
	uint32_t total = (thisstep + (work_completed_ << 16));
	if (work_steps_) {
		total /= work_steps_;
	}

	// This assert is no longer true with a multithreaded race condition
	// assert(total <= (1 << 16));
	total = std::min<uint32_t>(total, 1 << 16);

	return total;
}

/*
===============
Called by transfer code when the builder has arrived on site.
===============
*/
void PartiallyFinishedBuilding::request_builder_callback(
   Game& game, Request& rq, DescriptionIndex, Worker* const w, PlayerImmovable& target) {
	assert(w);

	PartiallyFinishedBuilding& b = dynamic_cast<PartiallyFinishedBuilding&>(target);

	b.builder_ = w;

	delete &rq;
	b.builder_request_ = nullptr;

	w->start_task_buildingwork(game);
}

/*
===============
Override: Builders normally don't make (finished) buildings see.
PartiallyFinishedBuilding is an exception.
===============
*/
void PartiallyFinishedBuilding::add_worker(Worker& worker) {
	Building::add_worker(worker);
	set_seeing(true);
}
}  // namespace Widelands
