/*
 * Copyright (C) 2010-2023 by the Widelands Development Team
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

#include "logic/map_objects/tribes/ship.h"

#include <array>
#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/ship_fleet.h"
#include "economy/wares_queue.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/pinned_note.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/requirements.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/mapastar.h"
#include "logic/mapregion.h"
#include "logic/path.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

namespace {

constexpr unsigned kSinkAnimationDuration = 3000;
constexpr unsigned kNearDestinationShipRadius = 4;
constexpr unsigned kNearDestinationNoteRadius = 1;

/// Returns true if 'coord' is not blocked by immovables
/// Trees are allowed, because we don't want spreading forests to block portspaces from expeditions
bool can_support_port(const FCoords& coord, BaseImmovable::Size max_immo_size) {
	BaseImmovable* baim = coord.field->get_immovable();
	Immovable* imo = dynamic_cast<Immovable*>(baim);
	// we have a player immovable
	if (imo == nullptr && baim != nullptr) {
		return false;
	}
	return (baim == nullptr || baim->get_size() <= max_immo_size ||
	        imo->descr().has_terrain_affinity());
}

/// Returns true if a ship owned by 'player_number' can land and erect a port at 'coord'.
bool can_build_port_here(const PlayerNumber player_number, const Map& map, const FCoords& coord) {
	// First check ownership of the port space
	// All fields of the port + their neighboring fields (for the border) must
	// be conquerable without military influence. Check radius 2 around
	// the main spot to cover radius 1 around each of the 4 fields and the flag
	MapRegion<Area<FCoords>> area(map, Area<FCoords>(coord, 2));
	do {
		const PlayerNumber owner = area.location().field->get_owned_by();
		if (owner != neutral() && owner != player_number) {
			return false;
		}
	} while (area.advance(map));

	// then check whether we can build a port like if reached from land
	if ((coord.field->nodecaps() & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG &&
	    !map.find_portdock(coord, false).empty()) {
		return true;
	}

	// now check if we are allowed to build one although some conditions not met
	// All fields of the port and some neighbouring fields must be free of
	// blocking immovables.

	// Immediate neighbours must not have immovables, except for trees, which can
	// spread, but will be cleared by exp_construct_port()
	Widelands::FCoords c[7];
	c[0] = coord;
	map.get_ln(coord, &c[1]);
	map.get_tln(coord, &c[2]);
	map.get_trn(coord, &c[3]);
	map.get_rn(coord, &c[4]);
	map.get_brn(coord, &c[5]);
	map.get_bln(coord, &c[6]);
	for (const Widelands::FCoords& fc : c) {
		if (!can_support_port(fc, BaseImmovable::NONE)) {  // check for blocking immovables
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
	return std::all_of(cn.begin(), cn.end(), [](const Widelands::FCoords& fc) {
		return can_support_port(fc, BaseImmovable::SMALL);
	});
}

}  // namespace

struct FindBobDefender : public FindBob {
	explicit FindBobDefender(const Player& p) : player_(p) {
	}
	bool accept(Bob* b) const override {
		if (b->descr().type() != MapObjectType::SHIP) {
			return false;
		}
		const Ship& s = dynamic_cast<const Ship&>(*b);
		return s.get_ship_type() == ShipType::kWarship &&
		       (s.owner().player_number() == player_.player_number() ||
		        (player_.team_number() > 0 && s.owner().team_number() == player_.team_number())) &&
		       !s.has_battle();
	}

private:
	const Player& player_;
};

struct FindBobEnemyWarship : public FindBob {
	explicit FindBobEnemyWarship(const Widelands::Ship& s) : ship_(s) {
	}
	[[nodiscard]] bool accept(Bob* bob) const override {
		return ship_.is_attackable_enemy_warship(*bob);
	}

private:
	const Widelands::Ship& ship_;
};

/**
 * The contents of 'table' are documented in
 * /data/tribes/ships/atlanteans/init.lua
 */
ShipDescr::ShipDescr(const std::string& init_descname, const LuaTable& table)
   : BobDescr(init_descname, MapObjectType::SHIP, MapObjectDescr::OwnerType::kTribe, table),
     max_hitpoints_(table.get_int("hitpoints")),
     min_attack_(table.get_int("min_attack")),
     max_attack_(table.get_int("max_attack")),
     defense_(table.get_int("defense")),
     attack_accuracy_(table.get_int("attack_accuracy")),
     heal_per_second_(table.get_int("heal_per_second")),
     default_capacity_(table.has_key("capacity") ? table.get_int("capacity") : 20),
     ship_names_(table.get_table("names")->array_entries<std::string>()) {
	// Read the sailing animations
	assign_directional_animation(&sail_anims_, "sail");
}

uint32_t ShipDescr::movecaps() const {
	return MOVECAPS_SWIM;
}

Bob& ShipDescr::create_object() const {
	return *new Ship(*this);
}

Ship::Ship(const ShipDescr& gdescr)
   : Bob(gdescr),
     hitpoints_(gdescr.max_hitpoints_),
     capacity_(gdescr.get_default_capacity()),
     warship_soldier_capacity_(capacity_) {
}

PortDock* Ship::get_lastdock(EditorGameBase& egbase) const {
	return lastdock_.get(egbase);
}

ShipFleet* Ship::get_fleet() const {
	return fleet_;
}

void Ship::init_auto_task(Game& game) {
	start_task_ship(game);
}

bool Ship::init(EditorGameBase& egbase) {
	Bob::init(egbase);
	init_fleet(egbase);
	assert(get_owner());
	get_owner()->add_ship(serial());

	// Assigning a ship name
	shipname_ = get_owner()->pick_shipname();
	molog(egbase.get_gametime(), "New ship: %s\n", shipname_.c_str());
	Notifications::publish(NoteShip(this, NoteShip::Action::kGained));
	return true;
}

void Ship::set_shipname(const std::string& name) {
	shipname_ = name;
	get_owner()->reserve_shipname(name);
}

/**
 * Create the initial singleton @ref ShipFleet to which we belong.
 * The fleet code will automatically merge us into a larger
 * fleet, if one is reachable.
 */
bool Ship::init_fleet(EditorGameBase& egbase) {
	assert(get_owner() != nullptr);
	assert(!fleet_);
	ShipFleet* fleet = new ShipFleet(get_owner());
	fleet->add_ship(egbase, this);
	const bool result = fleet->init(egbase);
	// fleet calls the set_fleet function appropriately
	assert(fleet_);
	fleet_->update(egbase);
	return result;
}

void Ship::cleanup(EditorGameBase& egbase) {
	if (fleet_ != nullptr) {
		fleet_->remove_ship(egbase, this);
	}

	Player* o = get_owner();
	if (o != nullptr) {
		o->remove_ship(serial());
	}

	while (!items_.empty()) {
		items_.back().remove(egbase);
		items_.pop_back();
	}

	Notifications::publish(NoteShip(this, NoteShip::Action::kLost));

	Bob::cleanup(egbase);
}

/**
 * This function is to be called only by @ref ShipFleet.
 */
void Ship::set_fleet(ShipFleet* fleet) {
	fleet_ = fleet;
}

void Ship::wakeup_neighbours(Game& game) {
	FCoords position = get_position();
	Area<FCoords> area(position, 1);
	std::vector<Bob*> ships;
	game.map().find_bobs(game, area, &ships, FindBobShip());

	for (Bob* it : ships) {
		if (it == this) {
			continue;
		}

		dynamic_cast<Ship&>(*it).ship_wakeup(game);
	}
}

/**
 * Standard behaviour of ships.
 *
 * ivar1 = helper flag for coordination of mutual evasion of ships
 */
// TODO(Nordfriese): Having just 1 global task and those numerous ship_update_x
// functions is ugly. Refactor to use a stack of multiple tasks like every
// other bob. But not while I'm still working on the naval warfare please ;)
const Bob::Task Ship::taskShip = {
   "ship", static_cast<Bob::Ptr>(&Ship::ship_update), nullptr, nullptr,
   true  // unique task
};

void Ship::start_task_ship(Game& game) {
	push_task(game, taskShip);
	top_state().ivar1 = 0;
}

void Ship::ship_wakeup(Game& game) {
	if (get_state(taskShip) != nullptr) {
		send_signal(game, "wakeup");
	}
}

void Ship::ship_update(Game& game, Bob::State& state) {
	// Handle signals
	std::string signal = get_signal();
	if (!signal.empty()) {
		if (signal == "wakeup") {
			signal_handled();
		} else if (signal == "cancel_expedition") {
			pop_task(game);
			PortDock* dst = fleet_->get_arbitrary_dock();
			// TODO(sirver): What happens if there is no port anymore?
			if (dst != nullptr) {
				start_task_movetodock(game, *dst);
			}

			signal_handled();
			return;
		} else {
			send_signal(game, "fail");
			pop_task(game);
			return;
		}
	}

	if (send_message_at_destination_) {
		const MapObject* mo = destination_object_.get(game);
		if (mo == nullptr && destination_coords_ == nullptr) {  // Destination vanished.
			send_message_at_destination_ = false;
			send_message(game, _("Destination Gone"), _("Ship Destination Vanished"),
			             _("Your ship’s destination has disappeared."), descr().icon_filename());
			destination_object_ = nullptr;
		} else {
			bool arrived;
			if (destination_coords_ != nullptr) {
				arrived = destination_coords_->has_dockpoint(get_position());
			} else {
				switch (mo->descr().type()) {
				case MapObjectType::PORTDOCK:
					arrived = get_position().field->get_immovable() == mo;
					break;
				case MapObjectType::SHIP:
					arrived = game.map().calc_distance(
					             get_position(), dynamic_cast<const Ship&>(*mo).get_position()) <=
					          kNearDestinationShipRadius;
					break;
				case MapObjectType::PINNED_NOTE:
					arrived = game.map().calc_distance(
					             get_position(), dynamic_cast<const PinnedNote&>(*mo).get_position()) <=
					          kNearDestinationNoteRadius;
					break;
				default:
					NEVER_HERE();
				}
			}
			if (arrived) {
				send_message_at_destination_ = false;
				send_message(game, _("Ship Arrived"), _("Ship Reached Destination"),
				             _("Your ship has arrived at its destination."), descr().icon_filename());
			}
		}
	}

	if (has_battle()) {
		return battle_update(game);
	}

	if (is_refitting()) {
		assert(fleet_ == nullptr);
		if (PortDock* dest = get_destination_port(game); dest != nullptr) {
			const Map& map = game.map();
			FCoords position = map.get_fcoords(get_position());

			if (position.field->get_immovable() != dest) {
				molog(game.get_gametime(), "Move to dock %u for refit\n", dest->serial());
				return start_task_movetodock(game, *dest);
			}

			// Arrived at destination, now unload and refit
			set_destination(game, nullptr);
			Warehouse* wh = dest->get_warehouse();
			for (ShippingItem& si : items_) {
				/* Since the items may not have been in transit properly,
				 * force their reception instead of doing it the normal way.
				 */
				WareInstance* ware;
				Worker* worker;
				si.get(game, &ware, &worker);
				if (worker == nullptr) {
					assert(ware != nullptr);
					wh->receive_ware(game, game.descriptions().safe_ware_index(ware->descr().name()));
					ware->remove(game);
				} else {
					assert(ware == nullptr);
					worker->set_economy(nullptr, wwWARE);
					worker->set_economy(nullptr, wwWORKER);
					worker->set_position(game, wh->get_position());
					wh->incorporate_worker(game, worker);
				}
			}
			items_.clear();

			ship_type_ = pending_refit_;
			warship_soldier_request_.reset();

			if (ship_type_ == ShipType::kWarship) {
				start_task_expedition(game);
				set_destination(game, dest);
			} else {
				exp_cancel(game);
			}
		} else {
			// Destination vanished, try to find a new one
			molog(game.get_gametime(), "Refit failed, retry\n");
			const ShipType t = pending_refit_;
			pending_refit_ = ship_type_;
			refit(game, t);
		}
		return;
	}

	switch (ship_state_) {
	case ShipStates::kTransport:
		if (ship_update_transport(game, state)) {
			return;
		}
		break;
	case ShipStates::kExpeditionPortspaceFound:
	case ShipStates::kExpeditionScouting:
	case ShipStates::kExpeditionWaiting:
		if (ship_update_expedition(game, state)) {
			return;
		}
		break;
	case ShipStates::kExpeditionColonizing:
		break;
	case ShipStates::kSinkRequest:
		if (descr().is_animation_known("sinking")) {
			ship_state_ = ShipStates::kSinkAnimation;
			start_task_idle(game, descr().get_animation("sinking", this), kSinkAnimationDuration);
			return;
		}
		log_warn_time(game.get_gametime(), "Oh no... this ship has no sinking animation :(!\n");
		FALLS_THROUGH;
	case ShipStates::kSinkAnimation:
		// The sink animation has been played, so finally remove the ship from the map
		pop_task(game);
		schedule_destroy(game);
		return;
	}
	// if the real update function failed (e.g. nothing to transport), the ship goes idle
	ship_update_idle(game, state);
}

/// updates a ships tasks in transport mode \returns false if failed to update tasks
bool Ship::ship_update_transport(Game& game, Bob::State& state) {
	const Map& map = game.map();

	MapObject* destination_object = destination_object_.get(game);
	assert(destination_object == nullptr ||
	       destination_object->descr().type() == MapObjectType::PORTDOCK);
	PortDock* destination = dynamic_cast<PortDock*>(destination_object);

	if (destination == nullptr) {
		// The ship has no destination, so let it sleep
		ship_update_idle(game, state);
		return true;
	}

	FCoords position = map.get_fcoords(get_position());
	if (position.field->get_immovable() == destination) {
		if (lastdock_ != destination) {
			molog(game.get_gametime(), "ship_update: Arrived at dock %u\n", destination->serial());
			lastdock_ = destination;
		}
		while (withdraw_item(game, *destination)) {
		}

		destination->ship_arrived(game, *this);  // This will also set the destination

		if (destination != nullptr) {
			start_task_movetodock(game, *destination);
		} else {
			start_task_idle(game, descr().main_animation(), 250);
		}
		return true;
	}

	molog(game.get_gametime(), "ship_update: Go to dock %u\n", destination->serial());

	PortDock* lastdock = lastdock_.get(game);
	if (lastdock != nullptr && lastdock != destination) {
		molog(game.get_gametime(), "ship_update: Have lastdock %u\n", lastdock->serial());

		Path path;
		if (fleet_->get_path(*lastdock, *destination, path)) {
			uint32_t closest_idx = std::numeric_limits<uint32_t>::max();
			uint32_t closest_dist = std::numeric_limits<uint32_t>::max();
			Coords closest_target(Coords::null());

			Coords cur(path.get_start());
			for (uint32_t idx = 0; idx <= path.get_nsteps(); ++idx) {
				uint32_t dist = map.calc_distance(get_position(), cur);

				if (dist == 0) {
					molog(game.get_gametime(), "Follow pre-computed path from (%i,%i)  [idx = %u]\n",
					      cur.x, cur.y, idx);

					Path subpath(cur);
					while (idx < path.get_nsteps()) {
						subpath.append(map, path[idx]);
						idx++;
					}

					start_task_movepath(game, subpath, descr().get_sail_anims());
					return true;
				}

				if (dist < closest_dist) {
					closest_dist = dist;
					closest_idx = idx;
				}

				if (idx == closest_idx + closest_dist) {
					closest_target = cur;
				}

				if (idx < path.get_nsteps()) {
					map.get_neighbour(cur, path[idx], &cur);
				}
			}

			if (closest_target) {
				molog(game.get_gametime(), "Closest target en route is (%i,%i)\n", closest_target.x,
				      closest_target.y);
				if (start_task_movepath(game, closest_target, 0, descr().get_sail_anims())) {
					return true;
				}

				molog(game.get_gametime(), "  Failed to find path!!! Retry full search\n");
			}
		}

		lastdock_ = nullptr;
	}

	start_task_movetodock(game, *destination);
	return true;
}

/// updates a ships tasks in expedition mode; returns whether tasks were updated
bool Ship::ship_update_expedition(Game& game, Bob::State& /* state */) {
	Map* map = game.mutable_map();

	assert(expedition_);
	const FCoords position = get_position();

	// Update the knowledge of the surrounding fields
	for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
		expedition_->swimmable[dir - 1] =
		   ((map->get_neighbour(position, dir).field->nodecaps() & MOVECAPS_SWIM) != 0);
	}

	if (get_ship_type() == ShipType::kWarship) {
		// Look for nearby enemy warships.
		Area<FCoords> area(get_position(), descr().vision_range());
		bool found_new_target = false;
		std::vector<Bob*> candidates;
		map->find_reachable_bobs(
		   game, area, &candidates, CheckStepDefault(MOVECAPS_SWIM), FindBobEnemyWarship(*this));

		// Clear outdated attack targets.
		std::set<OPtr<Ship>>& attack_targets = expedition_->attack_targets;
		for (auto it = attack_targets.begin(); it != attack_targets.end();) {
			if (std::find_if(candidates.begin(), candidates.end(), [&it](Bob* b) {
				    return b->serial() == it->serial();
			    }) != candidates.end()) {
				++it;
			} else {
				it = attack_targets.erase(it);
			}
		}

		// Add new attack targets.
		for (Bob* enemy : candidates) {
			if (attack_targets.insert(OPtr<Ship>(dynamic_cast<Ship*>(enemy))).second) {
				found_new_target = true;
				send_message(game, _("Enemy Ship"), _("Enemy Ship Spotted"),
				             _("A warship spotted an enemy ship."), enemy->descr().icon_filename());
			}
		}

		// Remove outdated port spaces.
		std::vector<Coords>& portspaces = expedition_->seen_port_buildspaces;
		for (size_t i = 0; i < portspaces.size();) {
			if (map->calc_distance(get_position(), portspaces.at(i)) <= area.radius) {
				++i;
			} else {
				portspaces.at(i) = portspaces.back();
				portspaces.pop_back();
			}
		}

		// Look for new nearby port spaces.
		MapRegion<Area<Coords>> mr(*map, Area<Coords>(position, descr().vision_range()));
		do {
			if (map->is_port_space(mr.location()) &&
			    std::find(portspaces.begin(), portspaces.end(), mr.location()) == portspaces.end()) {
				found_new_target = true;
				portspaces.push_back(mr.location());
				remember_detected_portspace(mr.location());
				send_message(game, _("Port Space"), _("Port Space Found"),
				             _("A warship found a new port build space."), descr().icon_filename());
			}
		} while (mr.advance(*map));

		if (found_new_target) {
			set_ship_state_and_notify(
			   ShipStates::kExpeditionWaiting, NoteShip::Action::kWaitingForCommand);
		}
	}

	if (destination_coords_ != nullptr) {
		if (destination_coords_->has_dockpoint(get_position())) {  // Already there
			start_task_idle(game, descr().main_animation(), 250);
			return true;
		}

		if (start_task_movepath(
		       game, destination_coords_->dockpoints.front(), 0, descr().get_sail_anims())) {
			return true;
		}

		molog(game.get_gametime(), "Could not find path to destination at %dx%d",
		      destination_coords_->dockpoints.front().x, destination_coords_->dockpoints.front().y);
		if (send_message_at_destination_) {
			send_message(game, _("Destination Unreachable"), _("Ship Destination Unreachable"),
			             _("Your ship could not find a path to its destination."),
			             descr().icon_filename());
			send_message_at_destination_ = false;
		}
		destination_coords_ = nullptr;

	} else if (MapObject* destination_object = destination_object_.get(game);
	           destination_object != nullptr) {
		switch (destination_object->descr().type()) {
		case MapObjectType::PORTDOCK: {
			PortDock* dest = dynamic_cast<PortDock*>(destination_object);

			// Sail to the destination port if we're not there yet.
			if (position.field->get_immovable() != dest) {
				start_task_movetodock(game, *dest);
				return true;
			}

			// We're on the destination dock. Load soldiers, heal, and wait for orders.
			constexpr Duration kHealInterval(1000);
			if (hitpoints_ < descr().max_hitpoints_ &&
			    game.get_gametime() - last_heal_time_ >= kHealInterval) {
				last_heal_time_ = game.get_gametime();
				hitpoints_ = std::min(descr().max_hitpoints_, hitpoints_ + descr().heal_per_second_);
			}

			set_ship_state_and_notify(
			   ShipStates::kExpeditionWaiting, NoteShip::Action::kWaitingForCommand);

			if (warship_soldier_request_ == nullptr) {
				warship_soldier_request_.reset(new Request(*dest->get_warehouse(),
				                                           owner().tribe().soldier(),
				                                           Ship::warship_soldier_callback, wwWORKER));
			}
			update_warship_soldier_request(game);

			start_task_idle(game, descr().main_animation(), 250);

			return true;
		}

		case MapObjectType::SHIP:
		case MapObjectType::PINNED_NOTE: {
			Bob* dest = dynamic_cast<Bob*>(destination_object);

			if (map->calc_distance(position, dest->get_position()) <=
			    (dest->descr().type() == MapObjectType::SHIP ? kNearDestinationShipRadius :
                                                            kNearDestinationNoteRadius)) {
				// Already there, idle and await further orders.
				start_task_idle(game, descr().main_animation(), 250);
				return true;
			}

			// Sail to the destination ship/note if we're not there yet.
			Path path;
			if (map->findpath(
			       position, dest->get_position(), 0, path, CheckStepDefault(MOVECAPS_SWIM)) < 0) {
				molog(game.get_gametime(), "No path to destination ship/note found!");
				set_destination(game, nullptr);
				break;
			}

			// Ships tend to move around, so we need to recompute the path after every few steps.
			constexpr unsigned kMaxSteps = 4;
			if (path.get_nsteps() <= kMaxSteps) {
				start_task_movepath(game, path, descr().get_sail_anims());
				return true;
			}

			Path truncated(path.get_start());
			for (unsigned i = 0; i < kMaxSteps; ++i) {
				truncated.append(*map, path[i]);
			}
			start_task_movepath(game, truncated, descr().get_sail_anims());

			return true;
		}

		default:
			NEVER_HERE();
		}
	}

	warship_soldier_request_.reset();  // Clear the request when not in port

	if (ship_state_ == ShipStates::kExpeditionScouting && get_ship_type() == ShipType::kTransport) {
		// Check surrounding fields for port buildspaces
		std::vector<Coords> temp_port_buildspaces;
		MapRegion<Area<Coords>> mr(*map, Area<Coords>(position, descr().vision_range()));
		bool new_port_space = false;
		do {
			if (!map->is_port_space(mr.location())) {
				continue;
			}

			const FCoords fc = map->get_fcoords(mr.location());

			// Check whether the maximum theoretical possible NodeCap of the field
			// is of the size big and whether it can theoretically be a port space
			if ((map->get_max_nodecaps(game, fc) & BUILDCAPS_SIZEMASK) != BUILDCAPS_BIG ||
			    map->find_portdock(fc, true).empty()) {
				continue;
			}

			if (!can_build_port_here(get_owner()->player_number(), *map, fc)) {
				continue;
			}

			// Check if the ship knows this port space already from its last check
			remember_detected_portspace(mr.location());
			if (std::find(expedition_->seen_port_buildspaces.begin(),
			              expedition_->seen_port_buildspaces.end(),
			              mr.location()) != expedition_->seen_port_buildspaces.end()) {
				temp_port_buildspaces.push_back(mr.location());
			} else {
				new_port_space = true;
				temp_port_buildspaces.insert(temp_port_buildspaces.begin(), mr.location());
			}
		} while (mr.advance(*map));

		expedition_->seen_port_buildspaces = temp_port_buildspaces;
		if (new_port_space) {
			set_ship_state_and_notify(
			   ShipStates::kExpeditionPortspaceFound, NoteShip::Action::kWaitingForCommand);
			send_message(game, _("Port Space"), _("Port Space Found"),
			             _("An expedition ship found a new port build space."),
			             "images/wui/editor/fsel_editor_set_port_space.png");
		}
	} else if (ship_state_ == ShipStates::kExpeditionPortspaceFound) {
		check_port_space_still_available(game);
	}

	return false;  // Continue with the regular expedition updates
}

void Ship::remember_detected_portspace(const Coords& coords) {
	const EditorGameBase& egbase = owner().egbase();
	const Map& map = egbase.map();
	PlayerNumber space_owner = map[coords].get_owned_by();

	if (DetectedPortSpace* dps = get_owner()->has_detected_port_space(coords); dps != nullptr) {
		dps->owner = space_owner;
		return;
	}

	std::unique_ptr<DetectedPortSpace> dps(new DetectedPortSpace());
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
	dps->direction_from_portdock = CompassDir::kInvalid;
	if (static_cast<bool>(nearest_dock)) {
		dps->nearest_portdock =
		   dynamic_cast<const Warehouse&>(*map[nearest_dock].get_immovable()).get_warehouse_name();
		dps->direction_from_portdock =
		   get_compass_dir(nearest_dock, coords, map.get_width(), map.get_height());
	}

	get_owner()->detect_port_space(std::move(dps));
}

void Ship::update_warship_soldier_request(Game& /* game */) {
	if (warship_soldier_request_ == nullptr) {
		return;
	}
	warship_soldier_request_->set_count(get_nritems() > get_warship_soldier_capacity() ?
                                          0 :
                                          get_warship_soldier_capacity() - get_nritems());
}

// static
void Ship::warship_soldier_callback(Game& game,
                                    Request& req,
                                    DescriptionIndex /* di */,
                                    Worker* worker,
                                    PlayerImmovable& immovable) {
	Warehouse& warehouse = dynamic_cast<Warehouse&>(immovable);
	PortDock* dock = warehouse.get_portdock();
	assert(dock != nullptr);

	for (const Coords& c : dock->get_positions(game)) {
		for (Bob* b = game.map()[c].get_first_bob(); b != nullptr; b = b->get_next_bob()) {
			if (b->descr().type() == MapObjectType::SHIP) {
				upcast(Ship, ship, b);
				if (ship->warship_soldier_request_.get() == &req) {
					assert(ship->get_owner() == dock->get_owner());
					assert(ship->get_ship_type() == ShipType::kWarship);

					ship->molog(game.get_gametime(), "%s %u embarked on warship",
					            worker->descr().name().c_str(), worker->serial());

					worker->set_location(nullptr);
					worker->start_task_shipping(game, nullptr);

					ship->add_item(game, ShippingItem(*worker));
					ship->update_warship_soldier_request(game);

					return;
				}
			}
		}
	}

	verb_log_info_time(game.get_gametime(), "%s %u missed his assigned warship at dock %s",
	                   worker->descr().name().c_str(), worker->serial(),
	                   warehouse.get_warehouse_name().c_str());
	// The soldier is in the port now, ready to get some new assignment by the economy.
}

bool Ship::is_attackable_enemy_warship(const Bob& b) const {
	if (b.descr().type() != MapObjectType::SHIP) {
		return false;
	}
	const Ship& s = dynamic_cast<const Ship&>(b);
	return s.can_be_attacked() && owner().is_hostile(s.owner());
}

bool Ship::can_be_attacked() const {
	// Only warships can be attacked, but not if they're already engaged in battle
	// against any other ship. Ships attacking a port CAN be attacked though.
	return get_ship_type() == ShipType::kWarship &&
	       std::all_of(battles_.begin(), battles_.end(), [this](const Battle& battle) {
		       const MapObject* opponent = battle.opponent.get(owner().egbase());
		       return opponent == nullptr || opponent->descr().type() == MapObjectType::PORTDOCK;
	       });
}

bool Ship::can_refit(const ShipType type) const {
	return !is_refitting() && !has_battle() && type != ship_type_;
}

#ifndef NDEBUG
void Ship::set_ship_type(EditorGameBase& egbase, ShipType t) {
	assert(!egbase.is_game());
	ship_type_ = t;
	pending_refit_ = ship_type_;
}
#else
void Ship::set_ship_type(EditorGameBase& /* egbase */, ShipType t) {
	ship_type_ = t;
	pending_refit_ = ship_type_;
}
#endif

void Ship::refit(Game& game, const ShipType type) {
	if (!can_refit(type)) {
		molog(game.get_gametime(), "Requested refit to %d not possible", static_cast<int>(type));
		return;
	}

	if (get_destination_port(game) != nullptr) {
		send_signal(game, "wakeup");
	} else if (PortDock* dest = find_nearest_port(game); dest != nullptr) {
		set_destination(game, dest);
	} else {
		molog(game.get_gametime(), "Attempted refit to %d but no ports in fleet",
		      static_cast<int>(type));
		return;
	}

	pending_refit_ = type;

	// Already remove the ship from the fleet
	if (fleet_ != nullptr) {
		fleet_->remove_ship(game, this);
		assert(fleet_ == nullptr);
	}
}

PortDock* Ship::find_nearest_port(Game& game) {
	const bool in_fleet = fleet_ != nullptr;
	Economy* eco_ware = ware_economy_;
	Economy* eco_worker = worker_economy_;

	if (!in_fleet) {
		init_fleet(game);
		assert(fleet_ != nullptr);
	}

	PortDock* nearest = nullptr;
	int32_t dist = 0;
	for (PortDock* pd : fleet_->get_ports()) {
		Path path;
		int32_t d = -1;
		calculate_sea_route(game, *pd, &path);
		game.map().calc_cost(path, &d, nullptr);
		assert(d >= 0);
		if (nearest == nullptr || d < dist) {
			dist = d;
			nearest = pd;
		}
	}

	if (!in_fleet) {
		fleet_->remove_ship(game, this);
		assert(fleet_ == nullptr);
		set_economy(game, eco_ware, wwWARE);
		set_economy(game, eco_worker, wwWORKER);
	}

	return nearest;
}

bool Ship::is_on_destination_dock() const {
	const MapObject* dest = destination_object_.get(owner().egbase());
	return dest != nullptr && dest->descr().type() == MapObjectType::PORTDOCK &&
	       get_position().field->get_immovable() == dest;
}

uint32_t Ship::min_warship_soldier_capacity() const {
	return is_on_destination_dock() ? 0U : get_nritems();
}

std::vector<Soldier*> Ship::onboard_soldiers() const {
	std::vector<Soldier*> result;
	for (const ShippingItem& si : items_) {
		Worker* worker;
		si.get(owner().egbase(), nullptr, &worker);
		if (worker != nullptr && worker->descr().type() == MapObjectType::SOLDIER) {
			result.push_back(dynamic_cast<Soldier*>(worker));
		}
	}
	return result;
}

void Ship::drop_soldier(Game& game, Serial soldier) {
	PortDock* dest = get_destination_port(game);
	if (dest == nullptr) {
		verb_log_warn_time(game.get_gametime(), "Ship not in dock, cannot drop soldier");
		return;
	}

	for (size_t i = 0; i < items_.size(); ++i) {
		Worker* worker;
		items_[i].get(game, nullptr, &worker);
		if (worker != nullptr && worker->serial() == soldier) {
			dest->shipping_item_arrived(game, items_[i]);

			items_[i] = items_.back();
			items_.pop_back();
			return;
		}
	}
	verb_log_warn_time(game.get_gametime(), "Ship::drop_soldier: %u is not on board", soldier);
}

void Ship::warship_command(Game& game,
                           const WarshipCommand cmd,
                           const std::vector<uint32_t>& parameters) {
	if (get_ship_type() != ShipType::kWarship) {
		return;
	}

	switch (cmd) {
	case WarshipCommand::kSetCapacity: {
		assert(parameters.size() == 1);
		warship_soldier_capacity_ =
		   std::max(std::min(parameters.back(), get_capacity()), min_warship_soldier_capacity());
		update_warship_soldier_request(game);

		// If we have too many soldiers on board now, unload the extras.
		PortDock* dest = get_destination_port(game);
		while (get_nritems() > warship_soldier_capacity_) {
			assert(dest != nullptr);
			dest->shipping_item_arrived(game, items_.back());
			items_.pop_back();
		}

		return;
	}

	case WarshipCommand::kAttack:
		assert(!parameters.empty());
		if (parameters.size() == 1) {  // Attacking a ship.
			if (Ship* target = dynamic_cast<Ship*>(game.objects().get_object(parameters.front()));
			    target != nullptr) {
				start_battle(game, Battle(target, Coords::null(), {}, true));
			}
		} else {  // Attacking port coordinates.
			assert(parameters.size() > 2);
			Coords portspace(parameters.at(0), parameters.at(1));
			assert(game.map().is_port_space(portspace));

			std::vector<Coords> dockpoints = game.map().find_portdock(portspace, true);
			assert(!dockpoints.empty());

			start_battle(
			   game, Battle(nullptr, dockpoints.at(game.logic_rand() % dockpoints.size()),
			                std::vector<uint32_t>(parameters.begin() + 2, parameters.end()), true));
		}
		return;
	}

	throw wexception("Invalid warship command %d", static_cast<int>(cmd));
}

void Ship::start_battle(Game& game, const Battle new_battle) {
	Ship* enemy_ship = new_battle.opponent.get(game);

	if (enemy_ship == nullptr && !static_cast<bool>(new_battle.attack_coords)) {
		return;
	}

	battles_.emplace_back(new_battle);
	if (!new_battle.is_first) {
		return;
	}

	// Summon someone to the defence
	if (enemy_ship != nullptr) {
		enemy_ship->send_message(game, _("Naval Attack"), _("Enemy Ship Attacking"),
		                         format(_("Your ship ‘%s’ is under attack from an enemy warship."),
		                                enemy_ship->get_shipname()),
		                         "images/wui/ship/ship_attack.png");
		enemy_ship->start_battle(game, Battle(this, Coords::null(), {}, false));
	}
}

/** Onboard soldiers add a bonus onto the base attack strength, expressed in percent. */
unsigned Ship::get_sea_attack_soldier_bonus(const EditorGameBase& egbase) const {
	unsigned attack_bonus = 0;
	for (const ShippingItem& si : items_) {
		Worker* worker;
		si.get(egbase, nullptr, &worker);
		if (worker != nullptr && worker->descr().type() == MapObjectType::SOLDIER) {
			attack_bonus += dynamic_cast<Soldier*>(worker)->get_total_level() + 1;
		}
	}
	return attack_bonus;
}

constexpr uint8_t kPortUnderAttackDefendersSearchRadius = 10;
constexpr uint32_t kAttackAnimationDuration = 2000;

void Ship::battle_update(Game& game) {
	Battle& current_battle = battles_.back();
	Ship* target_ship = current_battle.opponent.get(game);
	if (target_ship == nullptr && !static_cast<bool>(current_battle.attack_coords)) {
		molog(game.get_gametime(), "[battle] Enemy disappeared, cancel");
		battles_.pop_back();
		start_task_idle(game, descr().main_animation(), 100);
		return;
	}

	assert((target_ship != nullptr) ^ static_cast<bool>(current_battle.attack_coords));
	assert(target_ship != nullptr || current_battle.is_first);
	const Map& map = game.map();

	Battle* other_battle = (target_ship == nullptr) ? nullptr : &target_ship->battles_.back();
	assert(other_battle == nullptr || (other_battle->opponent.get(game) == this &&
	                                   other_battle->is_first != current_battle.is_first &&
	                                   other_battle->phase == current_battle.phase));

	auto set_phase = [&game, &current_battle, other_battle](Battle::Phase new_phase) {
		current_battle.phase = new_phase;
		current_battle.time_of_last_action = game.get_gametime();
		if (other_battle != nullptr) {
			other_battle->phase = new_phase;
			other_battle->time_of_last_action = current_battle.time_of_last_action;
		}
	};
	auto fight = [this, &current_battle, other_battle, &game, target_ship]() {
		if (target_ship == nullptr) {
			molog(game.get_gametime(), "[battle] Attacking a port");
			current_battle.pending_damage = 1;                             // Ports always take 1 point
		} else if (game.logic_rand() % 100 < descr().attack_accuracy_) {  // Hit
			uint32_t attack_strength =
			   (game.logic_rand() % (descr().max_attack_ - descr().min_attack_));
			attack_strength += descr().min_attack_;

			attack_strength += attack_strength * get_sea_attack_soldier_bonus(game) / 100;

			molog(game.get_gametime(), "[battle] Hit with %u points", attack_strength);
			current_battle.pending_damage =
			   attack_strength * (100 - target_ship->descr().defense_) / 100;
		} else {  // Miss
			molog(game.get_gametime(), "[battle] Miss");
			current_battle.pending_damage = 0;
		}

		if (other_battle != nullptr) {
			other_battle->pending_damage = current_battle.pending_damage;
		}
	};
	auto damage = [this, &game, set_phase, &current_battle, other_battle,
	               target_ship](Battle::Phase next) {
		assert(target_ship != nullptr);
		if (target_ship->hitpoints_ > current_battle.pending_damage) {
			molog(game.get_gametime(), "[battle] Subtracting %u hitpoints from enemy",
			      current_battle.pending_damage);
			target_ship->hitpoints_ -= current_battle.pending_damage;
			set_phase(next);
		} else {
			molog(game.get_gametime(), "[battle] Enemy defeated");

			get_owner()->count_naval_victory();
			target_ship->get_owner()->count_naval_loss();
			target_ship->send_message(game, _("Ship Sunk"), _("Ship Destroyed"),
			                          _("An enemy ship has destroyed your warship."),
			                          "images/wui/ship/ship_attack.png");

			target_ship->battles_.clear();
			target_ship->reset_tasks(game);
			target_ship->set_ship_state_and_notify(
			   ShipStates::kSinkRequest, NoteShip::Action::kDestinationChanged);

			battles_.pop_back();
			return true;
		}

		current_battle.pending_damage = 0;
		other_battle->pending_damage = 0;
		return false;
	};

	if (!current_battle.is_first) {
		switch (current_battle.phase) {
		case Battle::Phase::kDefenderAttacking: {
			// Our turn is over, now it's the enemy's turn.
			molog(game.get_gametime(), "[battle] Defender's turn ends");
			bool won = damage(Battle::Phase::kAttackersTurn);
			// Make sure we will idle until the enemy ship is truly gone, so we won't attack again
			start_task_idle(
			   game, descr().main_animation(), won ? (kSinkAnimationDuration + 1000) : 100);
			return;
		}

		case Battle::Phase::kDefendersTurn:
			molog(game.get_gametime(), "[battle] Defender's turn begins");
			fight();
			set_phase(Battle::Phase::kDefenderAttacking);
			start_task_idle(game, descr().main_animation(),
			                kAttackAnimationDuration);  // TODO(Nordfriese): proper animation
			return;

		default:
			// Idle until the enemy tells us it's our turn now.
			return start_task_idle(game, descr().main_animation(), 100);
		}
		NEVER_HERE();
	}

	switch (current_battle.phase) {
	case Battle::Phase::kDefendersTurn:
	case Battle::Phase::kDefenderAttacking:
		// Idle until the opponent's turn is over.
		return start_task_idle(game, descr().main_animation(), 100);

	case Battle::Phase::kNotYetStarted:
		molog(game.get_gametime(), "[battle] Preparing to engage");
		set_phase(Battle::Phase::kAttackerMovingTowardsOpponent);
		FALLS_THROUGH;
	case Battle::Phase::kAttackerMovingTowardsOpponent: {
		// Move towards the opponent.
		Coords dest;
		bool exact_match_required;
		if (target_ship != nullptr) {
			exact_match_required = false;
			dest = target_ship->get_position();
		} else {
			exact_match_required = true;
			dest = current_battle.attack_coords;
		}
		if (dest == get_position() ||
		    (!exact_match_required && map.calc_distance(get_position(), dest) < 2)) {
			// Already there, start the fight in the next act.
			// For ports, skip the first round to allow defense warships to approach.
			molog(game.get_gametime(), "[battle] Enemy in range");
			set_phase(target_ship != nullptr ? Battle::Phase::kAttackersTurn :
                                            Battle::Phase::kAttackerAttacking);
			return start_task_idle(game, descr().main_animation(), 100);
		}

		Path path;
		if (map.findpath(get_position(), dest, 0, path, CheckStepDefault(MOVECAPS_SWIM)) < 0) {
			if (target_ship != nullptr) {
				molog(game.get_gametime(),
				      "Could not find a path to opponent ship %u %s from %dx%d to %dx%d",
				      target_ship->serial(), target_ship->get_shipname().c_str(), get_position().x,
				      get_position().y, dest.x, dest.y);
			} else {
				molog(game.get_gametime(), "Could not find a path to attack coords from %dx%d to %dx%d",
				      get_position().x, get_position().y, dest.x, dest.y);
			}

			battles_.pop_back();
			return start_task_idle(game, descr().main_animation(), 100);
		}

		int steps = path.get_nsteps();
		if (!exact_match_required) {
			assert(steps > 1);
			steps--;
		}

		molog(game.get_gametime(), "[battle] Moving towards enemy");
		start_task_movepath(game, path, descr().get_sail_anims(), false, steps);
		return;
	}

	case Battle::Phase::kAttackerAttacking:
		if (target_ship != nullptr) {
			// Our turn is over, now it's the enemy's turn.
			molog(game.get_gametime(), "[battle] Attacker's turn ends");
			bool won = damage(Battle::Phase::kDefendersTurn);
			if (won) {
				return start_task_idle(game, descr().main_animation(), kSinkAnimationDuration + 1000);
			}
		} else if (current_battle.pending_damage > 0) {
			// The naval assault was successful. Now unload the soldiers.
			// From the ship's perspective, the attack was a success.
			molog(game.get_gametime(), "[battle] Naval invasion commencing");

			Coords portspace = map.find_portspace_for_dockpoint(current_battle.attack_coords);
			assert(portspace != Coords::null());
			Field& portspace_field = map[portspace];

			const PlayerNumber enemy_pn = portspace_field.get_owned_by();
			if (enemy_pn != 0) {
				game.get_player(enemy_pn)->add_message_with_timeout(
				   game,
				   std::unique_ptr<Message>(new Message(
				      Message::Type::kSeafaring, game.get_gametime(), _("Naval Attack"),
				      "images/wui/ship/ship_attack.png", _("Enemy Ship Attacking"),
				      _("Your coast is under attack from an enemy warship."), get_position())),
				   Duration(60 * 1000) /* throttle timeout in milliseconds */, 6 /* throttle radius */);
			}

			// If the portspace is blocked, find a walkable node as closely nearby as possible.
			Coords representative_location;
			if ((portspace_field.nodecaps() & MOVECAPS_WALK) != 0) {
				representative_location = portspace;
			} else if (Coords brn = map.br_n(portspace); (map[brn].nodecaps() & MOVECAPS_WALK) != 0) {
				representative_location = brn;
			} else {
				for (;;) {
					Coords coords = game.random_location(portspace, 2);
					const Field& field = map[coords];
					if ((field.nodecaps() & MOVECAPS_WALK) != 0) {
						representative_location = coords;
						break;
					}
				}
			}

			CheckStepDefault worker_checkstep(MOVECAPS_WALK);
			Path unused_path;

			assert(!battles_.back().attack_soldier_serials.empty());
			for (Serial serial : battles_.back().attack_soldier_serials) {
				auto it = std::find_if(items_.begin(), items_.end(), [serial](const ShippingItem& si) {
					return si.get_object_serial() == serial;
				});
				if (it == items_.end()) {
					continue;
				}

				Worker* worker;
				it->get(game, nullptr, &worker);
				if (worker == nullptr || worker->descr().type() != MapObjectType::SOLDIER) {
					continue;
				}

				it->set_location(game, nullptr);
				it->end_shipping(game);

				// Distribute the soldiers on walkable fields around the point of invasion.
				// Do not drop them off directly on a flag as that would interfere with battle code.
				for (;;) {
					Coords coords = game.random_location(current_battle.attack_coords, 4);
					const Field& field = map[coords];
					if ((field.nodecaps() & MOVECAPS_WALK) != 0U &&
					    (field.get_immovable() == nullptr ||
					     field.get_immovable()->descr().type() != MapObjectType::FLAG) &&
					    map.findpath(
					       coords, representative_location, 3, unused_path, worker_checkstep) >= 0) {
						worker->set_position(game, coords);
						break;
					}
				}

				worker->reset_tasks(game);
				dynamic_cast<Soldier&>(*worker).start_task_naval_invasion(game, portspace);

				items_.erase(it);
			}

			battles_.pop_back();
		} else {
			// Summon someone to the port's defense, if possible.
			std::vector<Bob*> defenders;
			const PlayerNumber enemy_pn = map[current_battle.attack_coords].get_owned_by();
			Player* enemy = enemy_pn == 0 ? nullptr : game.get_player(enemy_pn);
			if (enemy != nullptr && enemy->is_hostile(owner())) {
				map.find_reachable_bobs(
				   game, Area<FCoords>(get_position(), kPortUnderAttackDefendersSearchRadius),
				   &defenders, CheckStepDefault(MOVECAPS_SWIM), FindBobDefender(*enemy));
			}

			Bob* nearest = nullptr;
			uint32_t distance = 0;
			for (Bob* candidate : defenders) {
				const uint32_t d = map.calc_distance(candidate->get_position(), get_position());
				if (nearest == nullptr || d < distance) {
					nearest = candidate;
					distance = d;
				}
			}

			if (nearest != nullptr) {
				// Let the best candidate launch an attack against us. This
				// suspends the current battle until the new fight is over.
				Ship& nearest_ship = dynamic_cast<Ship&>(*nearest);
				molog(game.get_gametime(), "[battle] Summoning %s to the port's defense",
				      nearest_ship.get_shipname().c_str());
				nearest_ship.start_battle(game, Battle(this, Coords::null(), {}, true));
			}

			// Since ports can't defend themselves on their own, start the next round at once.
			molog(game.get_gametime(), "[battle] Port is undefended");
			set_phase(Battle::Phase::kAttackersTurn);
		}
		start_task_idle(game, descr().main_animation(), 100);
		return;

	case Battle::Phase::kAttackersTurn:
		molog(game.get_gametime(), "[battle] Attacker's turn begins");
		fight();
		set_phase(Battle::Phase::kAttackerAttacking);
		start_task_idle(game, descr().main_animation(),
		                kAttackAnimationDuration);  // TODO(Nordfriese): proper animation
		return;
	}

	NEVER_HERE();
}

void Ship::ship_update_idle(Game& game, Bob::State& state) {

	if (state.ivar1 != 0) {
		// We've just completed one step, so give neighbours
		// a chance to move away first
		wakeup_neighbours(game);
		state.ivar1 = 0;
		schedule_act(game, Duration(25));
		return;
	}

	// If we are waiting for the next transport job, check if we should move away from ships and
	// shores
	const Map& map = game.map();
	switch (ship_state_) {
	case ShipStates::kTransport: {
		FCoords position = get_position();
		unsigned int dirs[LAST_DIRECTION + 1];
		unsigned int dirmax = 0;

		for (Direction dir = 0; dir <= LAST_DIRECTION; ++dir) {
			FCoords node = dir != 0u ? map.get_neighbour(position, dir) : position;
			dirs[dir] = (node.field->nodecaps() & MOVECAPS_WALK) != 0 ? 10 : 0;

			Area<FCoords> area(node, 0);
			std::vector<Bob*> ships;
			map.find_bobs(game, area, &ships, FindBobShip());

			for (Bob* it : ships) {
				if (it == this) {
					continue;
				}

				dirs[dir] += 3;
			}

			dirmax = std::max(dirmax, dirs[dir]);
		}

		if (dirmax != 0u) {
			unsigned int prob[LAST_DIRECTION + 1];
			unsigned int totalprob = 0;

			// The probability for moving into a given direction is also
			// affected by the "close" directions.
			for (Direction dir = 0; dir <= LAST_DIRECTION; ++dir) {
				prob[dir] = 10 * dirmax - 10 * dirs[dir];

				if (dir > 0) {
					unsigned int delta =
					   std::min(prob[dir], dirs[(dir % 6) + 1] + dirs[1 + ((dir - 1) % 6)]);
					prob[dir] -= delta;
				}

				totalprob += prob[dir];
			}

			if (totalprob == 0) {
				start_task_idle(game, descr().main_animation(), kShipInterval);
				return;
			}

			unsigned int rnd = game.logic_rand() % totalprob;
			Direction dir = 0;
			while (rnd >= prob[dir]) {
				rnd -= prob[dir];
				++dir;
			}

			if (dir == 0 || dir > LAST_DIRECTION) {
				start_task_idle(game, descr().main_animation(), kShipInterval);
				return;
			}

			FCoords neighbour = map.get_neighbour(position, dir);
			if ((neighbour.field->nodecaps() & MOVECAPS_SWIM) == 0) {
				start_task_idle(game, descr().main_animation(), kShipInterval);
				return;
			}

			state.ivar1 = 1;
			start_task_move(game, dir, descr().get_sail_anims(), false);
			return;
		}
		// No desire to move around, so sleep
		start_task_idle(game, descr().main_animation(), -1);
		return;
	}

	case ShipStates::kExpeditionScouting: {
		if (expedition_->island_exploration) {  // Exploration of the island
			if (exp_close_to_coast()) {
				if (expedition_->scouting_direction == WalkingDir::IDLE) {
					// Make sure we know the location of the coast and use it as initial direction we
					// come from
					expedition_->scouting_direction = WALK_SE;
					for (uint8_t secure = 0; exp_dir_swimmable(expedition_->scouting_direction);
					     ++secure) {
						if (secure >= 6) {
							throw wexception("Scouting ship on land");
						}
						expedition_->scouting_direction =
						   get_cw_neighbour(expedition_->scouting_direction);
					}
					expedition_->scouting_direction = get_backward_dir(expedition_->scouting_direction);
					// Save the position - this is where we start
					expedition_->exploration_start = get_position();
				} else {
					// Check whether the island was completely surrounded
					if (ship_type_ != ShipType::kWarship &&
					    get_position() == expedition_->exploration_start) {
						set_ship_state_and_notify(
						   ShipStates::kExpeditionWaiting, NoteShip::Action::kWaitingForCommand);
						send_message(game,
						             /** TRANSLATORS: A ship has circumnavigated an island and is waiting
						                for orders */
						             pgettext("ship", "Waiting"), _("Island Circumnavigated"),
						             _("An expedition ship sailed around its island without any events."),
						             "images/wui/ship/ship_explore_island_cw.png");
						return start_task_idle(game, descr().main_animation(), kShipInterval);
					}
				}
				// The ship is supposed to follow the coast as close as possible, therefore the check
				// for
				// a swimmable field begins at the neighbour field of the direction we came from.
				expedition_->scouting_direction = get_backward_dir(expedition_->scouting_direction);
				if (expedition_->island_explore_direction == IslandExploreDirection::kClockwise) {
					do {
						expedition_->scouting_direction =
						   get_ccw_neighbour(expedition_->scouting_direction);
					} while (!exp_dir_swimmable(expedition_->scouting_direction));
				} else {
					do {
						expedition_->scouting_direction =
						   get_cw_neighbour(expedition_->scouting_direction);
					} while (!exp_dir_swimmable(expedition_->scouting_direction));
				}
				state.ivar1 = 1;
				return start_task_move(
				   game, expedition_->scouting_direction, descr().get_sail_anims(), false);
			}
			// The ship got the command to scout around an island, but is not close to any island
			// Most likely the command was send as the ship was on an exploration and just leaving
			// the island - therefore we try to find the island again.
			FCoords position = get_position();
			for (uint8_t dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
				FCoords neighbour = map.get_neighbour(position, dir);
				for (uint8_t sur = FIRST_DIRECTION; sur <= LAST_DIRECTION; ++sur) {
					if ((map.get_neighbour(neighbour, sur).field->nodecaps() & MOVECAPS_SWIM) == 0) {
						// Okay we found the next coast, so now the ship should go there.
						// However, we do neither save the position as starting position, nor do we
						// save
						// the direction we currently go. So the ship can start exploring normally
						state.ivar1 = 1;
						return start_task_move(game, dir, descr().get_sail_anims(), false);
					}
				}
			}
			// if we are here, it seems something really strange happend.
			log_warn_time(game.get_gametime(),
			              "ship %s was not able to start exploration. Entering WAIT mode.",
			              shipname_.c_str());
			set_ship_state_and_notify(
			   ShipStates::kExpeditionWaiting, NoteShip::Action::kWaitingForCommand);
			start_task_idle(game, descr().main_animation(), kShipInterval);
			return;
		}  // scouting towards a specific direction
		if (exp_dir_swimmable(expedition_->scouting_direction)) {
			// the scouting direction is still free to move
			state.ivar1 = 1;
			start_task_move(game, expedition_->scouting_direction, descr().get_sail_anims(), false);
			return;
		}
		// coast reached
		set_ship_state_and_notify(
		   ShipStates::kExpeditionWaiting, NoteShip::Action::kWaitingForCommand);
		start_task_idle(game, descr().main_animation(), kShipInterval);
		if (ship_type_ != ShipType::kWarship) {
			// Send a message to the player, that a new coast was reached
			send_message(game,
			             /** TRANSLATORS: A ship has discovered land */
			             _("Land Ahoy!"), _("Coast Reached"),
			             _("An expedition ship reached a coast and is waiting for further commands."),
			             "images/wui/ship/ship_scout_ne.png");
		}
		return;
	}
	case ShipStates::kExpeditionColonizing: {
		assert(!expedition_->seen_port_buildspaces.empty());
		upcast(ConstructionSite, cs, map[expedition_->seen_port_buildspaces.front()].get_immovable());
		// some safety checks that we have identified the correct csite
		if ((cs != nullptr) && cs->get_owner() == get_owner() && cs->get_built_per64k() == 0 &&
		    owner().tribe().building_index(cs->building().name()) == owner().tribe().port()) {
			for (ShippingItem& si : items_) {
				WareInstance* ware;
				Worker* worker;
				si.get(game, &ware, &worker);
				assert((worker == nullptr) ^ (ware == nullptr));
				if (ware != nullptr) {
					WaresQueue* wq;
					try {
						wq = dynamic_cast<WaresQueue*>(
						   &cs->inputqueue(ware->descr_index(), wwWARE, nullptr));
						assert(wq);
					} catch (const WException&) {
						// cs->inputqueue() may throw if this is an additional item
						wq = nullptr;
					}
					// Wares are not preserved in the same way as workers. We register the ware as a
					// number in the building's statistics table, then delete the actual instance.
					if ((wq == nullptr) || wq->get_filled() >= wq->get_max_fill()) {
						cs->add_additional_ware(ware->descr_index());
					} else {
						wq->set_filled(wq->get_filled() + 1);
					}
					ware->remove(game);
				} else {
					worker->set_economy(nullptr, wwWARE);
					worker->set_economy(nullptr, wwWORKER);
					worker->set_location(cs);
					worker->set_position(game, cs->get_position());
					worker->reset_tasks(game);
					if ((cs->get_builder_request() != nullptr) &&
					    worker->descr().worker_index() == worker->get_owner()->tribe().builder()) {
						PartiallyFinishedBuilding::request_builder_callback(
						   game, *cs->get_builder_request(), worker->descr().worker_index(), worker, *cs);
					} else {
						cs->add_additional_worker(game, *worker);
					}
				}
			}
			items_.clear();
		} else {  // it seems that port constructionsite has disappeared
			// Send a message to the player, that a port constructionsite is gone
			send_message(game, _("Port Lost!"), _("New port construction site is gone"),
			             _("Unloading of wares failed, expedition is cancelled now."),
			             "images/wui/ship/menu_ship_cancel_expedition.png");
			send_signal(game, "cancel_expedition");
		}

		set_ship_state_and_notify(ShipStates::kTransport, NoteShip::Action::kDestinationChanged);

		init_fleet(game);

		// for case that there are any workers left on board
		// (applicable when port construction space is kLost)
		Worker* worker;
		for (ShippingItem& item : items_) {
			item.get(game, nullptr, &worker);
			if (worker != nullptr) {
				worker->reset_tasks(game);
				worker->start_task_shipping(game, nullptr);
			}
		}

		expedition_.reset(nullptr);
		return start_task_idle(game, descr().main_animation(), kShipInterval);
	}
	case ShipStates::kExpeditionWaiting:
	case ShipStates::kExpeditionPortspaceFound: {
		// wait for input
		start_task_idle(game, descr().main_animation(), kShipInterval);
		return;
	}
	case ShipStates::kSinkRequest:
	case ShipStates::kSinkAnimation:
		break;
	}
	NEVER_HERE();
}

void Ship::set_ship_state_and_notify(ShipStates state, NoteShip::Action action) {
	if (ship_state_ != state) {
		ship_state_ = state;
		Notifications::publish(NoteShip(this, action));
	}
}

bool Ship::check_port_space_still_available(Game& game) {
	assert(expedition_);
	// recheck ownership before setting the csite
	Map* map = game.mutable_map();
	if (expedition_->seen_port_buildspaces.empty()) {
		log_warn_time(
		   game.get_gametime(), "Expedition list of seen port spaces is unexpectedly empty!\n");
		return false;
	}
	const FCoords fc = map->get_fcoords(expedition_->seen_port_buildspaces.front());
	if (!can_build_port_here(get_owner()->player_number(), *map, fc)) {
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

void Ship::set_economy(const Game& game, Economy* e, WareWorker type) {
	// Do not check here that the economy actually changed, because on loading
	// we rely that wares really get reassigned our economy.

	(type == wwWARE ? ware_economy_ : worker_economy_) = e;
	for (ShippingItem& shipping_item : items_) {
		shipping_item.set_economy(game, e, type);
	}
}

bool Ship::has_destination() const {
	return destination_coords_ != nullptr || destination_object_.get(owner().egbase()) != nullptr;
}
PortDock* Ship::get_destination_port(EditorGameBase& e) const {
	if (MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::PORTDOCK) {
		return dynamic_cast<PortDock*>(mo);
	}
	return nullptr;
}
Ship* Ship::get_destination_ship(EditorGameBase& e) const {
	if (MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::SHIP) {
		return dynamic_cast<Ship*>(mo);
	}
	return nullptr;
}
PinnedNote* Ship::get_destination_note(EditorGameBase& e) const {
	if (MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::PINNED_NOTE) {
		return dynamic_cast<PinnedNote*>(mo);
	}
	return nullptr;
}

const PortDock* Ship::get_destination_port(const EditorGameBase& e) const {
	if (const MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::PORTDOCK) {
		return dynamic_cast<const PortDock*>(mo);
	}
	return nullptr;
}
const Ship* Ship::get_destination_ship(const EditorGameBase& e) const {
	if (const MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::SHIP) {
		return dynamic_cast<const Ship*>(mo);
	}
	return nullptr;
}
const PinnedNote* Ship::get_destination_note(const EditorGameBase& e) const {
	if (const MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::PINNED_NOTE) {
		return dynamic_cast<const PinnedNote*>(mo);
	}
	return nullptr;
}

void Ship::set_destination(EditorGameBase& egbase, MapObject* dest, bool is_playercommand) {
	assert(dest == nullptr || dest->descr().type() == MapObjectType::PORTDOCK ||
	       dest->descr().type() == MapObjectType::SHIP ||
	       dest->descr().type() == MapObjectType::PINNED_NOTE);

	destination_object_ = dest;
	destination_coords_ = nullptr;
	send_message_at_destination_ = is_playercommand;

	if (upcast(Game, g, &egbase)) {
		send_signal(*g, "wakeup");
	}

	if (is_playercommand) {
		assert(ship_state_ != ShipStates::kTransport);
		assert(expedition_ != nullptr);

		expedition_->scouting_direction = WalkingDir::IDLE;
		expedition_->island_exploration = false;

		set_ship_state_and_notify(
		   dest == nullptr ? ShipStates::kExpeditionWaiting : ShipStates::kExpeditionScouting,
		   NoteShip::Action::kDestinationChanged);
	} else {
		Notifications::publish(NoteShip(this, NoteShip::Action::kDestinationChanged));
	}
}
void Ship::set_destination(EditorGameBase& egbase,
                           const DetectedPortSpace& dest,
                           bool is_playercommand) {
	destination_object_ = nullptr;
	destination_coords_ = &dest;
	send_message_at_destination_ = is_playercommand;

	if (upcast(Game, g, &egbase)) {
		send_signal(*g, "wakeup");
	}

	if (is_playercommand) {
		assert(ship_state_ != ShipStates::kTransport);
		assert(expedition_ != nullptr);

		expedition_->scouting_direction = WalkingDir::IDLE;
		expedition_->island_exploration = false;

		set_ship_state_and_notify(
		   ShipStates::kExpeditionScouting, NoteShip::Action::kDestinationChanged);
	} else {
		Notifications::publish(NoteShip(this, NoteShip::Action::kDestinationChanged));
	}
}

void Ship::add_item(Game& game, const ShippingItem& item) {
	assert(items_.size() < get_capacity());

	items_.push_back(item);
	items_.back().set_location(game, this);
}

/**
 * Unload one item designated for given dock or for no dock.
 * \return true if item unloaded.
 */
bool Ship::withdraw_item(Game& game, PortDock& pd) {
	bool unloaded = false;
	size_t dst = 0;
	for (ShippingItem& si : items_) {
		if (!unloaded) {
			const PortDock* itemdest = si.get_destination(game);
			if ((itemdest == nullptr) || itemdest == &pd) {
				pd.shipping_item_arrived(game, si);
				unloaded = true;
				continue;
			}
		}
		items_[dst++] = si;
	}
	items_.resize(dst);
	return unloaded;
}

/**
 * Find a path to the dock @p pd, returns its length, and the path optionally.
 */
uint32_t Ship::calculate_sea_route(EditorGameBase& egbase, PortDock& pd, Path* finalpath) const {
	Map* map = egbase.mutable_map();
	StepEvalAStar se(pd.get_warehouse()->get_position());
	se.swim_ = true;
	se.conservative_ = false;
	se.estimator_bias_ = -5 * map->calc_cost(0);

	MapAStar<StepEvalAStar> astar(*map, se, wwWORKER);

	astar.push(get_position());

	int32_t cost;
	FCoords cur;
	while (astar.step(cur, cost)) {
		if (cur.field->get_immovable() == &pd) {
			if (finalpath != nullptr) {
				astar.pathto(cur, *finalpath);
				return finalpath->get_nsteps();
			}
			Path path;
			astar.pathto(cur, path);
			return path.get_nsteps();
		}
	}

	molog(egbase.get_gametime(), "   calculate_sea_distance: Failed to find path!\n");
	return std::numeric_limits<uint32_t>::max();
}

/**
 * Find a path to the dock @p pd and follow it without using precomputed paths.
 */
void Ship::start_task_movetodock(Game& game, PortDock& pd) {
	Path path;

	uint32_t const distance = calculate_sea_route(game, pd, &path);

	// if we get a meaningfull result
	if (distance < std::numeric_limits<uint32_t>::max()) {
		start_task_movepath(game, path, descr().get_sail_anims());
		return;
	}
	log_warn_time(
	   game.get_gametime(),
	   "start_task_movedock: Failed to find a path: ship at %3dx%3d to port at: %3dx%3d\n",
	   get_position().x, get_position().y, pd.get_positions(game)[0].x, pd.get_positions(game)[0].y);
	// This should not happen, but in theory there could be some inconstinency
	// I (tiborb) failed to invoke this situation when testing so
	// I am not sure if following line behaves allright
	get_fleet()->update(game);
	start_task_idle(game, descr().main_animation(), kFleetInterval.get());
}

/// Prepare everything for the coming exploration
void Ship::start_task_expedition(Game& game) {
	// Initialize a new, yet empty expedition
	expedition_.reset(new Expedition());
	expedition_->seen_port_buildspaces.clear();
	expedition_->island_exploration = false;
	expedition_->scouting_direction = WalkingDir::IDLE;
	expedition_->exploration_start = Coords(0, 0);
	expedition_->island_explore_direction = IslandExploreDirection::kClockwise;
	expedition_->attack_targets.clear();
	expedition_->ware_economy = get_owner()->create_economy(wwWARE);
	expedition_->worker_economy = get_owner()->create_economy(wwWORKER);

	// Now we are waiting
	set_ship_state_and_notify(ShipStates::kExpeditionWaiting, NoteShip::Action::kDestinationChanged);

	// We are no longer in any other economy, but instead are an economy of our
	// own.
	if (fleet_ != nullptr) {
		fleet_->remove_ship(game, this);
		assert(fleet_ == nullptr);
	}

	set_economy(game, expedition_->ware_economy, wwWARE);
	set_economy(game, expedition_->worker_economy, wwWORKER);

	for (const ShippingItem& si : items_) {
		WareInstance* ware;
		Worker* worker;
		si.get(game, &ware, &worker);
		if (worker != nullptr) {
			worker->reset_tasks(game);
			worker->start_task_idle(game, 0, -1);
		} else {
			assert(ware);
		}
	}

	// Send a message to the player that an expedition is ready to go
	if (ship_type_ == ShipType::kWarship) {
		send_message(game,
		             /** TRANSLATORS: Warship ready */
		             pgettext("ship", "Warship"), _("Warship Ready"),
		             _("A warship is waiting for your commands."),
		             "images/wui/buildings/start_expedition.png");
	} else {
		send_message(game,
		             /** TRANSLATORS: Ship expedition ready */
		             pgettext("ship", "Expedition"), _("Expedition Ready"),
		             _("An expedition ship is waiting for your commands."),
		             "images/wui/buildings/start_expedition.png");
	}
	Notifications::publish(NoteShip(this, NoteShip::Action::kWaitingForCommand));
}

/// Initializes / changes the direction of scouting to @arg direction
/// @note only called via player command
void Ship::exp_scouting_direction(Game& game, WalkingDir scouting_direction) {
	assert(expedition_ != nullptr);
	destination_object_ = nullptr;
	destination_coords_ = nullptr;
	set_ship_state_and_notify(
	   ShipStates::kExpeditionScouting, NoteShip::Action::kDestinationChanged);
	expedition_->scouting_direction = scouting_direction;
	expedition_->island_exploration = false;
	set_destination(game, nullptr);
}

WalkingDir Ship::get_scouting_direction() const {
	if (expedition_ != nullptr && ship_state_ == ShipStates::kExpeditionScouting &&
	    !expedition_->island_exploration) {
		return expedition_->scouting_direction;
	}
	return WalkingDir::IDLE;
}

/// Initializes the construction of a port at @arg c
/// @note only called via player command
void Ship::exp_construct_port(Game& game, const Coords& c) {
	assert(expedition_ != nullptr);
	// recheck ownership and availability before setting the csite
	if (!check_port_space_still_available(game)) {
		return;
	}
	get_owner()->force_csite(c, get_owner()->tribe().port()).set_destruction_blocked(true);

	// Make sure that we have space to squeeze in a lumberjack or a quarry
	std::vector<ImmovableFound> all_immos;
	game.map().find_immovables(game, Area<FCoords>(game.map().get_fcoords(c), 3), &all_immos,
	                           FindImmovableType(MapObjectType::IMMOVABLE));
	for (auto& immo : all_immos) {
		if (immo.object->descr().has_attribute(MapObjectDescr::get_attribute_id("rocks")) ||
		    dynamic_cast<Immovable*>(immo.object)->descr().has_terrain_affinity()) {
			immo.object->remove(game);
		}
	}
	set_ship_state_and_notify(
	   ShipStates::kExpeditionColonizing, NoteShip::Action::kDestinationChanged);
}

/// Initializes / changes the direction the island exploration in @arg island_explore_direction
/// direction
/// @note only called via player command
void Ship::exp_explore_island(Game& game, IslandExploreDirection island_explore_direction) {
	assert(expedition_ != nullptr);
	destination_object_ = nullptr;
	destination_coords_ = nullptr;
	set_ship_state_and_notify(
	   ShipStates::kExpeditionScouting, NoteShip::Action::kDestinationChanged);
	expedition_->island_explore_direction = island_explore_direction;
	expedition_->scouting_direction = WalkingDir::IDLE;
	expedition_->island_exploration = true;
	set_destination(game, nullptr);
}

IslandExploreDirection Ship::get_island_explore_direction() const {
	if (expedition_ != nullptr && ship_state_ == ShipStates::kExpeditionScouting &&
	    expedition_->island_exploration) {
		return expedition_->island_explore_direction;
	}
	return IslandExploreDirection::kNotSet;
}

/// Cancels a currently running expedition
/// @note only called via player command
void Ship::exp_cancel(Game& game) {
	// Running colonization has the highest priority before cancelation
	// + cancelation only works if an expedition is actually running

	if ((ship_state_ == ShipStates::kExpeditionColonizing) || !state_is_expedition()) {
		return;
	}

	// The workers were hold in an idle state so that they did not try
	// to become fugitive or run to the next warehouse. But now, we
	// have a proper destination, so we can just inform them that they
	// are now getting shipped there.
	// Theres nothing to be done for wares - they already changed
	// economy with us and the warehouse will make sure that they are
	// getting used.
	Worker* worker;
	for (ShippingItem& item : items_) {
		item.get(game, nullptr, &worker);
		if (worker != nullptr) {
			worker->reset_tasks(game);
			worker->start_task_shipping(game, nullptr);
		}
	}
	set_ship_state_and_notify(ShipStates::kTransport, NoteShip::Action::kDestinationChanged);

	// Bring us back into a fleet and a economy.
	set_economy(game, nullptr, wwWARE);
	set_economy(game, nullptr, wwWORKER);
	init_fleet(game);
	if ((get_fleet() == nullptr) || !get_fleet()->has_ports()) {
		// We lost our last reachable port, so we reset the expedition's state
		set_ship_state_and_notify(
		   ShipStates::kExpeditionWaiting, NoteShip::Action::kDestinationChanged);
		if (fleet_ != nullptr) {
			fleet_->remove_ship(game, this);
			assert(fleet_ == nullptr);
		}
		set_economy(game, expedition_->ware_economy, wwWARE);
		set_economy(game, expedition_->worker_economy, wwWORKER);

		worker = nullptr;
		for (ShippingItem& item : items_) {
			item.get(game, nullptr, &worker);
			if (worker != nullptr) {
				worker->reset_tasks(game);
				worker->start_task_idle(game, 0, -1);
			}
		}

		Notifications::publish(NoteShip(this, NoteShip::Action::kNoPortLeft));
		return;
	}
	assert(get_economy(wwWARE) && get_economy(wwWARE) != expedition_->ware_economy);
	assert(get_economy(wwWORKER) && get_economy(wwWORKER) != expedition_->worker_economy);

	send_signal(game, "cancel_expedition");

	// Delete the expedition and the economy it created.
	expedition_.reset(nullptr);
}

/// Sinks the ship
/// @note only called via player command
void Ship::sink_ship(Game& game) {
	// Running colonization has the highest priority + a sink request is only valid once
	if (!state_is_sinkable()) {
		return;
	}
	ship_state_ = ShipStates::kSinkRequest;
	// Make sure the ship is active and close possible open windows
	ship_wakeup(game);
}

constexpr int kShipHealthBarWidth = 30;

void Ship::draw(const EditorGameBase& egbase,
                const InfoToDraw& info_to_draw,
                const Vector2f& field_on_dst,
                const Widelands::Coords& coords,
                const float scale,
                RenderTarget* dst) const {
	Bob::draw(egbase, info_to_draw, field_on_dst, coords, scale, dst);

	// Show ship name and current activity
	std::string statistics_string;
	if ((info_to_draw & InfoToDraw::kStatistics) != 0) {
		if (has_battle()) {
			statistics_string = pgettext("ship_state", "Fighting");
		} else if (is_refitting()) {
			switch (pending_refit_) {
			case ShipType::kTransport:
				statistics_string = pgettext("ship_state", "Refitting to Transport Ship");
				break;
			case ShipType::kWarship:
				statistics_string = pgettext("ship_state", "Refitting to Warship");
				break;
			}
		} else {
			if (ship_type_ == ShipType::kWarship) {
				// TODO(Nordfriese): maybe show more state here
				statistics_string = pgettext("ship_state", "Warship");
			} else {
				switch (ship_state_) {
				case (ShipStates::kTransport): {
					const MapObject* dest = destination_object_.get(egbase);
					if (destination_coords_ != nullptr) {
						format(pgettext("ship_state", "Sailing to %s"),
						       destination_coords_->to_long_string(egbase));
					} else if (dest == nullptr) {
						/** TRANSLATORS: This is a ship state. The ship is ready
						 * to transport wares, but has nothing to do. */
						statistics_string = pgettext("ship_state", "Empty");
					} else if (dest->descr().type() == MapObjectType::SHIP) {
						statistics_string =
						   /** TRANSLATORS: This is a ship state. The ship is
						    * currently sailing to a specific destination ship. */
						   format(pgettext("ship_state", "Sailing to %s"),
						          dynamic_cast<const Ship*>(dest)->get_shipname());
					} else if (dest->descr().type() == MapObjectType::PINNED_NOTE) {
						statistics_string =
						   /** TRANSLATORS: This is a ship state. The ship is
						    * currently sailing to a specific destination note. */
						   format(pgettext("ship_state", "Sailing to %s"),
						          dynamic_cast<const PinnedNote*>(dest)->get_text());
					} else {
						const std::string& wh_name =
						   dynamic_cast<const PortDock*>(dest)->get_warehouse()->get_warehouse_name();
						if (fleet_->get_schedule().is_busy(*this)) {
							statistics_string =
							   /** TRANSLATORS: This is a ship state. The ship is currently
							    * transporting wares to a specific destination port. */
							   format(pgettext("ship_state", "Shipping to %s"), wh_name);
						} else {
							statistics_string =
							   /** TRANSLATORS: This is a ship state. The ship is currently sailing
							    * to a specific destination port without transporting wares. */
							   format(pgettext("ship_state", "Sailing to %s"), wh_name);
						}
					}
					break;
				}
				case (ShipStates::kExpeditionWaiting):
					/** TRANSLATORS: This is a ship state. An expedition is waiting for your commands. */
					statistics_string = pgettext("ship_state", "Waiting");
					break;
				case (ShipStates::kExpeditionScouting):
					/** TRANSLATORS: This is a ship state. An expedition is scouting for port spaces. */
					statistics_string = pgettext("ship_state", "Scouting");
					break;
				case (ShipStates::kExpeditionPortspaceFound):
					/** TRANSLATORS: This is a ship state. An expedition has found a port space. */
					statistics_string = pgettext("ship_state", "Port Space Found");
					break;
				case (ShipStates::kExpeditionColonizing):
					/** TRANSLATORS: This is a ship state. An expedition is unloading wares/workers to
					 * build a port. */
					statistics_string = pgettext("ship_state", "Founding a Colony");
					break;
				case (ShipStates::kSinkRequest):
				case (ShipStates::kSinkAnimation):
					break;
				}
			}
		}
		statistics_string = StyleManager::color_tag(
		   statistics_string, g_style_manager->building_statistics_style().medium_color());
	}

	const Vector2f point_on_dst = calc_drawpos(egbase, field_on_dst, scale);
	do_draw_info(info_to_draw, shipname_, statistics_string, point_on_dst, scale, dst);

	if ((info_to_draw & InfoToDraw::kSoldierLevels) != 0 &&
	    (ship_type_ == ShipType::kWarship || hitpoints_ < descr().max_hitpoints_)) {
		draw_healthbar(egbase, dst, point_on_dst, scale);
	}
}

void Ship::draw_healthbar(const EditorGameBase& egbase,
                          RenderTarget* dst,
                          const Vector2f& point_on_dst,
                          float scale) const {
	// TODO(Nordfriese): Common code with Soldier::draw_info_icon
	const RGBColor& color = owner().get_playercolor();
	const uint16_t color_sum = color.r + color.g + color.b;

	const Vector2i draw_position = point_on_dst.cast<int>();

	// The frame gets a slight tint of player color
	const Recti energy_outer(draw_position - Vector2i(kShipHealthBarWidth, 0) * scale,
	                         kShipHealthBarWidth * 2 * scale, 5 * scale);
	dst->fill_rect(energy_outer, color);
	dst->brighten_rect(energy_outer, 230 - color_sum / 3);

	// Adjust health to current animation tick
	uint32_t health_to_show = hitpoints_;
	if (has_battle() &&
	    battles_.back().phase == (battles_.back().is_first ? Battle::Phase::kDefenderAttacking :
                                                            Battle::Phase::kAttackerAttacking)) {
		uint32_t pending_damage =
		   battles_.back().pending_damage *
		   (owner().egbase().get_gametime() - battles_.back().time_of_last_action).get() /
		   kAttackAnimationDuration;
		if (pending_damage > health_to_show) {
			health_to_show = 0;
		} else {
			health_to_show -= pending_damage;
		}
	}

	// Now draw the health bar itself
	const int health_width = 2 * (kShipHealthBarWidth - 1) * health_to_show / descr().max_hitpoints_;

	Recti energy_inner(draw_position + Vector2i(-kShipHealthBarWidth + 1, 1) * scale,
	                   health_width * scale, 3 * scale);
	Recti energy_complement(energy_inner.origin() + Vector2i(health_width, 0) * scale,
	                        (2 * (kShipHealthBarWidth - 1) - health_width) * scale, 3 * scale);

	const RGBColor complement_color =
	   color_sum > 128 * 3 ? RGBColor(32, 32, 32) : RGBColor(224, 224, 224);
	dst->fill_rect(energy_inner, color);
	dst->fill_rect(energy_complement, complement_color);

	// Now soldier strength bonus indicators
	constexpr unsigned kBonusIconSize = 6;
	constexpr unsigned kMaxRows = 16;
	const unsigned bonus = get_sea_attack_soldier_bonus(egbase);
	if (bonus > 0) {
		unsigned n_cols = std::min(2 * kShipHealthBarWidth / kBonusIconSize, bonus);
		unsigned n_rows = std::min(kMaxRows - 1, bonus / n_cols);
		if (n_cols * n_rows < bonus) {
			++n_rows;
		}
		while (n_cols * n_rows < bonus) {
			++n_cols;
		}
		const unsigned last_row_cols = n_cols - (n_cols * n_rows - bonus);

		for (unsigned row = 0; row < n_rows; ++row) {
			const unsigned cols_in_row = (row + 1 < n_rows ? n_cols : last_row_cols);
			for (unsigned col = 0; col < cols_in_row; ++col) {
				Recti rect(
				   draw_position.x + ((col - cols_in_row * 0.5f) * kBonusIconSize + 1.f) * scale,
				   draw_position.y + (7.f + row * kBonusIconSize) * scale,
				   (kBonusIconSize - 2.f) * scale, (kBonusIconSize - 2.f) * scale);
				dst->fill_rect(rect, color);
				dst->draw_rect(rect, complement_color);
			}
		}
	}
}

void Ship::log_general_info(const EditorGameBase& egbase) const {
	Bob::log_general_info(egbase);

	molog(egbase.get_gametime(), "Name: %s", get_shipname().c_str());
	molog(egbase.get_gametime(), "Ship belongs to fleet %u\nlastdock: %s\n",
	      fleet_ != nullptr ? fleet_->serial() : 0,
	      (lastdock_.is_set()) ?
            format("%u (%s at %3dx%3d)", lastdock_.serial(),
	                lastdock_.get(egbase)->get_warehouse()->get_warehouse_name().c_str(),
	                lastdock_.get(egbase)->get_positions(egbase)[0].x,
	                lastdock_.get(egbase)->get_positions(egbase)[0].y)
	            .c_str() :
            "-");
	if (const PortDock* dock = get_destination_port(egbase); dock != nullptr) {
		molog(egbase.get_gametime(), "Has destination port %u (%3dx%3d) %s\n", dock->serial(),
		      dock->get_positions(egbase)[0].x, dock->get_positions(egbase)[0].y,
		      dock->get_warehouse()->get_warehouse_name().c_str());
	} else if (const Ship* ship = get_destination_ship(egbase); ship != nullptr) {
		molog(egbase.get_gametime(), "Has destination ship %u (%3dx%3d) %s\n", ship->serial(),
		      ship->get_position().x, ship->get_position().y, ship->get_shipname().c_str());
	} else if (const PinnedNote* note = get_destination_note(egbase); note != nullptr) {
		molog(egbase.get_gametime(), "Has destination note %u (%3dx%3d) %s\n", note->serial(),
		      note->get_position().x, note->get_position().y, note->get_text().c_str());
	} else if (destination_coords_ != nullptr) {
		molog(egbase.get_gametime(), "Has destination detected port space %u at %3dx%3d\n",
		      destination_coords_->serial, destination_coords_->coords.x,
		      destination_coords_->coords.y);
	} else {
		molog(egbase.get_gametime(), "No destination\n");
	}

	molog(egbase.get_gametime(), "In state: %u (%s)\n", static_cast<unsigned int>(ship_state_),
	      (expedition_) ? "expedition" : "transportation");

	if (is_on_destination_dock()) {
		molog(egbase.get_gametime(), "Currently in destination portdock\n");
	}

	molog(egbase.get_gametime(), "Carrying %" PRIuS " items%s\n", items_.size(),
	      (items_.empty()) ? "." : ":");

	for (const ShippingItem& shipping_item : items_) {
		molog(egbase.get_gametime(), "  * %u (%s), destination: %s\n", shipping_item.object_.serial(),
		      shipping_item.object_.get(egbase)->descr().name().c_str(),
		      (shipping_item.destination_dock_.is_set()) ?
               format("%u (%d x %d)", shipping_item.destination_dock_.serial(),
		                shipping_item.destination_dock_.get(egbase)->get_positions(egbase)[0].x,
		                shipping_item.destination_dock_.get(egbase)->get_positions(egbase)[0].y)

		            .c_str() :
               "-");
	}
}

/**
 * Send a message to the owning player.
 *
 * It will have the ship's coordinates, and display a picture in its description.
 *
 * \param msgsender a computer-readable description of why the message was sent
 * \param title short title to be displayed in message listings
 * \param heading long title to be displayed within the message
 * \param description user-visible message body, will be placed in an appropriate rich-text
 *paragraph
 * \param picture the filename to be used for the icon in message listings
 */
void Ship::send_message(Game& game,
                        const std::string& title,
                        const std::string& heading,
                        const std::string& description,
                        const std::string& picture) {
	const std::string rt_description =
	   as_mapobject_message(picture, g_image_cache->get(picture)->width(), description);

	get_owner()->add_message(game, std::unique_ptr<Message>(new Message(
	                                  Message::Type::kSeafaring, game.get_gametime(), title, picture,
	                                  heading, rt_description, get_position(), serial_)));
}

Ship::Expedition::~Expedition() {
	if (ware_economy != nullptr) {
		ware_economy->owner().remove_economy(ware_economy->serial());
	}
	if (worker_economy != nullptr) {
		worker_economy->owner().remove_economy(worker_economy->serial());
	}
}

/*
==============================

Load / Save implementation

==============================
*/

/* Changelog:
 * 12 - v1.1
 * 13 - Added warships and naval warfare.
 * 14 - Another naval warfare change (coords as destination).
 */
constexpr uint8_t kCurrentPacketVersion = 14;

const Bob::Task* Ship::Loader::get_task(const std::string& name) {
	if (name == "shipidle" || name == "ship") {
		return &taskShip;
	}
	return Bob::Loader::get_task(name);
}

void Ship::Loader::load(FileRead& fr, uint8_t packet_version) {
	// TODO(Nordfriese): Savegame compatibility v1.1
	if (packet_version >= 12 && packet_version <= kCurrentPacketVersion) {
		Bob::Loader::load(fr);
		// Economy
		ware_economy_serial_ = fr.unsigned_32();
		worker_economy_serial_ = fr.unsigned_32();

		// The state the ship is in
		ship_state_ = static_cast<ShipStates>(fr.unsigned_8());
		ship_type_ =
		   (packet_version >= 13) ? static_cast<ShipType>(fr.unsigned_8()) : ShipType::kTransport;
		pending_refit_ = (packet_version >= 13) ? static_cast<ShipType>(fr.unsigned_8()) : ship_type_;

		// Expedition specific data
		switch (ship_state_) {
		case ShipStates::kExpeditionScouting:
		case ShipStates::kExpeditionWaiting:
		case ShipStates::kExpeditionPortspaceFound:
		case ShipStates::kExpeditionColonizing: {
			expedition_.reset(new Expedition());
			// Currently seen port build spaces
			expedition_->seen_port_buildspaces.clear();
			uint8_t numofports = fr.unsigned_8();
			for (uint8_t i = 0; i < numofports; ++i) {
				expedition_->seen_port_buildspaces.push_back(read_coords_32(&fr));
			}
			// Swimability of the directions
			for (bool& swimmable : expedition_->swimmable) {
				swimmable = (fr.unsigned_8() != 0u);
			}
			// whether scouting or exploring
			expedition_->island_exploration = (fr.unsigned_8() != 0u);
			// current direction
			expedition_->scouting_direction = static_cast<WalkingDir>(fr.unsigned_8());
			// Start coordinates of an island exploration
			expedition_->exploration_start = read_coords_32(&fr);
			// Whether the exploration is done clockwise or counter clockwise
			expedition_->island_explore_direction =
			   static_cast<IslandExploreDirection>(fr.unsigned_8());
			for (unsigned i = (packet_version >= 13) ? fr.unsigned_32() : 0; i > 0; --i) {
				expedition_attack_target_serials_.insert(fr.unsigned_32());
			}
		} break;

		default:
			ship_state_ = ShipStates::kTransport;
			break;
		}

		for (uint8_t i = (packet_version >= 13) ? fr.unsigned_8() : 0; i != 0U; --i) {
			const bool first = fr.unsigned_8() != 0U;
			battle_serials_.push_back(fr.unsigned_32());
			battles_.emplace_back(nullptr, Coords::null(), std::vector<uint32_t>(), first);
			battles_.back().phase = static_cast<Battle::Phase>(fr.unsigned_8());
			battles_.back().attack_coords.x = fr.signed_16();
			battles_.back().attack_coords.y = fr.signed_16();
			battles_.back().pending_damage = fr.unsigned_32();
			for (size_t j = fr.unsigned_32(); j > 0U; --j) {
				battles_.back().attack_soldier_serials.push_back(fr.unsigned_32());
			}
			battles_.back().time_of_last_action = Time(fr);
		}
		hitpoints_ = (packet_version >= 13) ? fr.unsigned_32() : -1;
		if (packet_version >= 13) {
			last_heal_time_ = Time(fr);
			send_message_at_destination_ = fr.unsigned_8() != 0;
		}

		shipname_ = fr.c_string();
		capacity_ = fr.unsigned_32();
		warship_soldier_capacity_ = (packet_version >= 13) ? fr.unsigned_32() : capacity_;
		lastdock_ = fr.unsigned_32();
		destination_object_ = fr.unsigned_32();
		destination_coords_ = packet_version >= 14 ? fr.unsigned_32() : 0;

		items_.resize(fr.unsigned_32());
		for (ShippingItem::Loader& item_loader : items_) {
			item_loader.load(fr);
		}
	} else {
		throw UnhandledVersionError("MapObjectPacket::Ship", packet_version, kCurrentPacketVersion);
	}
}

void Ship::Loader::load_pointers() {
	Bob::Loader::load_pointers();

	Ship& ship = get<Ship>();

	if (lastdock_ != 0u) {
		ship.lastdock_ = &mol().get<PortDock>(lastdock_);
	}
	if (destination_object_ != 0u) {
		MapObject& mo = mol().get<MapObject>(destination_object_);
		assert(mo.descr().type() == MapObjectType::PORTDOCK ||
		       mo.descr().type() == MapObjectType::SHIP ||
		       mo.descr().type() == MapObjectType::PINNED_NOTE);
		ship.destination_object_ = &mo;
	} else {
		ship.destination_object_ = nullptr;
	}
	ship.destination_coords_ = destination_coords_ == 0U ?
                                 nullptr :
                                 &ship.owner().get_detected_port_space(destination_coords_);

	for (Serial serial : expedition_attack_target_serials_) {
		if (serial != 0) {
			expedition_->attack_targets.insert(&mol().get<Ship>(serial));
		}
	}

	for (uint32_t i = 0; i < battle_serials_.size(); ++i) {
		if (battle_serials_[i] != 0U) {
			battles_[i].opponent = &mol().get<Ship>(battle_serials_[i]);
		}
	}

	ship.items_.resize(items_.size());
	for (uint32_t i = 0; i < items_.size(); ++i) {
		ship.items_[i] = items_[i].get(mol());
	}
}

void Ship::Loader::load_finish() {
	Bob::Loader::load_finish();

	Ship& ship = get<Ship>();

	// The economy can sometimes be nullptr (e.g. when there are no ports).
	if (ware_economy_serial_ != kInvalidSerial) {
		ship.ware_economy_ = ship.get_owner()->get_economy(ware_economy_serial_);
		if (ship.ware_economy_ == nullptr) {
			ship.ware_economy_ = ship.get_owner()->create_economy(ware_economy_serial_, wwWARE);
		}
	}
	if (worker_economy_serial_ != kInvalidSerial) {
		ship.worker_economy_ = ship.get_owner()->get_economy(worker_economy_serial_);
		if (ship.worker_economy_ == nullptr) {
			ship.worker_economy_ = ship.get_owner()->create_economy(worker_economy_serial_, wwWORKER);
		}
	}

	// restore the state the ship is in
	ship.ship_state_ = ship_state_;
	ship.ship_type_ = ship_type_;
	ship.pending_refit_ = pending_refit_;
	ship.hitpoints_ = (hitpoints_ < 0) ? ship.descr().max_hitpoints_ : hitpoints_;
	ship.last_heal_time_ = last_heal_time_;
	ship.send_message_at_destination_ = send_message_at_destination_;

	// restore the  ship id and name
	ship.shipname_ = shipname_;

	ship.capacity_ = capacity_;
	ship.warship_soldier_capacity_ = warship_soldier_capacity_;

	// if the ship is on an expedition, restore the expedition specific data
	if (expedition_) {
		ship.expedition_.swap(expedition_);
		ship.expedition_->ware_economy = ship.ware_economy_;
		ship.expedition_->worker_economy = ship.worker_economy_;
	} else {
		assert(ship_state_ == ShipStates::kTransport);
	}
	ship.battles_ = battles_;

	// Workers load code set their economy to the economy of their location
	// (which is a PlayerImmovable), that means that workers on ships do not get
	// a correct economy assigned. We, as ship therefore have to reset the
	// economy of all workers we're transporting so that they are in the correct
	// economy. Also, we might are on an expedition which means that we just now
	// created the economy of this ship and must inform all wares.
	ship.set_economy(dynamic_cast<Game&>(egbase()), ship.ware_economy_, wwWARE);
	ship.set_economy(dynamic_cast<Game&>(egbase()), ship.worker_economy_, wwWORKER);
	ship.get_owner()->add_ship(ship.serial());

	// The ship's serial may have changed, inform onboard workers
	for (uint32_t i = 0; i < ship.items_.size(); ++i) {
		Worker* worker;
		ship.items_[i].get(ship.owner().egbase(), nullptr, &worker);
		if (worker != nullptr) {
			worker->set_ship_serial(ship.serial());
		}
	}
}

MapObject::Loader* Ship::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);
	try {
		// The header has been peeled away by the caller
		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version >= 12 && packet_version <= kCurrentPacketVersion) {
			try {
				const ShipDescr* descr = nullptr;
				// Removing this will break the test suite
				std::string name = fr.c_string();
				const DescriptionIndex& ship_index = egbase.descriptions().safe_ship_index(name);
				descr = egbase.descriptions().get_ship_descr(ship_index);
				loader->init(egbase, mol, descr->create_object());
				loader->load(fr, packet_version);
			} catch (const WException& e) {
				throw GameDataError("Failed to load ship: %s", e.what());
			}
		} else {
			throw UnhandledVersionError("Ship", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading ship: %s", e.what());
	}

	return loader.release();
}

void Ship::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderShip);
	fw.unsigned_8(kCurrentPacketVersion);
	fw.c_string(descr().name());

	Bob::save(egbase, mos, fw);

	// The economy can sometimes be nullptr (e.g. when there are no ports).
	fw.unsigned_32(ware_economy_ != nullptr ? ware_economy_->serial() : kInvalidSerial);
	fw.unsigned_32(worker_economy_ != nullptr ? worker_economy_->serial() : kInvalidSerial);

	// state the ship is in
	fw.unsigned_8(static_cast<uint8_t>(ship_state_));
	fw.unsigned_8(static_cast<uint8_t>(ship_type_));
	fw.unsigned_8(static_cast<uint8_t>(pending_refit_));

	// expedition specific data
	if (state_is_expedition()) {
		// currently seen port buildspaces
		fw.unsigned_8(expedition_->seen_port_buildspaces.size());
		for (const Coords& coords : expedition_->seen_port_buildspaces) {
			write_coords_32(&fw, coords);
		}
		// swimmability of the directions
		for (const bool& swim : expedition_->swimmable) {
			fw.unsigned_8(swim ? 1 : 0);
		}
		// whether scouting or exploring
		fw.unsigned_8(expedition_->island_exploration ? 1 : 0);
		// current direction
		fw.unsigned_8(static_cast<uint8_t>(expedition_->scouting_direction));
		// Start coordinates of an island exploration
		write_coords_32(&fw, expedition_->exploration_start);
		// Whether the exploration is done clockwise or counter clockwise
		fw.unsigned_8(static_cast<uint8_t>(expedition_->island_explore_direction));
		fw.unsigned_32(expedition_->attack_targets.size());
		for (const auto& ptr : expedition_->attack_targets) {
			fw.unsigned_32(mos.get_object_file_index_or_zero(ptr.get(egbase)));
		}
	}
	fw.unsigned_8(battles_.size());
	for (const Battle& b : battles_) {
		fw.unsigned_8(b.is_first ? 1 : 0);
		fw.unsigned_32(mos.get_object_file_index_or_zero(b.opponent.get(egbase)));
		fw.unsigned_8(static_cast<uint8_t>(b.phase));
		fw.signed_16(b.attack_coords.x);
		fw.signed_16(b.attack_coords.y);
		fw.unsigned_32(b.pending_damage);
		fw.unsigned_32(b.attack_soldier_serials.size());
		for (Serial s : b.attack_soldier_serials) {
			fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(s)));
		}
		b.time_of_last_action.save(fw);
	}
	fw.unsigned_32(hitpoints_);
	last_heal_time_.save(fw);
	fw.unsigned_8(send_message_at_destination_ ? 1 : 0);

	fw.string(shipname_);
	fw.unsigned_32(capacity_);
	fw.unsigned_32(warship_soldier_capacity_);
	fw.unsigned_32(mos.get_object_file_index_or_zero(lastdock_.get(egbase)));
	fw.unsigned_32(mos.get_object_file_index_or_zero(destination_object_.get(egbase)));
	fw.unsigned_32(destination_coords_ == nullptr ? 0 : destination_coords_->serial);

	fw.unsigned_32(items_.size());
	for (ShippingItem& shipping_item : items_) {
		shipping_item.save(egbase, mos, fw);
	}
}

}  // namespace Widelands
