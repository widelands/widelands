/*
 * Copyright (C) 2010-2026 by the Widelands Development Team
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

#include "logic/map_objects/tribes/ship/ship.h"

#include <memory>

#include "logic/map_objects/tribes/naval_invasion_base.h"
#include "logic/mapregion.h"
#include "logic/player.h"

namespace {

const std::string kPortspaceIconFile = "images/wui/editor/fsel_editor_set_port_space.png";

/// Returns true if 'coords' is not blocked by immovables
/// Trees are allowed, because we don't want spreading forests to block portspaces from expeditions
bool can_support_port(const Widelands::FCoords& coords, Widelands::BaseImmovable::Size max_immo_size) {
	Widelands::BaseImmovable* baim = coords.field->get_immovable();
	Widelands::Immovable* imo = dynamic_cast<Widelands::Immovable*>(baim);
	// we have a player immovable
	if (imo == nullptr && baim != nullptr) {
		return false;
	}
	return (baim == nullptr || baim->get_size() <= max_immo_size ||
	        imo->descr().has_terrain_affinity());
}

}  // namespace

namespace Widelands {

Ship::Expedition::~Expedition() {
	if (ware_economy != nullptr) {
		ware_economy->owner().remove_economy(ware_economy->serial());
	}
	if (worker_economy != nullptr) {
		worker_economy->owner().remove_economy(worker_economy->serial());
	}
}

/// Returns true if the ship can land and erect a port at 'coords'.
bool Ship::can_build_port_here(const Coords& coords) const {
	const PlayerNumber player_number = owner().player_number();
	const EditorGameBase& egbase = owner().egbase();
	const Map& map = egbase.map();
	if (!map.is_port_space(coords)) {
		return false;
	}

	const FCoords fc = map.get_fcoords(coords);

	// First check ownership of the port space
	// All fields of the port + their neighboring fields (for the border) must
	// be conquerable without military influence. Check radius 2 around
	// the main spot to cover radius 1 around each of the 4 fields and the flag
	MapRegion<Area<FCoords>> area(map, Area<FCoords>(fc, 2));
	do {
		const PlayerNumber field_owner = area.location().field->get_owned_by();
		if (field_owner != neutral() && field_owner != player_number) {
			return false;
		}
	} while (area.advance(map));

	// then check whether we can build a port like if reached from land
	// (like Map::is_port_space_allowed(), but checks current buildcaps instead of max buildcaps)
	if ((fc.field->nodecaps() & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG &&
	    !map.find_portdock(fc, false).empty()) {
		return true;
	}

	// now check if we are allowed to build one although some conditions not met
	// All fields of the port and some neighbouring fields must be free of
	// blocking immovables.

	// Immediate neighbours must not have immovables, except for trees, which can
	// spread, but will be cleared by exp_construct_port()
	Widelands::FCoords c[7];
	c[0] = fc;
	map.get_ln(fc, &c[1]);
	map.get_tln(fc, &c[2]);
	map.get_trn(fc, &c[3]);
	map.get_rn(fc, &c[4]);
	map.get_brn(fc, &c[5]);
	map.get_bln(fc, &c[6]);
	for (const Widelands::FCoords& fcc : c) {
		if (!can_support_port(fcc, BaseImmovable::NONE)) {  // check for blocking immovables
			return false;
		}
	}

	// Next neighbours to the North and the West may have size = small immovables
	std::array<Widelands::FCoords, 7> cn;
	map.get_bln(c[1], &cn[0]);  // NOLINT no readability-container-data-pointer here
	map.get_ln(c[1], &cn[1]);
	map.get_tln(c[1], &cn[2]);
	map.get_tln(c[2], &cn[3]);
	map.get_trn(c[2], &cn[4]);
	map.get_trn(c[3], &cn[5]);
	map.get_rn(c[3], &cn[6]);
	return std::all_of(cn.begin(), cn.end(), [](const Widelands::FCoords& fcn) {
		return can_support_port(fcn, BaseImmovable::SMALL);
	});
}

void Ship::recalc_expedition_swimmable(const EditorGameBase& egbase) {
	assert(expedition_ != nullptr);
	for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
		expedition_->swimmable[dir - FIRST_DIRECTION] =
		   ((egbase.map().get_neighbour(get_position(), dir).field->nodecaps() & MOVECAPS_SWIM) != 0);
	}
}

bool Ship::suited_as_invasion_portspace(const Coords& coords) const {
	// We can invade any port space, regardless of owner and immovables.
	// But we ignore port spaces where we already have an own port nearby.

	const EditorGameBase& egbase = owner().egbase();
	const Map& map = egbase.map();
	if (!map.is_port_space(coords)) {
		return false;
	}

	MapRegion<Area<Coords>> mr(map, Area<Coords>(coords, kPortSpaceGeneralAreaRadius));
	do {
		const Field& field = map[mr.location()];
		if (field.get_immovable() != nullptr && field.get_immovable()->get_owner() == get_owner() &&
		    egbase.descriptions().building_index(field.get_immovable()->descr().name()) ==
		       owner().tribe().port()) {
			return false;
		}
	} while (mr.advance(map));

	return true;
}

bool Ship::is_suitable_portspace(const Coords& coords) const {
	if (expedition_ == nullptr) {
		return false;
	}
	const Map& map = owner().egbase().map();
	if (!map.is_port_space(coords)) {
		return false;
	}
	return ship_type_ == ShipType::kWarship ? suited_as_invasion_portspace(coords) :
	                                          can_build_port_here(coords);
}

void Ship::send_known_portspace_message(Game& game) {
	const std::string& icon_filename =
	   ship_type_ == ShipType::kWarship ? descr().icon_filename() : kPortspaceIconFile;
	const std::string message_body = ship_type_ == ShipType::kWarship ?
	                                    _("A warship arrived at a known port build space.") :
	                                    _("An expedition ship arrived at a known port build space.");
	send_message(game, _("Port Space"), _("Port Space Spotted"), message_body, icon_filename);
}
void Ship::send_new_portspace_message(Game& game) {
	const std::string& icon_filename =
	   ship_type_ == ShipType::kWarship ? descr().icon_filename() : kPortspaceIconFile;
	const std::string message_body = ship_type_ == ShipType::kWarship ?
	                                    _("A warship found a new port build space.") :
	                                    _("An expedition ship found a new port build space.");
	send_message(game, _("Port Space"), _("Port Space Found"), message_body, icon_filename);
}

bool Ship::update_seen_portspaces(Game& game, const bool report_known, const bool stop_on_report) {
	if (expedition_ == nullptr) {
		return false;
	}

	bool changed = false;
	bool stopped = false;
	const Map& map = owner().egbase().map();

	// Remove outdated port spaces.
	std::vector<Coords>& seen_portspaces = expedition_->seen_port_buildspaces;
	auto portspace_it = seen_portspaces.begin();
	while (portspace_it != seen_portspaces.end()) {
		if (map.calc_distance(get_position(), *portspace_it) <= descr().vision_range() &&
		    is_suitable_portspace(*portspace_it)) {
			++portspace_it;
		} else {
			portspace_it = seen_portspaces.erase(portspace_it);
			changed = true;
		}
	}

	// Look for new nearby port spaces.
	MapRegion<Area<Coords>> mr(map, Area<Coords>(get_position(), descr().vision_range()));
	do {
		if (!map.is_port_space(mr.location())) {
			continue;
		}
		if (!is_suitable_portspace(mr.location())) {
			if (get_owner()->has_detected_port_space(mr.location()) != nullptr) {
				// Update owner even if can't use currently
				remember_detected_portspace(mr.location());
			}  // TODO(tothxa): But what to do with enemy ports spotted first by expeditions?
			   //               Re: #5889
			continue;
		}
		if (std::find(seen_portspaces.begin(), seen_portspaces.end(), mr.location()) ==
		    seen_portspaces.end()) {
			// Not seen recently
			seen_portspaces.push_back(mr.location());
			changed = true;
			if (remember_detected_portspace(mr.location())) {
				send_new_portspace_message(game);
				stopped = stop_on_report;
			} else if (report_known) {
				send_known_portspace_message(game);
				stopped = stop_on_report;
			}
		}
	} while (mr.advance(map));

	if (stopped) {
		set_ship_state_and_notify(ship_type_ == ShipType::kWarship ?
		                             ShipStates::kExpeditionWaiting :
		                             ShipStates::kExpeditionPortspaceFound,
		                          NoteShip::Action::kWaitingForCommand);
	} else if (changed) {
		// TODO(tothxa): Is this still needed now that InteractivePlayer doesn't cache it?
		Notifications::publish(NoteShip(this, NoteShip::Action::kWaitingForCommand));
	}
	return stopped;
}

bool Ship::remember_detected_portspace(const Coords& coords) {
	EditorGameBase& egbase = get_owner()->egbase();
	const Map& map = egbase.map();
	PlayerNumber space_owner = map[coords].get_owned_by();

	if (DetectedPortSpace* dps = get_owner()->has_detected_port_space(coords); dps != nullptr) {
		dps->owner = space_owner;
		return false;
	}

	std::unique_ptr<DetectedPortSpace> dps(new DetectedPortSpace(dynamic_cast<Game&>(egbase)));
	dps->coords = coords;
	dps->owner = space_owner;
	dps->time_discovered = egbase.get_gametime();
	dps->discovering_ship = get_shipname();
	dps->dockpoints = map.find_portdock(coords, true);

	Coords nearest_dock = Coords::null();
	uint32_t nearest_distance = std::numeric_limits<uint32_t>::max();

	for (const auto& warehouse : owner().get_building_statistics(owner().tribe().port())) {
		if (warehouse.is_constructionsite) {
			continue;
		}
		uint32_t d = map.calc_distance(coords, warehouse.pos);
		if (d < nearest_distance) {
			nearest_distance = d;
			nearest_dock = warehouse.pos;
		}
	}

	// Find the main direction from the nearest own port to the portspace.
	if (nearest_dock.valid()) {
		dps->nearest_portdock =
		   dynamic_cast<const Warehouse&>(*map[nearest_dock].get_immovable()).get_warehouse_name();
		dps->direction_from_portdock =
		   get_compass_dir(nearest_dock, coords, map.get_width(), map.get_height());
		dps->distance_to_portdock = nearest_distance;
	} else {
		dps->direction_from_portdock = CompassDir::kInvalid;
		dps->distance_to_portdock = 0;
	}

	get_owner()->detect_port_space(std::move(dps));
	return true;
}

bool Ship::check_port_space_still_available(Game& game) {
	assert(expedition_);
	const Coords portspace = current_portspace();
	// recheck ownership before setting the csite
	if (!portspace.valid()) {
		log_warn_time(
		   game.get_gametime(), "Expedition list of seen port spaces is unexpectedly empty!\n");
		return false;
	}
	if (!can_build_port_here(portspace)) {
		set_ship_state_and_notify(
		   ShipStates::kExpeditionWaiting, NoteShip::Action::kDestinationChanged);
		send_message(game, _("Port Space Lost!"), _("No Port Can Be Built"),
		             _("A discovered port build space is not available for building a port anymore."),
		             "images/wui/editor/fsel_editor_set_port_space.png");
		expedition_->seen_port_buildspaces.clear();
		return false;
	}
	return true;
}

}  // namespace Widelands
