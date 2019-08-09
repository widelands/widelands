/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

ProductionsiteSettings::ProductionsiteSettings(const ProductionSiteDescr& descr)
   : BuildingSettings(descr.name()), stopped(false) {
	for (const auto& pair : descr.input_wares()) {
		ware_queues.push_back(
		   std::make_pair(pair.first, InputQueueSetting{pair.second, pair.second, kPriorityNormal}));
	}
	for (const auto& pair : descr.input_workers()) {
		worker_queues.push_back(
		   std::make_pair(pair.first, InputQueueSetting{pair.second, pair.second, kPriorityNormal}));
	}
}

MilitarysiteSettings::MilitarysiteSettings(const MilitarySiteDescr& descr)
   : BuildingSettings(descr.name()),
     max_capacity(descr.get_max_number_of_soldiers()),
     desired_capacity(descr.get_max_number_of_soldiers()),
     prefer_heroes(descr.prefers_heroes_at_start_) {
}

TrainingsiteSettings::TrainingsiteSettings(const TrainingSiteDescr& descr)
   : ProductionsiteSettings(descr),
     max_capacity(descr.get_max_number_of_soldiers()),
     desired_capacity(descr.get_max_number_of_soldiers()) {
}

WarehouseSettings::WarehouseSettings(const WarehouseDescr& wh, const TribeDescr& tribe)
   : BuildingSettings(wh.name()),
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

void ProductionsiteSettings::apply(const BuildingSettings& bs) {
	BuildingSettings::apply(bs);
	if (upcast(const ProductionsiteSettings, s, &bs)) {
		stopped = s->stopped;
		for (auto& pair : ware_queues) {
			for (const auto& other : s->ware_queues) {
				if (pair.first == other.first) {
					pair.second.priority = other.second.priority;
					pair.second.desired_fill = std::min(pair.second.max_fill, other.second.desired_fill);
					break;
				}
			}
		}
		for (auto& pair : worker_queues) {
			for (const auto& other : s->worker_queues) {
				if (pair.first == other.first) {
					pair.second.priority = other.second.priority;
					pair.second.desired_fill = std::min(pair.second.max_fill, other.second.desired_fill);
					break;
				}
			}
		}
	}
}

void TrainingsiteSettings::apply(const BuildingSettings& bs) {
	ProductionsiteSettings::apply(bs);
	if (upcast(const TrainingsiteSettings, s, &bs)) {
		desired_capacity = std::min(max_capacity, s->desired_capacity);
	}
}

void MilitarysiteSettings::apply(const BuildingSettings& bs) {
	BuildingSettings::apply(bs);
	if (upcast(const MilitarysiteSettings, s, &bs)) {
		desired_capacity = std::min(max_capacity, s->desired_capacity);
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

constexpr uint8_t kCurrentPacketVersion = 1;
constexpr uint8_t kCurrentPacketVersionMilitarysite = 1;
constexpr uint8_t kCurrentPacketVersionProductionsite = 1;
constexpr uint8_t kCurrentPacketVersionTrainingsite = 1;
constexpr uint8_t kCurrentPacketVersionWarehouse = 1;

// static
BuildingSettings* BuildingSettings::load(const Game& game, const TribeDescr& tribe, FileRead& fr) {
	try {
		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			const std::string name(fr.c_string());
			const DescriptionIndex index = game.tribes().building_index(name);
			const BuildingType type = static_cast<BuildingType>(fr.unsigned_8());
			BuildingSettings* result = nullptr;
			switch (type) {
			case BuildingType::kTrainingsite: {
				result = new TrainingsiteSettings(
				   *dynamic_cast<const TrainingSiteDescr*>(game.tribes().get_building_descr(index)));
				break;
			}
			case BuildingType::kProductionsite: {
				result = new ProductionsiteSettings(
				   *dynamic_cast<const ProductionSiteDescr*>(game.tribes().get_building_descr(index)));
				break;
			}
			case BuildingType::kMilitarysite: {
				result = new MilitarysiteSettings(
				   *dynamic_cast<const MilitarySiteDescr*>(game.tribes().get_building_descr(index)));
				break;
			}
			case BuildingType::kWarehouse: {
				result = new WarehouseSettings(
				   *dynamic_cast<const WarehouseDescr*>(game.tribes().get_building_descr(index)),
				   tribe);
				break;
			}
			}
			if (!result) {
				throw wexception(
				   "Unknown building category %u (%s)", static_cast<uint8_t>(type), name.c_str());
			}
			result->read(game, fr);
			return result;
		} else {
			throw UnhandledVersionError(
			   "BuildingSettings_load", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("BuildingSettings_load: %s", e.what());
	}
	NEVER_HERE();
}

void BuildingSettings::read(const Game&, FileRead&) {
	// Header was peeled away by load()
}

void BuildingSettings::save(const Game&, FileWrite& fw) const {
	fw.unsigned_8(kCurrentPacketVersion);
	fw.c_string(descr_.c_str());
	fw.unsigned_8(static_cast<uint8_t>(type()));
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
				const DescriptionIndex di = fr.unsigned_32();
				const uint32_t fill = fr.unsigned_32();
				const int32_t priority = fr.signed_32();
				ware_queues.at(i).first = di;
				ware_queues.at(i).second.desired_fill = fill;
				ware_queues.at(i).second.priority = priority;
			}
			for (uint32_t i = 0; i < nr_workers; ++i) {
				const DescriptionIndex di = fr.unsigned_32();
				const uint32_t fill = fr.unsigned_32();
				const int32_t priority = fr.signed_32();
				worker_queues.at(i).first = di;
				worker_queues.at(i).second.desired_fill = fill;
				worker_queues.at(i).second.priority = priority;
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
		fw.unsigned_32(pair.first);
		fw.unsigned_32(pair.second.desired_fill);
		fw.signed_32(pair.second.priority);
	}
	for (const auto& pair : worker_queues) {
		fw.unsigned_32(pair.first);
		fw.unsigned_32(pair.second.desired_fill);
		fw.signed_32(pair.second.priority);
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
				const DescriptionIndex di = fr.unsigned_32();
				const uint8_t pref = fr.unsigned_8();
				ware_preferences[di] = static_cast<StockPolicy>(pref);
			}
			for (uint32_t i = 0; i < nr_workers; ++i) {
				const DescriptionIndex di = fr.unsigned_32();
				const uint8_t pref = fr.unsigned_8();
				worker_preferences[di] = static_cast<StockPolicy>(pref);
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
		fw.unsigned_32(pair.first);
		fw.unsigned_8(static_cast<uint8_t>(pair.second));
	}
	for (const auto& pair : worker_preferences) {
		fw.unsigned_32(pair.first);
		fw.unsigned_8(static_cast<uint8_t>(pair.second));
	}
}

}  // namespace Widelands
