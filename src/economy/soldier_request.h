/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#include <memory>

#include "base/macros.h"
#include "economy/request.h"
#include "logic/widelands.h"

namespace Widelands {

class SoldierRequest {
public:
	SoldierRequest(Building& target_building, SoldierPreference pref, Request::CallbackFn callback)
	: building_(target_building), preference_(pref), callback_(callback) {
	}

	void update();

	void set_preference(SoldierPreference p) {
		preference_ = p;
	}

	[[nodiscard]] Building& get_building() const {
		return building_;
	}
	[[nodiscard]] SoldierPreference get_preference() const {
		return preference_;
	}

	[[nodiscard]] const Request* get_request() const {
		return request_.get();
	}
	[[nodiscard]] Request* get_request() {
		return request_.get();
	}

	void read(FileRead& fr, Game& game, MapObjectLoader& mol);
	void write(FileWrite& fw, Game& game, MapObjectSaver& mos) const;

private:
	void create_request();

	Building& building_;
	SoldierPreference preference_;
	Request::CallbackFn callback_;

	std::unique_ptr<Request> request_;

	DISALLOW_COPY_AND_ASSIGN(SoldierRequest);
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_SOLDIER_REQUEST_H
