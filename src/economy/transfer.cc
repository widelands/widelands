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

#include "economy/transfer.h"

#include "base/log.h"
#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/roadbase.h"
#include "economy/ware_instance.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

Transfer::Transfer(Game& game, Request& req, WareInstance& it)
   : game_(game), request_(&req), destination_(&req.target()), ware_(&it), worker_(nullptr) {
	ware_->set_transfer(game, *this);
}

Transfer::Transfer(Game& game, Request& req, Worker& w)
   : game_(game), request_(&req), destination_(&req.target()), ware_(nullptr), worker_(&w) {
	worker_->start_task_transfer(game, this);
}

/**
 * Create a transfer without linking it into the
 * given ware instance and without a request.
 */
Transfer::Transfer(Game& game, WareInstance& w)
   : game_(game), request_(nullptr), ware_(&w), worker_(nullptr) {
}

/**
 * Create a transfer without linking it into the given
 * worker and without a request.
 */
Transfer::Transfer(Game& game, Worker& w)
   : game_(game), request_(nullptr), ware_(nullptr), worker_(&w) {
}

/**
 * Cleanup.
 */
Transfer::~Transfer() {
	if (worker_) {
		assert(!ware_);

		worker_->cancel_task_transfer(game_);
	} else if (ware_) {
		ware_->cancel_transfer(game_);
	}
}

/**
 * Override the \ref Request of this transfer.
 *
 * \note Only use for loading savegames
 */
void Transfer::set_request(Request* req) {
	assert(!request_);
	assert(req);

	if (&req->target() != destination_.get(game_)) {
		if (destination_.is_set()) {
			log_warn_time(game_.get_gametime(),
			              "Transfer::set_request req->target (%u) "
			              "vs. destination (%u) mismatch\n",
			              req->target().serial(), destination_.serial());
		}
		destination_ = &req->target();
	}
	request_ = req;
}

/**
 * Set the destination for a transfer that has no associated \ref Request.
 */
void Transfer::set_destination(PlayerImmovable& imm) {
	assert(!request_);

	destination_ = &imm;
}

/**
 * Get this transfer's destination.
 */
PlayerImmovable* Transfer::get_destination(Game& g) {
	return destination_.get(g);
}

/**
 * Determine where we should be going from our current location.
 */
PlayerImmovable* Transfer::get_next_step(PlayerImmovable* const location, bool& success) {
	assert((worker_ == nullptr) ^ (ware_ == nullptr));
	const WareWorker type = worker_ ? wwWORKER : wwWARE;
	if (!location || !location->get_economy(type)) {
		tlog("no location or economy -> fail\n");
		success = false;
		return nullptr;
	}

	PlayerImmovable* destination = destination_.get(location->get_economy(type)->owner().egbase());
	if (!destination || destination->get_economy(type) != location->get_economy(type)) {
		tlog("destination disappeared or economy mismatch -> fail\n");
		success = false;
		return nullptr;
	}

	success = true;

	if (location == destination) {
		return nullptr;
	}

	Flag& locflag = location->base_flag();
	Flag& destflag = destination->base_flag();

	if (&locflag == &destflag) {
		return &locflag == location ? destination : &locflag;
	}

	// Brute force: recalculate the best route every time
	if (!locflag.get_economy(type)->find_route(locflag, destflag, &route_)) {
		tlog("destination appears to have become split from current location -> fail\n");
		Economy::check_split(locflag, destflag, type);
		success = false;
		return nullptr;
	}

	if (route_.get_nrsteps() >= 1) {
		if (upcast(RoadBase const, road, location)) {
			if (&road->get_flag(RoadBase::FlagEnd) == &route_.get_flag(game_, 1)) {
				route_.trim_start(1);
			}
		}
	}

	if (route_.get_nrsteps() >= 1) {
		if (upcast(RoadBase const, road, destination)) {
			if (&road->get_flag(RoadBase::FlagEnd) ==
			    &route_.get_flag(game_, route_.get_nrsteps() - 1)) {
				route_.truncate(route_.get_nrsteps() - 1);
			}
		}
	}

	// Reroute into PortDocks or the associated warehouse when appropriate
	if (route_.get_nrsteps() >= 1) {
		Flag& curflag(route_.get_flag(game_, 0));
		Flag& nextflag(route_.get_flag(game_, 1));
		if (type == wwWORKER ? curflag.get_road(nextflag) == nullptr :
                             curflag.get_roadbase(nextflag) == nullptr) {
			upcast(Warehouse, wh, curflag.get_building());
			assert(wh);

			PortDock* pd = wh->get_portdock();
			assert(pd);

			if (location == pd) {
				return pd->get_dock(nextflag);
			}
			if (location == wh) {
				return pd;
			}
			if (location == &curflag || ware_) {
				return wh;
			}
			return &curflag;
		}

		if (ware_ && location == &curflag && route_.get_nrsteps() >= 2) {
			Flag& nextnextflag(route_.get_flag(game_, 2));
			if (nextflag.get_roadbase(nextnextflag) == nullptr) {
				assert(nextflag.get_building());
				assert(nextflag.get_building()->descr().type() == MapObjectType::WAREHOUSE);
				return nextflag.get_building();
			}
		}
	}

	// Now decide where we want to go
	if (location && location->descr().type() == Widelands::MapObjectType::FLAG) {
		assert(&route_.get_flag(game_, 0) == location);

		// special rule to get wares into buildings
		if (ware_ && route_.get_nrsteps() == 1) {
			if (destination && destination->descr().type() >= Widelands::MapObjectType::BUILDING) {
				assert(&route_.get_flag(game_, 1) == &destflag);

				return destination;
			}
		}

		if (route_.get_nrsteps() >= 1) {
			return &route_.get_flag(game_, 1);
		}

		return destination;
	}

	return &route_.get_flag(game_, 0);
}

/**
 * Transfer finished successfully.
 * This Transfer object will be deleted.
 * The caller might be destroyed, too.
 */
void Transfer::has_finished() {
	if (request_) {
		request_->transfer_finish(game_, *this);
	} else {
		PlayerImmovable* destination = destination_.get(game_);
		assert(destination);
		if (worker_) {
			destination->receive_worker(game_, *worker_);
			worker_ = nullptr;
		} else {
			destination->receive_ware(game_, ware_->descr_index());
			ware_->destroy(game_);
			ware_ = nullptr;
		}

		delete this;
	}
}

/**
 * Transfer failed for reasons beyond our control.
 * This Transfer object will be deleted.
 */
void Transfer::has_failed() {
	if (request_) {
		request_->transfer_fail(game_, *this);
	} else {
		delete this;
	}
}

void Transfer::tlog(char const* const fmt, ...) {
	if (!g_verbose) {
		return;
	}
	char buffer[1024];
	va_list va;
	char id;
	uint32_t serial;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (worker_) {
		id = 'W';
		serial = worker_->serial();
	} else if (ware_) {
		id = 'I';
		serial = ware_->serial();
	} else {
		id = '?';
		serial = 0;
	}

	log_dbg_time(game_.get_gametime(), "T%c(%u): %s", id, serial, buffer);
}

/*
==============================

Load/save support

==============================
*/

constexpr uint8_t kCurrentPacketVersion = 1;

void Transfer::read(FileRead& fr, Transfer::ReadData& rd) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			rd.destination = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("Transfer", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw wexception("transfer: %s", e.what());
	}
}

void Transfer::read_pointers(MapObjectLoader& mol, const Widelands::Transfer::ReadData& rd) {
	if (rd.destination) {
		destination_ = &mol.get<PlayerImmovable>(rd.destination);
	}
}

void Transfer::write(MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(kCurrentPacketVersion);
	fw.unsigned_32(mos.get_object_file_index_or_zero(destination_.get(game_)));
	// not saving route right now, will be recaculated anyway
}
}  // namespace Widelands
