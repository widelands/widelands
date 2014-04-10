/*
 * Copyright (C) 2010-2013 by the Widelands Development Team
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

#include "logic/ship.h"

#include <boost/foreach.hpp>

#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/fleet.h"
#include "economy/portdock.h"
#include "economy/wares_queue.h"
#include "graphic/graphic.h"
#include "logic/constructionsite.h"
#include "logic/findbob.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/mapastar.h"
#include "logic/mapregion.h"
#include "logic/path.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "ref_cast.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

Ship_Descr::Ship_Descr
	(const char * given_name, const char * gdescname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 const Tribe_Descr & gtribe)
: BobDescr(given_name, gdescname, directory, prof, global_s, &gtribe)
{
	m_sail_anims.parse(*this, directory, prof, "sail");

	Section * sinking_s = prof.get_section("sinking");
	if (sinking_s)
		add_animation("sinking", g_gr->animations().load(directory, *sinking_s));

	m_capacity     = global_s.get_natural("capacity", 20);
	m_vision_range = global_s.get_natural("vision_range", 7);
}

uint32_t Ship_Descr::movecaps() const {
	return MOVECAPS_SWIM;
}

Bob & Ship_Descr::create_object() const {
	return *new Ship(*this);
}


Ship::Ship(const Ship_Descr & gdescr) :
	Bob(gdescr),
	m_window(nullptr),
	m_fleet(nullptr),
	m_economy(nullptr),
	m_ship_state(TRANSPORT)
{
}

Ship::~Ship() {
	close_window();
}

Bob::Type Ship::get_bob_type() const {
	return SHIP;
}

PortDock* Ship::get_destination(Editor_Game_Base& egbase) const {
	return m_destination.get(egbase);
}

PortDock* Ship::get_lastdock(Editor_Game_Base& egbase) const {
	return m_lastdock.get(egbase);
}

Fleet* Ship::get_fleet() const {
	return m_fleet;
}

void Ship::init_auto_task(Game & game) {
	start_task_ship(game);
}

void Ship::init(Editor_Game_Base & egbase) {
	Bob::init(egbase);
	init_fleet(egbase);
}

/**
 * Create the initial singleton @ref Fleet to which we belong.
 * The fleet code will automatically merge us into a larger
 * fleet, if one is reachable.
 */
void Ship::init_fleet(Editor_Game_Base & egbase) {
	assert(get_owner() != nullptr);
	Fleet * fleet = new Fleet(*get_owner());
	fleet->add_ship(this);
	fleet->init(egbase);
	// fleet calls the set_fleet function appropriately
}

void Ship::cleanup(Editor_Game_Base & egbase) {
	if (m_fleet) {
		m_fleet->remove_ship(egbase, this);
	}

	while (!m_items.empty()) {
		m_items.back().remove(egbase);
		m_items.pop_back();
	}

	Bob::cleanup(egbase);
}

/**
 * This function is to be called only by @ref Fleet.
 */
void Ship::set_fleet(Fleet * fleet) {
	m_fleet = fleet;
}

void Ship::wakeup_neighbours(Game & game) {
	FCoords position = get_position();
	Area<FCoords> area(position, 1);
	std::vector<Bob *> ships;
	game.map().find_bobs(area, &ships, FindBobShip());

	for
		(std::vector<Bob *>::const_iterator it = ships.begin();
		 it != ships.end(); ++it)
	{
		if (*it == this)
			continue;

		static_cast<Ship *>(*it)->ship_wakeup(game);
	}
}


/**
 * Standard behaviour of ships.
 *
 * ivar1 = helper flag for coordination of mutual evasion of ships
 */
const Bob::Task Ship::taskShip = {
	"ship",
	static_cast<Bob::Ptr>(&Ship::ship_update),
	nullptr,
	nullptr,
	true // unique task
};

void Ship::start_task_ship(Game & game) {
	push_task(game, taskShip);
	top_state().ivar1 = 0;
}

void Ship::ship_wakeup(Game & game) {
	if (get_state(taskShip))
		send_signal(game, "wakeup");
}

void Ship::ship_update(Game & game, Bob::State & state) {
	// Handle signals
	std::string signal = get_signal();
	if (!signal.empty()) {
		if (signal == "wakeup") {
			signal_handled();
		} else if (signal == "cancel_expedition") {
			pop_task(game);
			PortDock* dst = m_fleet->get_arbitrary_dock();
			// TODO(sirver): What happens if there is no port anymore?
			if (dst) {
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

	switch (m_ship_state) {
		case TRANSPORT:
			if (ship_update_transport(game, state))
				return;
			break;
		case EXP_FOUNDPORTSPACE:
		case EXP_SCOUTING:
		case EXP_WAITING:
			ship_update_expedition(game, state);
			break;
		case EXP_COLONIZING:
			break;
		case SINK_REQUEST:
			if (descr().is_animation_known("sinking")) {
				m_ship_state = SINK_ANIMATION;
				start_task_idle(game, descr().get_animation("sinking"), 3000);
				return;
			}
			log("Oh no... this ship has no sinking animation :(!\n");
			// fall trough
		case SINK_ANIMATION:
			// The sink animation has been played, so finally remove the ship from the map
			pop_task(game);
			remove(game);
			return;
		default:
			assert(false); // never here
	}

	// if the real update function failed (e.g. nothing to transport), the ship goes idle
	ship_update_idle(game, state);
}


/// updates a ships tasks in transport mode \returns false if failed to update tasks
bool Ship::ship_update_transport(Game & game, Bob::State &) {
	Map & map = game.map();

	PortDock * dst = get_destination(game);
	if (!dst) {
		molog("ship_update: No destination anymore.\n");
		if (m_items.empty())
			return false;
		molog("but it has wares....\n");
		pop_task(game);
		PortDock* other_dock = m_fleet->get_arbitrary_dock();
		// TODO(sirver): What happens if there is no port anymore?
		if (other_dock) {
			set_destination(game, *other_dock);
		} else {
			start_task_idle(game, descr().main_animation(), 2000);
		}
		return true;
	}

	FCoords position = map.get_fcoords(get_position());
	if (position.field->get_immovable() == dst) {
		molog("ship_update: Arrived at dock %u\n", dst->serial());
		m_lastdock = dst;
		m_destination = nullptr;
		dst->ship_arrived(game, *this);
		start_task_idle(game, descr().main_animation(), 250);
		return true;
	}

	molog("ship_update: Go to dock %u\n", dst->serial());

	PortDock * lastdock = m_lastdock.get(game);
	if (lastdock && lastdock != dst) {
		molog("ship_update: Have lastdock %u\n", lastdock->serial());

		Path path;
		if (m_fleet->get_path(*lastdock, *dst, path)) {
			uint32_t closest_idx = std::numeric_limits<uint32_t>::max();
			uint32_t closest_dist = std::numeric_limits<uint32_t>::max();
			Coords closest_target(Coords::Null());

			Coords cur(path.get_start());
			for (uint32_t idx = 0; idx <= path.get_nsteps(); ++idx) {
				uint32_t dist = map.calc_distance(get_position(), cur);

				if (dist == 0) {
					molog("Follow pre-computed path from (%i,%i)  [idx = %u]\n", cur.x, cur.y, idx);

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

				if (idx == closest_idx + closest_dist)
					closest_target = cur;

				if (idx < path.get_nsteps())
					map.get_neighbour(cur, path[idx], &cur);
			}

			if (closest_target) {
				molog("Closest target en route is (%i,%i)\n", closest_target.x, closest_target.y);
				if (start_task_movepath(game, closest_target, 0, descr().get_sail_anims()))
					return true;

				molog("  Failed to find path!!! Retry full search\n");
			}
		}

		m_lastdock = nullptr;
	}

	start_task_movetodock(game, *dst);
	return true;
}


/// updates a ships tasks in expedition mode
void Ship::ship_update_expedition(Game & game, Bob::State &) {
	Map & map = game.map();

	assert(m_expedition);

	// Update the knowledge of the surrounding fields
	FCoords position = get_position();
	for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
		m_expedition->swimable[dir - 1] =
			map.get_neighbour(position, dir).field->nodecaps() & MOVECAPS_SWIM;
	}

	if (m_ship_state == EXP_SCOUTING) {
		// Check surrounding fields for port buildspaces
		std::unique_ptr<std::list<Coords> > temp_port_buildspaces(new std::list<Coords>());
		MapRegion<Area<Coords> > mr(map, Area<Coords>(position, vision_range()));
		bool new_port_space = false;
		do {
			if (map.is_port_space(mr.location())) {
				FCoords fc = map.get_fcoords(mr.location());

				// Check whether the maximum theoretical possible NodeCap of the field is of the size big
				// and whether it can theoretically be a port space
				if
					((map.get_max_nodecaps(fc) & BUILDCAPS_SIZEMASK) != BUILDCAPS_BIG
					 ||
					 map.find_portdock(fc).empty())
				{
					continue;
				}

				// NOTE This is the place to handle enemy territory and "clearing a port space from the enemy".
				// NOTE There is a simple check for the current land owner to avoid placement of ports into enemy
				// NOTE territory, as "clearing" is not yet implemented.
				// NOTE further it checks, whether there is a Player_immovable on one of the fields.
				// FIXME handle this more gracefully concering opposing players
				Player_Number pn = get_owner()->player_number();
				FCoords coord = fc;
				bool invalid = false;
				for (uint8_t step = 0; !invalid && step < 5; ++step) {
					if (coord.field->get_owned_by() != Neutral() && coord.field->get_owned_by() != pn) {
						invalid = true;
						continue;
					}
					BaseImmovable * baim = coord.field->get_immovable();
					if (baim)
						if (is_a(PlayerImmovable, baim)) {
							invalid = true;
							continue;
						}

					// Check all neighboured fields that will be used by the port
					switch (step) {
						case 0:
							map.get_ln(fc, &coord);
							break;
						case 1:
							map.get_tln(fc, &coord);
							break;
						case 2:
							map.get_trn(fc, &coord);
							break;
						case 3:
							// Flag coordinate
							map.get_brn(fc, &coord);
							break;
						default:
							break;
					}
				}
				// Now check whether there is a flag in the surroundings of the flag position
				FCoords neighb;
				map.get_ln(coord, &neighb);
				for (uint8_t step = 0; !invalid && step < 5; ++step) {
					BaseImmovable * baim = neighb.field->get_immovable();
					if (baim)
						if (is_a(Flag, baim)) {
							invalid = true;
							continue;
						}
					// Check all neighboured fields but not the one already checked for a PlayerImmovable.
					switch (step) {
						case 0:
							map.get_bln(coord, &neighb);
							break;
						case 1:
							map.get_brn(coord, &neighb);
							break;
						case 2:
							map.get_rn(coord, &neighb);
							break;
						case 3:
							map.get_trn(coord, &neighb);
							break;
						default:
							break;
					}
				}
				if (invalid)
					continue;

				bool pbs_saved = false;
				for
					(std::list<Coords>::const_iterator it = m_expedition->seen_port_buildspaces->begin();
					 it != m_expedition->seen_port_buildspaces->end() && !pbs_saved;
					 ++it)
				{
					// Check if the ship knows this port space already from its last check
					if (*it == mr.location()) {
						temp_port_buildspaces->push_back(mr.location());
						pbs_saved = true;
					}
				}
				if (!pbs_saved) {
					new_port_space = true;
					temp_port_buildspaces->push_front(mr.location());
				}
			}
		} while (mr.advance(map));

		if (new_port_space) {
			m_ship_state = EXP_FOUNDPORTSPACE;
			// Send a message to the player, that a new port space was found
			std::string msg_head = _("Port Space Found");
			std::string msg_body = _("An expedition ship found a new port build space.");
			send_message(game, "exp_port_space", msg_head, msg_body, "port.png");
		}
		m_expedition->seen_port_buildspaces.swap(temp_port_buildspaces);
	}
}

void Ship::ship_update_idle(Game & game, Bob::State & state) {
	if (state.ivar1) {
		// We've just completed one step, so give neighbours
		// a chance to move away first
		wakeup_neighbours(game);
		state.ivar1 = 0;
		schedule_act(game, 25);
		return;
	}

	// If we are waiting for the next transport job, check if we should move away from ships and shores
	switch (m_ship_state) {
		case TRANSPORT: {
			FCoords position = get_position();
			Map & map = game.map();
			unsigned int dirs[LAST_DIRECTION + 1];
			unsigned int dirmax = 0;

			for (Direction dir = 0; dir <= LAST_DIRECTION; ++dir) {
				FCoords node = dir ? map.get_neighbour(position, dir) : position;
				dirs[dir] = node.field->nodecaps() & MOVECAPS_WALK ? 10 : 0;

				Area<FCoords> area(node, 0);
				std::vector<Bob *> ships;
				map.find_bobs(area, &ships, FindBobShip());

				for (std::vector<Bob *>::const_iterator it = ships.begin(); it != ships.end(); ++it) {
					if (*it == this)
						continue;

					dirs[dir] += 3;
				}

				dirmax = std::max(dirmax, dirs[dir]);
			}

			if (dirmax) {
				unsigned int prob[LAST_DIRECTION + 1];
				unsigned int totalprob = 0;

				// The probability for moving into a given direction is also
				// affected by the "close" directions.
				for (Direction dir = 0; dir <= LAST_DIRECTION; ++dir) {
					prob[dir] = 10 * dirmax - 10 * dirs[dir];

					if (dir > 0) {
						unsigned int delta = std::min(prob[dir], dirs[(dir % 6) + 1] + dirs[1 + ((dir - 1) % 6)]);
						prob[dir] -= delta;
					}

					totalprob += prob[dir];
				}

				if (totalprob == 0) {
					start_task_idle(game, descr().main_animation(), 1500);
					return;
				}

				unsigned int rnd = game.logic_rand() % totalprob;
				Direction dir = 0;
				while (rnd >= prob[dir]) {
					rnd -= prob[dir];
					++dir;
				}

				if (dir == 0 || dir > LAST_DIRECTION) {
					start_task_idle(game, descr().main_animation(), 1500);
					return;
				}

				FCoords neighbour = map.get_neighbour(position, dir);
				if (!(neighbour.field->nodecaps() & MOVECAPS_SWIM)) {
					start_task_idle(game, descr().main_animation(), 1500);
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
		case EXP_SCOUTING: {
			if (m_expedition->island_exploration) { // Exploration of the island
				if (exp_close_to_coast()) {
					if (m_expedition->direction == 0) {
						// Make sure we know the location of the coast and use it as initial direction we come from
						m_expedition->direction = WALK_SE;
						for (uint8_t secure = 0; exp_dir_swimable(m_expedition->direction); ++secure) {
							assert(secure < 6);
							m_expedition->direction = get_cw_neighbour(m_expedition->direction);
						}
						m_expedition->direction = get_backward_dir(m_expedition->direction);
						// Save the position - this is where we start
						m_expedition->exploration_start = get_position();
					} else {
						// Check whether the island was completely surrounded
						if (get_position() == m_expedition->exploration_start) {
							std::string msg_head = _("Island Circumnavigated");
							std::string msg_body = _("An expedition ship sailed around its"
										 " island without any events.");
							send_message(game, "exp_island", msg_head, msg_body,
								"ship_explore_island_cw.png");
							m_ship_state = EXP_WAITING;
							return start_task_idle(game, descr().main_animation(), 1500);
						}
					}
					// The ship is supposed to follow the coast as close as possible, therefore the check for
					// a swimable field begins at the neighbour field of the direction we came from.
					m_expedition->direction = get_backward_dir(m_expedition->direction);
					if (m_expedition->clockwise) {
						do {
							m_expedition->direction = get_ccw_neighbour(m_expedition->direction);
						} while (!exp_dir_swimable(m_expedition->direction));
					} else {
						do {
							m_expedition->direction = get_cw_neighbour(m_expedition->direction);
						} while (!exp_dir_swimable(m_expedition->direction));
					}
					state.ivar1 = 1;
					return start_task_move(game, m_expedition->direction, descr().get_sail_anims(), false);
				} else {
					// The ship got the command to scout around an island, but is not close to any island
					// Most likely the command was send as the ship was on an exploration and just leaving
					// the island - therefore we try to find the island again.
					FCoords position = get_position();
					Map & map = game.map();
					for (uint8_t dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
						FCoords neighbour = map.get_neighbour(position, dir);
						for (uint8_t sur = FIRST_DIRECTION; sur <= LAST_DIRECTION; ++sur)
							if (!(map.get_neighbour(neighbour, sur).field->nodecaps() & MOVECAPS_SWIM)) {
								// Okay we found the next coast, so now the ship should go there.
								// However, we do neither save the position as starting position, nor do we save
								// the direction we currently go. So the ship can start exploring normally
								state.ivar1 = 1;
								return start_task_move(game, dir, descr().get_sail_anims(), false);
							}
					}
					// if we are here, it seems something really strange happend.
					log("WARNING: ship was not able to start exploration. Entering WAIT mode.");
					m_ship_state = EXP_WAITING;
					return start_task_idle(game, descr().main_animation(), 1500);
				}
			} else { // scouting towards a specific direction
				if (exp_dir_swimable(m_expedition->direction)) {
					// the scouting direction is still free to move
					state.ivar1 = 1;
					start_task_move(game, m_expedition->direction, descr().get_sail_anims(), false);
					return;
				} else { // coast reached
					m_ship_state = EXP_WAITING;
					start_task_idle(game, descr().main_animation(), 1500);
					// Send a message to the player, that a new coast was reached
					std::string msg_head = _("Coast Reached");
					std::string msg_body =
						_("An expedition ship reached a coast and is waiting for further commands.");
					send_message(game, "exp_coast", msg_head, msg_body, "ship_explore_island_cw.png");
					return;
				}
			}
			break;
		}
		case EXP_COLONIZING: {
			assert(m_expedition->seen_port_buildspaces && !m_expedition->seen_port_buildspaces->empty());
			BaseImmovable * baim = game.map()[m_expedition->seen_port_buildspaces->front()].get_immovable();
			assert(baim);
			upcast(ConstructionSite, cs, baim);

			for (int i = m_items.size() - 1; i >= 0; --i) {
				WareInstance * ware;
				Worker * worker;
				m_items.at(i).get(game, &ware, &worker);
				if (ware) {
					// no, we don't transfer the wares, we create new ones out of air and remove the old ones ;)
					WaresQueue & wq = cs->waresqueue(ware->descr_index());
					const uint32_t max = wq.get_max_fill();
					const uint32_t cur = wq.get_filled();
					assert(max > cur);
					wq.set_filled(cur + 1);
					m_items.at(i).remove(game);
					m_items.resize(i);
					break;
				} else {
					assert(worker);
					worker->set_economy(nullptr);
					worker->set_location(cs);
					worker->set_position(game, cs->get_position());
					worker->reset_tasks(game);
					Partially_Finished_Building::request_builder_callback
						(game, *cs->get_builder_request(), worker->worker_index(), worker, *cs);
					m_items.resize(i);
				}
			}
			if (m_items.empty()) {
				m_ship_state = TRANSPORT; // That's it, expedition finished

				init_fleet(game);
				m_expedition.reset(nullptr);

				if (upcast(Interactive_GameBase, igb, game.get_ibase()))
					refresh_window(*igb);
			}
			return start_task_idle(game, descr().main_animation(), 1500); // unload the next item
		}

		default: {
			// wait for input
			start_task_idle(game, descr().main_animation(), 1500);
			return;
		}
	}

	// never here
	assert (false);
}

void Ship::set_economy(Game & game, Economy * e) {
	// Do not check here that the economy actually changed, because on loading
	// we rely that wares really get reassigned our economy.

	m_economy = e;
	container_iterate(std::vector<ShippingItem>, m_items, it) {
		it->set_economy(game, e);
	}
}

/**
 * Enter a new destination port for the ship.
 *
 * @note This is supposed to be called only from the scheduling code of @ref Fleet.
 */
void Ship::set_destination(Game & game, PortDock & pd) {
	molog("set_destination to %u (currently %" PRIuS " items)\n", pd.serial(), m_items.size());
	m_destination = &pd;
	send_signal(game, "wakeup");
}

void Ship::add_item(Game & game, const ShippingItem & item) {
	assert(m_items.size() < get_capacity());

	m_items.push_back(item);
	m_items.back().set_location(game, this);
}

void Ship::withdraw_items(Game & game, PortDock & pd, std::vector<ShippingItem> & items) {
	uint32_t dst = 0;
	for (uint32_t src = 0; src < m_items.size(); ++src) {
		PortDock * destination = m_items[src].get_destination(game);
		if (!destination || destination == &pd) {
			items.push_back(m_items[src]);
		} else {
			m_items[dst++] = m_items[src];
		}
	}
	m_items.resize(dst);
}

/**
 * Find a path to the dock @p pd and follow it without using precomputed paths.
 */
void Ship::start_task_movetodock(Game & game, PortDock & pd) {
	Map & map = game.map();
	StepEvalAStar se(pd.get_warehouse()->get_position());
	se.m_swim = true;
	se.m_conservative = false;
	se.m_estimator_bias = -5 * map.calc_cost(0);

	MapAStar<StepEvalAStar> astar(map, se);

	astar.push(get_position());

	int32_t cost;
	FCoords cur;
	while (astar.step(cur, cost)) {
		if (cur.field->get_immovable() == &pd) {
			Path path;
			astar.pathto(cur, path);
			start_task_movepath(game, path, descr().get_sail_anims());
			return;
		}
	}

	molog("start_task_movedock: Failed to find path!\n");
	start_task_idle(game, descr().main_animation(), 5000);
}

/// Prepare everything for the coming exploration
void Ship::start_task_expedition(Game & game) {
	// Now we are waiting
	m_ship_state = EXP_WAITING;
	// Initialize a new, yet empty expedition
	m_expedition.reset(new Expedition());
	m_expedition->seen_port_buildspaces.reset(new std::list<Coords>());
	m_expedition->island_exploration = false;
	m_expedition->direction = 0;
	m_expedition->exploration_start = Coords(0, 0);
	m_expedition->clockwise = false;
	m_expedition->economy.reset(new Economy(*get_owner()));

	// We are no longer in any other economy, but instead are an economy of our
	// own.
	m_fleet->remove_ship(game, this);
	assert(m_fleet == nullptr);

	set_economy(game, m_expedition->economy.get());

	for (int i = m_items.size() - 1; i >= 0; --i) {
		WareInstance * ware;
		Worker * worker;
		m_items.at(i).get(game, &ware, &worker);
		if (worker) {
			worker->reset_tasks(game);
			worker->start_task_idle(game, 0, -1);
		} else {
			assert(ware);
		}
	}

	// Send a message to the player, that an expedition is ready to go
	const std::string msg_head = _("Expedition Ready");
	const std::string msg_body = _("An expedition ship is waiting for your commands.");
	send_message(game, "exp_ready", msg_head, msg_body, "start_expedition.png");
}

/// Initializes / changes the direction of scouting to @arg direction
/// @note only called via player command
void Ship::exp_scout_direction(Game &, uint8_t direction) {
	assert(m_expedition);
	m_ship_state = EXP_SCOUTING;
	m_expedition->direction = direction;
	m_expedition->island_exploration = false;
}

/// Initializes the construction of a port at @arg c
/// @note only called via player command
void Ship::exp_construct_port (Game &, const Coords& c) {
	assert(m_expedition);
	Building_Index port_idx = get_owner()->tribe().safe_building_index("port");
	get_owner()->force_csite(c, port_idx);
	m_ship_state = EXP_COLONIZING;
}

/// Initializes / changes the direction the island exploration in @arg clockwise direction
/// @note only called via player command
void Ship::exp_explore_island (Game &, bool clockwise) {
	assert(m_expedition);
	m_ship_state = EXP_SCOUTING;
	m_expedition->clockwise = clockwise;
	m_expedition->direction = 0;
	m_expedition->island_exploration = true;
}

/// Cancels a currently running expedition
/// @note only called via player command
void Ship::exp_cancel (Game & game) {
	// Running colonization has the highest priority before cancelation
	// + cancelation only works if an expedition is actually running
	if ((m_ship_state == EXP_COLONIZING) || !state_is_expedition())
		return;
	send_signal(game, "cancel_expedition");

	// The workers were hold in an idle state so that they did not try
	// to become fugitive or run to the next warehouse. But now, we
	// have a proper destination, so we can just inform them that they
	// are now getting shipped there.
	// Theres nothing to be done for wares - they already changed
	// economy with us and the warehouse will make sure that they are
	// getting used.
	Worker * worker;
	BOOST_FOREACH(ShippingItem& item, m_items) {
		item.get(game, nullptr, &worker);
		if (worker) {
			worker->reset_tasks(game);
			worker->start_task_shipping(game, nullptr);
		}
	}
	m_ship_state = TRANSPORT;

	// Bring us back into a fleet and a economy.
	set_economy(game, nullptr);
	init_fleet(game);
	assert(get_economy() && get_economy() != m_expedition->economy.get());

	// Delete the expedition and the economy it created.
	m_expedition.reset(nullptr);

	// And finally update our ship window
	if (upcast(Interactive_GameBase, igb, game.get_ibase()))
		refresh_window(*igb);
}

/// Sinks the ship
/// @note only called via player command
void Ship::sink_ship (Game & game) {
	// Running colonization has the highest priority + a sink request is only valid once
	if (!state_is_sinkable())
		return;
	m_ship_state = SINK_REQUEST;
	// Make sure the ship is active and close possible open windows
	ship_wakeup(game);
	close_window();
}

void Ship::log_general_info(const Editor_Game_Base & egbase)
{
	Bob::log_general_info(egbase);

	molog
		("Fleet: %u, destination: %u, lastdock: %u, carrying: %" PRIuS "\n",
		 m_fleet? m_fleet->serial() : 0,
		 m_destination.serial(), m_lastdock.serial(),
		 m_items.size());

	container_iterate(std::vector<ShippingItem>, m_items, it) {
		molog
			("  IT %u, destination %u\n",
			 it.current->m_object.serial(),
			 it.current->m_destination_dock.serial());
	}
}


/**
 * Send a message to the owning player.
 *
 * It will have the ship's coordinates, and display a picture in its description.
 *
 * \param msgsender a computer-readable description of why the message was sent
 * \param title user-visible title of the message
 * \param description user-visible message body, will be placed in an appropriate rich-text paragraph
 * \param picture picture name relative to the pics directory
 */
void Ship::send_message
	(Game & game, const std::string & msgsender,
	 const std::string & title, const std::string & description,
	 const std::string & picture)
{
	std::string rt_description;
	if (picture.size() > 3) {
		rt_description  = "<rt image=pics/";
		rt_description += picture;
		rt_description += "><p font-size=14 font-face=DejaVuSerif>";
	} else
		rt_description  = "<rt><p font-size=14 font-face=DejaVuSerif>";
	rt_description += description;
	rt_description += "</p></rt>";

	Message * msg = new Message
		(msgsender, game.get_gametime(), 60 * 60 * 1000,
		 title, rt_description, get_position(), m_serial);

	get_owner()->add_message(game, *msg);
}


/*
==============================

Load / Save implementation

==============================
*/

#define SHIP_SAVEGAME_VERSION 4

Ship::Loader::Loader() :
	m_lastdock(0),
	m_destination(0)
{
}

const Bob::Task * Ship::Loader::get_task(const std::string & name)
{
	if (name == "shipidle" || name == "ship") return &taskShip;
	return Bob::Loader::get_task(name);
}

void Ship::Loader::load(FileRead & fr, uint8_t version)
{
	Bob::Loader::load(fr);

	if (version >= 2) {
		// The state the ship is in
		if (version >= 3) {
			m_ship_state = fr.Unsigned8();

			// Expedition specific data
			if
				(m_ship_state == EXP_SCOUTING
				 ||
				 m_ship_state == EXP_WAITING
				 ||
				 m_ship_state == EXP_FOUNDPORTSPACE
				 ||
				 m_ship_state == EXP_COLONIZING)
			{
				m_expedition.reset(new Expedition());
				// Currently seen port build spaces
				m_expedition->seen_port_buildspaces.reset(new std::list<Coords>());
				uint8_t numofports = fr.Unsigned8();
				for (uint8_t i = 0; i < numofports; ++i)
					m_expedition->seen_port_buildspaces->push_back(fr.Coords32());
				// Swimability of the directions
				for (uint8_t i = 0; i < LAST_DIRECTION; ++i)
					m_expedition->swimable[i] = (fr.Unsigned8() == 1);
				// whether scouting or exploring
				m_expedition->island_exploration = fr.Unsigned8() == 1;
				// current direction
				m_expedition->direction = fr.Unsigned8();
				// Start coordinates of an island exploration
				m_expedition->exploration_start = fr.Coords32();
				// Whether the exploration is done clockwise or counter clockwise
				m_expedition->clockwise = fr.Unsigned8() == 1;
			}
		} else
			m_ship_state = TRANSPORT;

		m_lastdock = fr.Unsigned32();
		m_destination = fr.Unsigned32();

		m_items.resize(fr.Unsigned32());
		container_iterate(std::vector<ShippingItem::Loader>, m_items, it) {
			it->load(fr);
		}
	}
}

void Ship::Loader::load_pointers()
{
	Bob::Loader::load_pointers();

	Ship & ship = get<Ship>();

	if (m_lastdock)
		ship.m_lastdock = &mol().get<PortDock>(m_lastdock);
	if (m_destination)
		ship.m_destination = &mol().get<PortDock>(m_destination);

	ship.m_items.resize(m_items.size());
	for (uint32_t i = 0; i < m_items.size(); ++i) {
		ship.m_items[i] = m_items[i].get(mol());
	}
}

void Ship::Loader::load_finish()
{
	Bob::Loader::load_finish();

	Ship & ship = get<Ship>();

	// restore the state the ship is in
	ship.m_ship_state = m_ship_state;

	// if the ship is on an expedition, restore the expedition specific data
	if (m_expedition) {
		ship.m_expedition.swap(m_expedition);
		ship.m_expedition->economy.reset(new Economy(*ship.get_owner()));
		ship.m_economy = ship.m_expedition->economy.get();
	} else assert(m_ship_state == TRANSPORT);

	// Workers load code set their economy to the economy of their location
	// (which is a PlayerImmovable), that means that workers on ships do not get
	// a correct economy assigned. We, as ship therefore have to reset the
	// economy of all workers we're transporting so that they are in the correct
	// economy. Also, we might are on an expedition which means that we just now
	// created the economy of this ship and must inform all wares.
	ship.set_economy(ref_cast<Game>(egbase()), ship.m_economy);
}


Map_Object::Loader * Ship::load
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const version = fr.Unsigned8();
		if (1 <= version && version <= SHIP_SAVEGAME_VERSION) {
			std::string owner = fr.CString();
			std::string name = fr.CString();
			const Ship_Descr * descr = nullptr;

			egbase.manually_load_tribe(owner);

			if (const Tribe_Descr * tribe = egbase.get_tribe(owner))
				descr = dynamic_cast<const Ship_Descr *>
					(tribe->get_bob_descr(name));

			if (!descr)
				throw game_data_error
					("undefined ship %s/%s", owner.c_str(), name.c_str());

			loader->init(egbase, mol, descr->create_object());
			loader->load(fr, version);
		} else
			throw game_data_error("unknown/unhandled version %u", version);
	} catch (const std::exception & e) {
		throw wexception("loading ship: %s", e.what());
	}

	return loader.release();
}

void Ship::save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_Ship);
	fw.Unsigned8(SHIP_SAVEGAME_VERSION);

	fw.CString(descr().get_owner_tribe()->name());
	fw.CString(descr().name());

	Bob::save(egbase, mos, fw);

	// state the ship is in
	fw.Unsigned8(m_ship_state);

	// expedition specific data
	if (state_is_expedition()) {
		// currently seen port buildspaces
		assert(m_expedition->seen_port_buildspaces);
		fw.Unsigned8(m_expedition->seen_port_buildspaces->size());
		for
			(std::list<Coords>::const_iterator it = m_expedition->seen_port_buildspaces->begin();
			 it != m_expedition->seen_port_buildspaces->end();
			 ++it)
		{
			fw.Coords32(*it);
		}
		// swimability of the directions
		for (uint8_t i = 0; i < LAST_DIRECTION; ++i)
			fw.Unsigned8(m_expedition->swimable[i] ? 1 : 0);
		// whether scouting or exploring
		fw.Unsigned8(m_expedition->island_exploration ? 1 : 0);
		// current direction
		fw.Unsigned8(m_expedition->direction);
		// Start coordinates of an island exploration
		fw.Coords32(m_expedition->exploration_start);
		// Whether the exploration is done clockwise or counter clockwise
		fw.Unsigned8(m_expedition->clockwise ? 1 : 0);
	}

	fw.Unsigned32(mos.get_object_file_index_or_zero(m_lastdock.get(egbase)));
	fw.Unsigned32(mos.get_object_file_index_or_zero(m_destination.get(egbase)));

	fw.Unsigned32(m_items.size());
	container_iterate(std::vector<ShippingItem>, m_items, it) {
		it->save(egbase, mos, fw);
	}
}

} // namespace Widelands
