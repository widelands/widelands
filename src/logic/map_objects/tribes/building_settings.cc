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

#include "logic/map_objects/tribes/building_settings.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"

namespace Widelands {

ProductionsiteSettings::ProductionsiteSettings(const ProductionSiteDescr& descr,
                                               const TribeDescr& tribe)
   : BuildingSettings(descr.name(), tribe), stopped(false) {
	for (const auto& pair : descr.input_wares()) {
		ware_queues.insert(std::make_pair(
		   pair.first, InputQueueSetting{pair.second, pair.second, WarePriority::kNormal}));
	}
	for (const auto& pair : descr.input_workers()) {
		worker_queues.insert(std::make_pair(
		   pair.first, InputQueueSetting{pair.second, pair.second, WarePriority::kNormal}));
	}
}

MilitarysiteSettings::MilitarysiteSettings(const MilitarySiteDescr& descr, const TribeDescr& tribe)
   : BuildingSettings(descr.name(), tribe),
     max_capacity(descr.get_max_number_of_soldiers()),
     desired_capacity(descr.get_max_number_of_soldiers()),
     prefer_heroes(descr.prefers_heroes_at_start_) {
}

TrainingsiteSettings::TrainingsiteSettings(const TrainingSiteDescr& descr, const TribeDescr& tribe)
   : ProductionsiteSettings(descr, tribe),
     max_capacity(descr.get_max_number_of_soldiers()),
     desired_capacity(descr.get_max_number_of_soldiers()) {
}

WarehouseSettings::WarehouseSettings(const WarehouseDescr& wh, const TribeDescr& tribe)
   : BuildingSettings(wh.name(), tribe),
     launch_expedition_allowed(wh.get_isport()),
     launch_expedition(false) {
	for (const DescriptionIndex di : tribe.wares()) {
		ware_preferences.emplace(di, StockPolicy::kNormal);
	}
	for (const DescriptionIndex di : tribe.workers()) {
		worker_preferences.emplace(di, StockPolicy::kNormal);
	}
	for (const DescriptionIndex di : tribe.worker_types_without_cost()) {
		worker_preferences.erase(di);
	}
}

inline uint32_t new_desired_capacity(uint32_t old_max, uint32_t old_des, uint32_t new_max) {
	return old_des * new_max / old_max;
}

void ProductionsiteSettings::apply(const BuildingSettings& bs) {
	BuildingSettings::apply(bs);
	if (upcast(const ProductionsiteSettings, s, &bs)) {
		stopped = s->stopped;
		for (auto& pair : ware_queues) {
			for (const auto& other : s->ware_queues) {
				if (pair.first == other.first) {
					pair.second.priority = other.second.priority;
					pair.second.desired_fill = new_desired_capacity(
					   other.second.max_fill, other.second.desired_fill, pair.second.max_fill);
					break;
				}
			}
		}
		for (auto& pair : worker_queues) {
			for (const auto& other : s->worker_queues) {
				if (pair.first == other.first) {
					pair.second.priority = other.second.priority;
					pair.second.desired_fill = new_desired_capacity(
					   other.second.max_fill, other.second.desired_fill, pair.second.max_fill);
					break;
				}
			}
		}
	}
}

void TrainingsiteSettings::apply(const BuildingSettings& bs) {
	ProductionsiteSettings::apply(bs);
	if (upcast(const TrainingsiteSettings, s, &bs)) {
		desired_capacity =
		   new_desired_capacity(s->max_capacity, s->desired_capacity, desired_capacity);
	}
}

void MilitarysiteSettings::apply(const BuildingSettings& bs) {
	BuildingSettings::apply(bs);
	if (upcast(const MilitarysiteSettings, s, &bs)) {
		desired_capacity =
		   new_desired_capacity(s->max_capacity, s->desired_capacity, desired_capacity);
		prefer_heroes = s->prefer_heroes;
	}
}

void WarehouseSettings::apply(const BuildingSettings& bs) {
	BuildingSettings::apply(bs);
	if (upcast(const WarehouseSettings, s, &bs)) {
		for (auto& pair : ware_preferences) {
			const auto it = s->ware_preferences.find(pair.first);
			if (it != s->ware_preferences.end()) {
				pair.second = it->second;
			}
		}
		for (auto& pair : worker_preferences) {
			const auto it = s->worker_preferences.find(pair.first);
			if (it != s->worker_preferences.end()) {
				pair.second = it->second;
			}
		}
		launch_expedition = launch_expedition_allowed && s->launch_expedition;
	}
}

// Saveloading

constexpr uint8_t kCurrentPacketVersion = 2;
constexpr uint8_t kCurrentPacketVersionMilitarysite = 1;
constexpr uint8_t kCurrentPacketVersionProductionsite = 2;
constexpr uint8_t kCurrentPacketVersionTrainingsite = 1;
constexpr uint8_t kCurrentPacketVersionWarehouse = 2;

// static
BuildingSettings* BuildingSettings::load(const Game& game, const TribeDescr& tribe, FileRead& fr) {
	try {
		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			const std::string name(fr.c_string());
			const DescriptionIndex index = tribe.safe_building_index(name);
			const BuildingDescr* descr = tribe.get_building_descr(index);
			BuildingSettings* result = nullptr;
			switch (descr->type()) {
			case MapObjectType::TRAININGSITE: {
				result =
				   new TrainingsiteSettings(*dynamic_cast<const TrainingSiteDescr*>(descr), tribe);
				break;
			}
			case MapObjectType::PRODUCTIONSITE: {
				result =
				   new ProductionsiteSettings(*dynamic_cast<const ProductionSiteDescr*>(descr), tribe);
				break;
			}
			case MapObjectType::MILITARYSITE: {
				result =
				   new MilitarysiteSettings(*dynamic_cast<const MilitarySiteDescr*>(descr), tribe);
				break;
			}
			case MapObjectType::WAREHOUSE: {
				result = new WarehouseSettings(*dynamic_cast<const WarehouseDescr*>(descr), tribe);
				break;
			}
			default:
				throw GameDataError("Unsupported building category %s (%s)",
				                    to_string(descr->type()).c_str(), name.c_str());
			}
			result->read(game, fr);
			return result;
		}
		throw UnhandledVersionError("BuildingSettings_load", packet_version, kCurrentPacketVersion);
	} catch (const WException& e) {
		throw GameDataError("BuildingSettings_load: %s", e.what());
	}
	NEVER_HERE();
}

void BuildingSettings::read(const Game& /* game */, FileRead& /* fr */) {
	// Header was peeled away by load()
}

void BuildingSettings::save(const Game& /* game */, FileWrite& fw) const {
	fw.unsigned_8(kCurrentPacketVersion);
	fw.c_string(descr_.c_str());
}

void MilitarysiteSettings::read(const Game& game, FileRead& fr) {
	BuildingSettings::read(game, fr);
	try {
		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionMilitarysite) {
			desired_capacity = fr.unsigned_32();
			prefer_heroes = fr.unsigned_8();
		} else {
			throw UnhandledVersionError(
			   "MilitarysiteSettings", packet_version, kCurrentPacketVersionMilitarysite);
		}
	} catch (const WException& e) {
		throw GameDataError("MilitarysiteSettings: %s", e.what());
	}
}

void MilitarysiteSettings::save(const Game& game, FileWrite& fw) const {
	BuildingSettings::save(game, fw);
	fw.unsigned_8(kCurrentPacketVersionMilitarysite);

	fw.unsigned_32(desired_capacity);
	fw.unsigned_8(prefer_heroes ? 1 : 0);
}

void ProductionsiteSettings::read(const Game& game, FileRead& fr) {
	BuildingSettings::read(game, fr);
	try {
		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionProductionsite) {
			stopped = fr.unsigned_8();
			const uint32_t nr_wares = fr.unsigned_32();
			const uint32_t nr_workers = fr.unsigned_32();
			for (uint32_t i = 0; i < nr_wares; ++i) {
				const DescriptionIndex di = tribe_.safe_ware_index(fr.c_string());
				const uint32_t fill = fr.unsigned_32();
				const WarePriority priority(fr);
				// Set the fill and priority if the queue exists.
				// The check protects against changes in the input wares - we simply keep using the
				// default instead in that case.
				if (ware_queues.count(di) == 1) {
					InputQueueSetting& setme = ware_queues.at(di);
					// Don't fill it with more than currently possible
					setme.desired_fill = std::min(setme.max_fill, fill);
					setme.priority = priority;
				}
			}
			for (uint32_t i = 0; i < nr_workers; ++i) {
				const DescriptionIndex di = tribe_.safe_worker_index(fr.c_string());
				const uint32_t fill = fr.unsigned_32();
				const WarePriority priority(fr);
				// Set the fill and priority if the queue exists.
				// The check protects against changes in the input workers - we simply keep using the
				// default instead in that case.
				if (worker_queues.count(di) == 1) {
					InputQueueSetting& setme = worker_queues.at(di);
					// Don't fill it with more than currently possible
					setme.desired_fill = std::min(setme.max_fill, fill);
					setme.priority = priority;
				}
			}
		} else {
			throw UnhandledVersionError(
			   "ProductionsiteSettings", packet_version, kCurrentPacketVersionProductionsite);
		}
	} catch (const WException& e) {
		throw GameDataError("ProductionsiteSettings: %s", e.what());
	}
}

void ProductionsiteSettings::save(const Game& game, FileWrite& fw) const {
	BuildingSettings::save(game, fw);
	fw.unsigned_8(kCurrentPacketVersionProductionsite);

	fw.unsigned_8(stopped ? 1 : 0);
	fw.unsigned_32(ware_queues.size());
	fw.unsigned_32(worker_queues.size());
	for (const auto& pair : ware_queues) {
		fw.c_string(tribe_.get_ware_descr(pair.first)->name());
		fw.unsigned_32(pair.second.desired_fill);
		pair.second.priority.write(fw);
	}
	for (const auto& pair : worker_queues) {
		fw.c_string(tribe_.get_worker_descr(pair.first)->name());
		fw.unsigned_32(pair.second.desired_fill);
		pair.second.priority.write(fw);
	}
}

void TrainingsiteSettings::read(const Game& game, FileRead& fr) {
	ProductionsiteSettings::read(game, fr);
	try {
		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionTrainingsite) {
			desired_capacity = fr.unsigned_32();
		} else {
			throw UnhandledVersionError(
			   "TrainingsiteSettings", packet_version, kCurrentPacketVersionTrainingsite);
		}
	} catch (const WException& e) {
		throw GameDataError("TrainingsiteSettings: %s", e.what());
	}
}

void TrainingsiteSettings::save(const Game& game, FileWrite& fw) const {
	ProductionsiteSettings::save(game, fw);
	fw.unsigned_8(kCurrentPacketVersionTrainingsite);
	fw.unsigned_32(desired_capacity);
}

void WarehouseSettings::read(const Game& game, FileRead& fr) {
	BuildingSettings::read(game, fr);
	try {
		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionWarehouse) {
			launch_expedition = fr.unsigned_8();
			const uint32_t nr_wares = fr.unsigned_32();
			const uint32_t nr_workers = fr.unsigned_32();
			for (uint32_t i = 0; i < nr_wares; ++i) {
				const DescriptionIndex di = tribe_.safe_ware_index(fr.c_string());
				const uint8_t pref = fr.unsigned_8();
				// Condition protects against changes in the tribe's roster
				if (tribe_.has_ware(di)) {
					ware_preferences[di] = static_cast<StockPolicy>(pref);
				}
			}
			for (uint32_t i = 0; i < nr_workers; ++i) {
				const DescriptionIndex di = tribe_.safe_worker_index(fr.c_string());
				const uint8_t pref = fr.unsigned_8();
				// Condition protects against changes in the tribe's roster
				if (tribe_.has_worker(di)) {
					worker_preferences[di] = static_cast<StockPolicy>(pref);
				}
			}
		} else {
			throw UnhandledVersionError(
			   "WarehouseSettings", packet_version, kCurrentPacketVersionWarehouse);
		}
	} catch (const WException& e) {
		throw GameDataError("WarehouseSettings: %s", e.what());
	}
}

void WarehouseSettings::save(const Game& game, FileWrite& fw) const {
	BuildingSettings::save(game, fw);
	fw.unsigned_8(kCurrentPacketVersionWarehouse);

	fw.unsigned_8(launch_expedition ? 1 : 0);
	fw.unsigned_32(ware_preferences.size());
	fw.unsigned_32(worker_preferences.size());
	for (const auto& pair : ware_preferences) {
		fw.c_string(tribe_.get_ware_descr(pair.first)->name());
		fw.unsigned_8(static_cast<uint8_t>(pair.second));
	}
	for (const auto& pair : worker_preferences) {
		fw.c_string(tribe_.get_worker_descr(pair.first)->name());
		fw.unsigned_8(static_cast<uint8_t>(pair.second));
	}
}

}  // namespace Widelands
