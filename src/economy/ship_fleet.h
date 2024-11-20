/*
 * Copyright (C) 2011-2024 by the Widelands Development Team
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

#ifndef WL_ECONOMY_SHIP_FLEET_H
#define WL_ECONOMY_SHIP_FLEET_H

#include <memory>

#include "base/macros.h"
#include "economy/shipping_schedule.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/path.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

class Economy;
struct Flag;
class ProductionSite;
struct RoutingNodeNeighbour;
class ShipFleetYardInterface;

class ShipFleetDescr : public MapObjectDescr {
public:
	ShipFleetDescr(char const* const init_name, char const* const init_descname)
	   : MapObjectDescr(MapObjectType::SHIP_FLEET, init_name, init_descname) {
	}
	~ShipFleetDescr() override = default;

private:
	DISALLOW_COPY_AND_ASSIGN(ShipFleetDescr);
};

class ShipFleetYardInterfaceDescr : public BobDescr {
public:
	ShipFleetYardInterfaceDescr(char const* const init_name, char const* const init_descname)
	   : BobDescr(init_name,
	              init_descname,
	              MapObjectType::SHIP_FLEET_YARD_INTERFACE,
	              MapObjectDescr::OwnerType::kTribe) {
	}
	~ShipFleetYardInterfaceDescr() override = default;
	[[nodiscard]] Bob& create_object() const override;

private:
	DISALLOW_COPY_AND_ASSIGN(ShipFleetYardInterfaceDescr);
};

constexpr Duration kFleetInterval(4000);
constexpr uint32_t kRouteNotCalculated = std::numeric_limits<uint32_t>::max();

/**
 * Manage all ships and ports of a player that are connected
 * by ocean.
 *
 * That is, two ports belong to the same fleet if - and only if - ships can
 * travel between them, and so on. Players may have several fleets, if they
 * build ports that cannot communicate (e.g. one port on the ocean, and another
 * in a lake).
 *
 * @paragraph Lifetime
 *
 * Fleet objects are created on-the-fly by @ref Ship and @ref PortDock,
 * and destroy themselves when they become empty.
 *
 * The intention is for fleet objects to merge automatically and separate
 * again in reaction to changes in the map. However, this may not work
 * properly at the moment.
 */
struct ShipFleet : MapObject {
	struct PortPath {
		int32_t cost{-1};
		std::shared_ptr<Path> path;

		PortPath() = default;
	};

	const ShipFleetDescr& descr() const;

	explicit ShipFleet(Player* player);

	PortDock* get_dock(Flag& flag) const;
	PortDock* get_dock(const EditorGameBase&, Coords) const;
	PortDock* get_arbitrary_dock() const;
	void set_economy(Economy* e, WareWorker);

	[[nodiscard]] bool active() const;

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void update(EditorGameBase&);

	void add_ship(EditorGameBase&, Ship* ship);
	void remove_ship(EditorGameBase& egbase, Ship* ship);
	void add_port(EditorGameBase& egbase, PortDock* port);
	void remove_port(EditorGameBase& egbase, PortDock* port);
	[[nodiscard]] bool has_ports() const;

	void add_interface(ShipFleetYardInterface* i);
	void remove_interface(EditorGameBase& egbase, ShipFleetYardInterface* i);

	void log_general_info(const EditorGameBase&) const override;

	bool get_path(const PortDock& start, const PortDock& end, Path& path);
	void add_neighbours(PortDock& pd, std::vector<RoutingNodeNeighbour>& neighbours);

	[[nodiscard]] uint32_t count_ships() const;
	[[nodiscard]] uint32_t count_ports() const;
	[[nodiscard]] bool get_act_pending() const;

	[[nodiscard]] bool empty() const;
	[[nodiscard]] bool lacks_ship() const;

	[[nodiscard]] ShippingSchedule& get_schedule() {
		return schedule_;
	}

	[[nodiscard]] const std::vector<Ship*>& get_ships() const {
		return ships_;
	}
	[[nodiscard]] std::vector<PortDock*>& get_ports() {
		return ports_;
	}

	[[nodiscard]] Quantity get_ships_target() const {
		return ships_target_;
	}
	void set_ships_target(EditorGameBase& egbase, Quantity t);

protected:
	void act(Game&, uint32_t data) override;

private:
	bool find_other_fleet(EditorGameBase& egbase);
	bool merge(EditorGameBase& egbase, ShipFleet* other);
	void check_merge_economy();
	void connect_port(EditorGameBase& egbase, uint32_t idx);

	PortPath& portpath(uint32_t i, uint32_t j);
	const PortPath& portpath(uint32_t i, uint32_t j) const;
	PortPath& portpath_bidir(uint32_t i, uint32_t j, bool& reverse);
	const PortPath& portpath_bidir(uint32_t i, uint32_t j, bool& reverse) const;

	std::vector<Ship*> ships_;
	std::vector<ShipFleetYardInterface*> interfaces_;
	std::vector<PortDock*> ports_;

	Quantity ships_target_{kEconomyTargetInfinity};
	Time target_last_modified_{0U};
	bool act_pending_{false};

	std::map<std::pair<Serial, Serial>, PortPath> port_paths_;

	ShippingSchedule schedule_;

	// saving and loading
protected:
	struct Loader : MapObject::Loader {
		Loader() = default;

		void load(FileRead& fr, uint8_t packet_version);
		void load_pointers() override;
		void load_finish() override;

	private:
		std::vector<uint32_t> ships_;
		std::vector<uint32_t> interfaces_;
		std::vector<uint32_t> ports_;
	};

public:
	bool has_new_save_support() override {
		return true;
	}
	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;

	static MapObject::Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);
};

class ShipFleetYardInterface : public Bob {
public:
	ShipFleetYardInterface();
	static ShipFleetYardInterface*
	create(EditorGameBase& egbase, ProductionSite& ps, const Coords& pos);

	const ShipFleetYardInterfaceDescr& descr() const;
	void init_auto_task(Game& game) override;
	void cleanup(EditorGameBase&) override;
	void log_general_info(const EditorGameBase&) const override;

	[[nodiscard]] ProductionSite* get_building() const {
		return building_;
	}
	[[nodiscard]] ShipFleet* get_fleet() const {
		return fleet_;
	}
	void set_fleet(ShipFleet* fleet) {
		fleet_ = fleet;
	}

	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
	static Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);

private:
	ProductionSite* building_{nullptr};
	ShipFleet* fleet_{nullptr};

protected:
	struct Loader : Bob::Loader {
		Loader() = default;

		void load(FileRead& fr);
		void load_pointers() override;

	private:
		Serial building_;
	};
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_SHIP_FLEET_H
