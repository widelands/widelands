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

#ifndef WL_ECONOMY_TRANSFER_H
#define WL_ECONOMY_TRANSFER_H

#include "economy/route.h"

namespace Widelands {
class Game;
struct PlayerImmovable;
class Request;
class WareInstance;
class Worker;

/**
 * Whenever an ware or worker is transferred to fulfill a Request,
 * a Transfer is allocated to describe this transfer.
 *
 * Transfers are always created and destroyed by a Request instance.
 *
 * Call get_next_step() to find out where you should go next. If
 * get_next_step() returns 0, the transfer is complete or cannot be
 * completed. Call has_finished() if success is true, has_failed() otherwise.
 * Call has_failed() if something really bad has happened (e.g. the worker
 * or ware was destroyed).
 */
struct Transfer {
	friend class Request;

	Transfer(Game&, Request&, WareInstance&);
	Transfer(Game&, Request&, Worker&);
	Transfer(Game&, WareInstance&);
	Transfer(Game&, Worker&);
	~Transfer();

	Request* get_request() const {
		return request_;
	}
	void set_request(Request* req);
	void set_destination(PlayerImmovable& imm);
	PlayerImmovable* get_destination(Game& g);
	uint32_t get_steps_left() const {
		return route_.get_nrsteps();
	}

	/// Called by the controlled ware or worker
	PlayerImmovable* get_next_step(PlayerImmovable*, bool& psuccess);
	void has_finished();
	void has_failed();

	struct ReadData {
		uint32_t destination;

		ReadData() : destination(0) {
		}
	};

	void read(FileRead& fr, ReadData& rd);
	void read_pointers(MapObjectLoader& mol, const ReadData& rd);
	void write(MapObjectSaver& mos, FileWrite& fw);

private:
	void tlog(char const* fmt, ...) PRINTF_FORMAT(2, 3);

	Game& game_;
	Request* request_;
	OPtr<PlayerImmovable> destination_;
	WareInstance* ware_;  ///< non-null iff this is transferring a ware
	Worker* worker_;      ///< non-null iff this is transferring a worker
	Route route_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_TRANSFER_H
