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

#include "economy/soldier_request.h"

#include <memory>

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/requirements.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/player.h"

namespace Widelands {

void SoldierRequestManager::create_request() {
	request_.reset(new SoldierRequest(
	   target_, target_.owner().tribe().soldier(), callback_, wwWORKER, preference_));
	request_->set_exact_match(true);
}

void SoldierRequestManager::set_economy(Economy* const e, WareWorker type) {
	if (type == wwWORKER && request_ != nullptr) {
		request_->set_economy(e);
	}
}

void SoldierRequestManager::update() {
	const Quantity target = get_desired_capacity_();
	const std::vector<Soldier*> stationed = get_stationed_soldiers_();
	const Quantity current = stationed.size();

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
		request_->set_preference(preference_);
		if (Economy* economy = request_->get_economy(); economy != nullptr) {
			economy->rebalance_supply();
		}
		return;
	}

	// Exchange soldiers, but only one at a time.
	if (current < 1) {
		request_.reset();
		return;
	}

	const SoldierDescr* descr = dynamic_cast<const SoldierDescr*>(
	   target_.owner().egbase().descriptions().get_worker_descr(target_.owner().tribe().soldier()));
	const int16_t max_level = descr->get_max_total_level();
	int16_t rmin = 0;
	int16_t rmax = max_level;

	switch (preference_) {
	case SoldierPreference::kAny:
		// No exchange desired
		request_.reset();
		return;

	case SoldierPreference::kHeroes: {
		int16_t worst_soldier = max_level;
		for (Soldier* soldier : stationed) {
			worst_soldier = std::min<int16_t>(worst_soldier, soldier->get_total_level());
		}
		rmin = worst_soldier + 1;
		break;
	}

	case SoldierPreference::kRookies: {
		int16_t best_soldier = 0;
		for (Soldier* soldier : stationed) {
			best_soldier = std::max<int16_t>(best_soldier, soldier->get_total_level());
		}
		rmax = best_soldier - 1;
		break;
	}

	default:
		NEVER_HERE();
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
	request_->set_preference(preference_);
	if (Economy* economy = request_->get_economy(); economy != nullptr) {
		economy->rebalance_supply();
	}
}

constexpr uint16_t kCurrentPacketVersion = 1;

void SoldierRequestManager::read(FileRead& fr, Game& game, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			preference_ = static_cast<SoldierPreference>(fr.unsigned_8());

			if (fr.unsigned_8() != 0) {
				create_request();
				request_->read(fr, game, mol);
				request_->set_preference(preference_);
			}
		} else {
			throw UnhandledVersionError(
			   "SoldierRequestManager", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw wexception("soldier request: %s", e.what());
	}
}

void SoldierRequestManager::write(FileWrite& fw, Game& game, MapObjectSaver& mos) const {
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
