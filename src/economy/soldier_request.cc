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

#include "economy/soldier_request.h"

#include <memory>

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/requirements.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/player.h"

namespace Widelands {

void SoldierRequest::create_request() {
	request_.reset(new Request(building_, building_.owner().tribe().soldier(), callback_, wwWORKER));
	request_->set_exact_match(true);
}

void SoldierRequest::update() {
	const SoldierControl* ctrl = building_.soldier_control();
	assert(ctrl != nullptr);
	const Quantity target = ctrl->soldier_capacity();
	const Quantity current = ctrl->stationed_soldiers().size();

	if (current > target) {
		// Site needs to drop soldiers, request nothing.
		request_.reset();
		return;
	}

	if (current < target) {
		// Site needs more soldiers, request anything.
		if (request_ == nullptr) {
			create_request();
		}

		request_->set_requirements(Requirements());
		request_->set_count(target - current);
		return;
	}

	// Exchange soldiers, but only one at a time.
	const int16_t max_level = dynamic_cast<const SoldierDescr*>(building_.owner().egbase().descriptions().get_worker_descr(
			building_.owner().tribe().soldier()))->get_max_total_level();
	int16_t rmin = 0;
	int16_t rmax = max_level;

	switch (preference_) {
	case SoldierPreference::kHeroes: {
		int16_t worst_soldier = max_level;
		for (Soldier* soldier : ctrl->stationed_soldiers()) {
			worst_soldier = std::min<int16_t>(worst_soldier, soldier->get_total_level());
		}
		rmin = worst_soldier + 1;
		break;
	}

	case SoldierPreference::kRookies: {
		int16_t best_soldier = 0;
		for (Soldier* soldier : ctrl->stationed_soldiers()) {
			best_soldier = std::max<int16_t>(best_soldier, soldier->get_total_level());
		}
		rmax = best_soldier - 1;
		break;
	}
	}

	if (rmax < rmin || rmin > max_level || rmax < 0) {
		// Soldier request is already saturated with all-optimal fits.
		request_.reset();
		return;
	}

	if (request_ == nullptr) {
		create_request();
	}

	request_->set_count(1);
	request_->set_requirements(RequireAttribute(TrainingAttribute::kTotal, rmin, rmax));
}

constexpr uint16_t kCurrentPacketVersion = 1;

void SoldierRequest::read(FileRead& fr, Game& game, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			preference_ = static_cast<SoldierPreference>(fr.unsigned_8());

			if (fr.unsigned_8() != 0) {
				create_request();
				request_->read(fr, game, mol);
			}
		} else {
			throw UnhandledVersionError("SoldierRequest", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw wexception("soldier request: %s", e.what());
	}
}

void SoldierRequest::write(FileWrite& fw, Game& game, MapObjectSaver& mos) const {
	fw.unsigned_16(kCurrentPacketVersion);

	fw.unsigned_8(static_cast<uint8_t>(preference_));

	if (request_ == nullptr) {
		fw.unsigned_8(0);
	} else {
		fw.unsigned_8(1);
		request_->write(fw, game, mos);
	}
}

}  // namespace Widelands
