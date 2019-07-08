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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_BUILDING_SETTINGS_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_BUILDING_SETTINGS_H

#include <map>
#include <string>
#include <vector>

#include "logic/widelands.h"

class FileRead;
class FileWrite;

namespace Widelands {

class Game;
class MilitarySiteDescr;
class ProductionSiteDescr;
enum class StockPolicy;
class TrainingSiteDescr;
class TribeDescr;
class WarehouseDescr;

struct BuildingSettings {
	BuildingSettings(const std::string& name) : descr_(name) {
	}
	virtual ~BuildingSettings() {
	}

	static BuildingSettings* load(const Game&, const TribeDescr&, FileRead&);

	virtual void save(const Game&, FileWrite&) const;
	virtual void read(const Game&, FileRead&);

	virtual void apply(const BuildingSettings&) {
	}

protected:
	enum class BuildingType : uint8_t {
		kWarehouse = 1,
		kProductionsite = 2,
		kTrainingsite = 3,
		kMilitarysite = 4,
	};
	virtual BuildingType type() const = 0;

private:
	const std::string descr_;
};

struct ProductionsiteSettings : public BuildingSettings {
	ProductionsiteSettings(const ProductionSiteDescr& descr);
	~ProductionsiteSettings() override {
	}
	void apply(const BuildingSettings&) override;

	void save(const Game&, FileWrite&) const override;
	void read(const Game&, FileRead&) override;

	struct InputQueueSetting {
		const uint32_t max_fill;
		uint32_t desired_fill;
		int32_t priority;
	};
	std::vector<std::pair<DescriptionIndex, InputQueueSetting>> ware_queues;
	std::vector<std::pair<DescriptionIndex, InputQueueSetting>> worker_queues;
	bool stopped;

protected:
	BuildingType type() const override {
		return BuildingType::kProductionsite;
	}
};

struct MilitarysiteSettings : public BuildingSettings {
	MilitarysiteSettings(const MilitarySiteDescr&);
	~MilitarysiteSettings() override {
	}
	void apply(const BuildingSettings&) override;

	void save(const Game&, FileWrite&) const override;
	void read(const Game&, FileRead&) override;

	const uint32_t max_capacity;
	uint32_t desired_capacity;
	bool prefer_heroes;

protected:
	BuildingType type() const override {
		return BuildingType::kMilitarysite;
	}
};

struct TrainingsiteSettings : public ProductionsiteSettings {
	TrainingsiteSettings(const TrainingSiteDescr&);
	~TrainingsiteSettings() override {
	}
	void apply(const BuildingSettings&) override;

	void save(const Game&, FileWrite&) const override;
	void read(const Game&, FileRead&) override;

	const uint32_t max_capacity;
	uint32_t desired_capacity;

protected:
	BuildingType type() const override {
		return BuildingType::kTrainingsite;
	}
};

struct WarehouseSettings : public BuildingSettings {
	WarehouseSettings(const WarehouseDescr&, const TribeDescr&);
	~WarehouseSettings() override {
	}
	void apply(const BuildingSettings&) override;

	void save(const Game&, FileWrite&) const override;
	void read(const Game&, FileRead&) override;

	std::map<DescriptionIndex, StockPolicy> ware_preferences;
	std::map<DescriptionIndex, StockPolicy> worker_preferences;
	const bool launch_expedition_allowed;
	bool launch_expedition;

protected:
	BuildingType type() const override {
		return BuildingType::kWarehouse;
	}
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_BUILDING_SETTINGS_H
