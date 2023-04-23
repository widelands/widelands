/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_CONSTRUCTIONSITE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_CONSTRUCTIONSITE_H

#include <memory>

#include "base/macros.h"
#include "logic/map_objects/tribes/building_settings.h"
#include "logic/map_objects/tribes/partially_finished_building.h"
#include "scripting/lua_table.h"

namespace Widelands {

class Building;
enum class StockPolicy;

/// Per-player and per-field constructionsite information
struct ConstructionsiteInformation {
	ConstructionsiteInformation() = default;

	/// Draw the partly finished constructionsite
	void draw(const Vector2f& point_on_dst,
	          const Coords& coords,
	          float scale,
	          bool visible,
	          const RGBColor& player_color,
	          RenderTarget* dst) const;

	const BuildingDescr* becomes{
	   nullptr};  // Also works as a marker telling whether there is a construction site.
	const BuildingDescr* was{nullptr};  // only valid if "becomes" is an enhanced building.
	std::vector<const BuildingDescr*>
	   intermediates;  // If we enhance a building while it's still under construction
	Duration totaltime{0U};
	Duration completedtime{0U};
};

/*
ConstructionSite
----------------
A "building" that acts as a placeholder during construction work.
Once the construction phase is finished, the ConstructionSite is removed
and replaced by the actual building.

A construction site can have a worker.
A construction site has one (or more) input wares types, each with an
  associated store.

Note that the ConstructionSiteDescr class is mostly a dummy class.
The ConstructionSite is derived from Building so that it fits in more cleanly
with the transport and Flag code.

Every tribe has exactly one ConstructionSiteDescr.
The ConstructionSite's idling animation is the basic construction site marker.
*/
class ConstructionSiteDescr : public BuildingDescr {
public:
	ConstructionSiteDescr(const std::string& init_descname,
	                      const LuaTable& t,
	                      Descriptions& descriptions);
	~ConstructionSiteDescr() override = default;

	[[nodiscard]] Building& create_object() const override;

	[[nodiscard]] FxId creation_fx() const;

private:
	const FxId creation_fx_;

	DISALLOW_COPY_AND_ASSIGN(ConstructionSiteDescr);
};

class ConstructionSite : public PartiallyFinishedBuilding {
	friend class MapBuildingdataPacket;

	MO_DESCR(ConstructionSiteDescr)

public:
	explicit ConstructionSite(const ConstructionSiteDescr& descr);

	const ConstructionsiteInformation& get_info() {
		return info_;
	}

	InputQueue& inputqueue(DescriptionIndex, WareWorker, const Request*) override;

	void set_building(const BuildingDescr&) override;
	const BuildingDescr& building() const {
		return *building_;
	}

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

	bool burn_on_destroy() override;

	bool fetch_from_flag(Game&) override;
	bool get_building_work(Game&, Worker&, bool success) override;

	void add_additional_ware(DescriptionIndex);
	void add_additional_worker(Game&, Worker&);
	const std::map<DescriptionIndex, uint8_t>& get_additional_wares() const {
		return additional_wares_;
	}
	const std::vector<Worker*>& get_additional_workers() const {
		return additional_workers_;
	}

	BuildingSettings* get_settings() const {
		return settings_.get();
	}
	void apply_settings(const BuildingSettings&);

	void enhance(const EditorGameBase&);

	void add_dropout_wares(const std::map<DescriptionIndex, Quantity>&);

protected:
	void update_statistics_string(std::string* statistics_string) override;

	static constexpr Duration kConstructionsiteStepTime{30000};
	const Duration& build_step_time() const override {
		return kConstructionsiteStepTime;
	}

	static void wares_queue_callback(Game&, InputQueue*, DescriptionIndex, Worker*, void* data);

	void draw(const Time& gametime,
	          InfoToDraw info_to_draw,
	          const Vector2f& point_on_dst,
	          const Coords& coords,
	          float scale,
	          RenderTarget* dst) override;

private:
	int32_t fetchfromflag_{0};  // # of wares to fetch from flag

	bool builder_idle_{false};          // used to determine whether the builder is idle
	ConstructionsiteInformation info_;  // asked for by player point of view for the gameview

	std::map<DescriptionIndex, uint8_t> additional_wares_;
	std::vector<Worker*> additional_workers_;

	std::unique_ptr<BuildingSettings> settings_;
	void init_settings();
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_CONSTRUCTIONSITE_H
