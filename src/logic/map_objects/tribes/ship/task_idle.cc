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

#include "logic/map_objects/findbob.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"

namespace Widelands {

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
		if (expedition_->scouting_direction != IDLE &&
		    exp_dir_swimmable(expedition_->scouting_direction)) {
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
		const Coords portspace = current_portspace();
		assert(portspace.valid());
		upcast(ConstructionSite, cs, map[portspace].get_immovable());
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
						   &cs->inputqueue(ware->descr_index(), wwWARE, nullptr, 0));
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
	default:
		NEVER_HERE();
	}

	NEVER_HERE();
}

}  // namespace Widelands
