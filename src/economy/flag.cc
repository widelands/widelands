/*
 * Copyright (C) 2004, 2006-2010 by the Widelands Development Team
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

#include "economy/flag.h"

#include "container_iterate.h"
#include "economy/economy.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/road.h"
#include "economy/ware_instance.h"
#include "logic/building.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/instances.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "logic/worker.h"
#include "upcast.h"
#include "wexception.h"

namespace Widelands {

Map_Object_Descr g_flag_descr("flag", "Flag");

/**
 * Create the flag. Initially, it doesn't have any attachments.
*/
Flag::Flag() :
PlayerImmovable(g_flag_descr),
m_animstart(0),
m_building(nullptr),
m_ware_capacity(8),
m_ware_filled(0),
m_wares(new PendingWare[m_ware_capacity]),
m_always_call_for_flag(nullptr)
{
	for (uint32_t i = 0; i < 6; ++i) m_roads[i] = nullptr;
}

/**
 * Shouldn't be necessary to do anything, since die() always calls
 * cleanup() first.
*/
Flag::~Flag()
{
	if (m_ware_filled)
		log("Flag: ouch! wares left\n");
	delete[] m_wares;

	if (m_building)
		log("Flag: ouch! building left\n");

	if (m_flag_jobs.size())
		log("Flag: ouch! flagjobs left\n");

	for (int32_t i = 0; i < 6; ++i)
		if (m_roads[i])
			log("Flag: ouch! road left\n");
}

void Flag::load_finish(Editor_Game_Base & egbase) {
	CapacityWaitQueue & queue = m_capacity_wait;
	for (wl_range<CapacityWaitQueue > r(queue); r;)
	{
		Worker & worker = *r->get(egbase);
		Bob::State const * const state =
			worker.get_state(Worker::taskWaitforcapacity);
		if (not state)
			log
				("WARNING: worker %u is in the capacity wait queue of flag %u but "
				 "does not have a waitforcapacity task! Removing from queue.\n",
				 worker.serial(), serial());
		else if (state->objvar1 != this)
			log
				("WARNING: worker %u is in the capacity wait queue of flag %u but "
				 "its waitforcapacity task is for map object %u! Removing from "
				 "queue.\n",
				 worker.serial(), serial(), state->objvar1.serial());
		else {
			++r;
			continue;
		}
		r = wl_erase(queue, r.current);
	}
}

/**
 * Create a flag at the given location
*/
Flag::Flag
	(Editor_Game_Base & egbase, Player & owning_player, Coords const coords)
	:
	PlayerImmovable       (g_flag_descr),
	m_building            (nullptr),
	m_ware_capacity       (8),
	m_ware_filled         (0),
	m_wares               (new PendingWare[m_ware_capacity]),
	m_always_call_for_flag(nullptr)
{
	for (uint32_t i = 0; i < 6; ++i) m_roads[i] = nullptr;

	set_owner(&owning_player);

	set_flag_position(coords);


	upcast(Road, road, egbase.map().get_immovable(coords));
	upcast(Game, game, &egbase);

	if (game) {
		//  we split a road, or a new, standalone flag is created
		(road ? road->get_economy() : new Economy (owning_player))
				->add_flag(*this);

		if (road)
			road->presplit(*game, coords);
	}

	init(egbase);

	if (road and game)
			road->postsplit(*game, *this);
}

void Flag::set_flag_position(Coords coords) {
	m_position = coords;
}

int32_t Flag::get_type() const
{
	return FLAG;
}

int32_t Flag::get_size() const
{
	return SMALL;
}

bool Flag::get_passable() const
{
	return true;
}


static std::string const flag_name = "flag";
const std::string & Flag::name() const {return flag_name;}


Flag & Flag::base_flag()
{
	return *this;
}

/**
 * Call this only from Economy code!
*/
void Flag::set_economy(Economy * const e)
{
	Economy * const old = get_economy();

	if (old == e)
		return;

	PlayerImmovable::set_economy(e);

	for (int32_t i = 0; i < m_ware_filled; ++i)
		m_wares[i].ware->set_economy(e);

	if (m_building)
		m_building->set_economy(e);

	container_iterate_const(FlagJobs, m_flag_jobs, i)
		i.current->request->set_economy(e);

	for (int8_t i = 0; i < 6; ++i) {
		if (m_roads[i])
			m_roads[i]->set_economy(e);
	}
}

/**
 * Call this only from the Building init!
*/
void Flag::attach_building(Editor_Game_Base & egbase, Building & building)
{
	assert(!m_building || m_building == &building);

	m_building = &building;

	const Map & map = egbase.map();
	egbase.set_road
		(map.get_fcoords(map.tl_n(m_position)), Road_SouthEast, Road_Normal);

	building.set_economy(get_economy());
}

/**
 * Call this only from the Building cleanup!
*/
void Flag::detach_building(Editor_Game_Base & egbase)
{
	assert(m_building);

	m_building->set_economy(nullptr);

	const Map & map = egbase.map();
	egbase.set_road
		(map.get_fcoords(map.tl_n(m_position)), Road_SouthEast, Road_None);

	m_building = nullptr;
}

/**
 * Call this only from the Road init!
*/
void Flag::attach_road(int32_t const dir, Road * const road)
{
	assert(!m_roads[dir - 1] || m_roads[dir - 1] == road);

	m_roads[dir - 1] = road;
	m_roads[dir - 1]->set_economy(get_economy());
}

/**
 * Call this only from the Road init!
*/
void Flag::detach_road(int32_t const dir)
{
	assert(m_roads[dir - 1]);

	m_roads[dir - 1]->set_economy(nullptr);
	m_roads[dir - 1] = nullptr;
}

/**
 * Return all positions we occupy on the map. For a Flag, this is only one
*/
BaseImmovable::PositionList Flag::get_positions
	(const Editor_Game_Base &) const
{
	PositionList rv;
	rv.push_back(m_position);
	return rv;
}

/**
 * Return neighbouring flags.
*/
void Flag::get_neighbours(WareWorker type, RoutingNodeNeighbours & neighbours)
{
	for (int8_t i = 0; i < 6; ++i) {
		Road * const road = m_roads[i];
		if (!road)
			continue;

		Flag * f = &road->get_flag(Road::FlagEnd);
		int32_t nb_cost;
		if (f != this) {
			nb_cost = road->get_cost(Road::FlagStart);
		} else {
			f = &road->get_flag(Road::FlagStart);
			nb_cost = road->get_cost(Road::FlagEnd);
		}
		if (type == wwWARE) {
			nb_cost += nb_cost * (get_waitcost() + f->get_waitcost()) / 2;
		}
		RoutingNodeNeighbour n(f, nb_cost);

		assert(n.get_neighbour() != this);
		neighbours.push_back(n);
	}

	if (m_building && m_building->descr().get_isport()) {
		Warehouse * wh = static_cast<Warehouse *>(m_building);
		if (PortDock * pd = wh->get_portdock()) {
			pd->add_neighbours(neighbours);
		}
	}
}

/**
 * Return the road that leads to the given flag.
*/
Road * Flag::get_road(Flag & flag)
{
	for (int8_t i = 0; i < 6; ++i)
		if (Road * const road = m_roads[i])
			if
				(&road->get_flag(Road::FlagStart) == &flag ||
				 &road->get_flag(Road::FlagEnd)   == &flag)
				return road;

	return nullptr;
}


/// returns the number of roads connected to the flag
uint8_t Flag::nr_of_roads() const {
	uint8_t counter = 0;
	for (uint8_t road_id = 6; road_id; --road_id)
		if (get_road(road_id) != nullptr)
			++counter;
	return counter;
}


bool Flag::is_dead_end() const {
	if (get_building())
		return false;
	Flag const * first_other_flag = nullptr;
	for (uint8_t road_id = 6; road_id; --road_id)
		if (Road * const road = get_road(road_id)) {
			Flag & start = road->get_flag(Road::FlagStart);
			Flag & other = this == &start ? road->get_flag(Road::FlagEnd) : start;
			if (first_other_flag) {
				if (&other != first_other_flag)
					return false;
			} else
				first_other_flag = &other;
		}
	return true;
}


/**
 * Returns true if the flag can hold more wares.
*/
bool Flag::has_capacity() const
{
	return (m_ware_filled < m_ware_capacity);
}

/**
 * Signal the given bob by interrupting its task as soon as capacity becomes
 * free.
 *
 * The capacity queue is a simple FIFO queue.
 */
void Flag::wait_for_capacity(Game &, Worker & bob)
{
	m_capacity_wait.push_back(&bob);
}

/**
 * Remove the worker from the list of workers waiting for free capacity.
 */
void Flag::skip_wait_for_capacity(Game &, Worker & w)
{
	CapacityWaitQueue::iterator const it =
		std::find(m_capacity_wait.begin(), m_capacity_wait.end(), &w);
	if (it != m_capacity_wait.end())
		m_capacity_wait.erase(it);
}


void Flag::add_ware(Editor_Game_Base & egbase, WareInstance & ware)
{

	assert(m_ware_filled < m_ware_capacity);

	PendingWare & pi = m_wares[m_ware_filled++];
	pi.ware     = &ware;
	pi.pending  = false;
	pi.nextstep = nullptr;
	pi.priority = 0;

	Transfer * trans = ware.get_transfer();
	if (trans) {
		uint32_t trans_steps = trans->get_steps_left();
		if (trans_steps < 3)
			pi.priority = 2;
		else if (trans_steps == 3)
			pi.priority = 1;

		Request * req = trans->get_request();
		if (req)
			pi.priority = pi.priority + req->get_transfer_priority();
	}

	ware.set_location(egbase, this);

	if (upcast(Game, game, &egbase))
		ware.update(*game); //  will call call_carrier() if necessary
}

/**
 * \return true if an ware is currently waiting for a carrier to the given Flag.
 *
 * \note Due to fetch_from_flag() semantics, this function makes no sense
 * for a  building destination.
*/
bool Flag::has_pending_ware(Game &, Flag & dest) {
	for (int32_t i = 0; i < m_ware_filled; ++i) {
		if (!m_wares[i].pending)
			continue;

		if (m_wares[i].nextstep != &dest)
			continue;

		return true;
	}

	return false;
}

/**
 * Clamp the maximal value of \ref PendingWare::priority.
 * After reaching this value, the pure FIFO approach is applied
 */
#define MAX_TRANSFER_PRIORITY  16


/**
 * Called by carrier code to indicate that the carrier is moving to pick up an
 * ware. Ware with highest transfer priority is chosen.
 * \return true if an ware is actually waiting for the carrier.
 */
bool Flag::ack_pickup(Game &, Flag & destflag) {
	int32_t highest_pri = -1;
	int32_t i_pri = -1;

	for (int32_t i = 0; i < m_ware_filled; ++i) {
		if (!m_wares[i].pending)
			continue;

		if (m_wares[i].nextstep != &destflag)
			continue;

		if (m_wares[i].priority > highest_pri) {
			highest_pri = m_wares[i].priority;
			i_pri = i;

			// Increase ware priority, it matters only if the ware has to wait.
			if (m_wares[i].priority < MAX_TRANSFER_PRIORITY)
				m_wares[i].priority++;
		}
	}

	if (i_pri >= 0) {
		m_wares[i_pri].pending = false;
		return true;
	}

	return false;
}
/**
 * Called by the carriercode when the carrier is called away from his job
 * but has acknowledged a ware before. This ware is then freed again
 * to be picked by another carrier. Returns true if an ware was indeed
 * made pending again
 */
bool Flag::cancel_pickup(Game & game, Flag & destflag) {
	int32_t lowest_prio = MAX_TRANSFER_PRIORITY + 1;
	int32_t i_pri = -1;

	for (int32_t i = 0; i < m_ware_filled; ++i) {
		if (m_wares[i].pending)
			continue;

		if (m_wares[i].nextstep != &destflag)
			continue;

		if (m_wares[i].priority < lowest_prio) {
			lowest_prio = m_wares[i].priority;
			i_pri = i;
		}
	}

	if (i_pri >= 0) {
		m_wares[i_pri].pending = true;
		m_wares[i_pri].ware->update(game); //  will call call_carrier() if necessary
		return true;
	}

	return false;
}

/**
 * Wake one sleeper from the capacity queue.
*/
void Flag::wake_up_capacity_queue(Game & game)
{
	while (!m_capacity_wait.empty()) {
		Worker * const w = m_capacity_wait[0].get(game);
		m_capacity_wait.erase(m_capacity_wait.begin());
		if (w and w->wakeup_flag_capacity(game, *this))
			break;
	}
}

/**
 * Called by carrier code to retrieve one of the wares on the flag that is meant
 * for that carrier.
 *
 * This function may return 0 even if \ref ack_pickup() has already been
 * called successfully.
*/
WareInstance * Flag::fetch_pending_ware(Game & game, PlayerImmovable & dest)
{
	int32_t best_index = -1;

	for (int32_t i = 0; i < m_ware_filled; ++i) {
		if (m_wares[i].nextstep != &dest)
			continue;

		// We prefer to retrieve wares that have already been acked
		if (best_index < 0 || !m_wares[i].pending)
			best_index = i;
	}

	if (best_index < 0)
		return nullptr;

	// move the other wares up the list and return this one
	WareInstance * const ware = m_wares[best_index].ware;
	--m_ware_filled;
	memmove
		(&m_wares[best_index], &m_wares[best_index + 1],
		 sizeof(m_wares[0]) * (m_ware_filled - best_index));

	ware->set_location(game, nullptr);

	// wake up capacity wait queue
	wake_up_capacity_queue(game);

	return ware;
}

/**
 * Return a List of all the wares currently on this Flag. Do not rely
 * the result value to stay valid and do not change them
 */
Flag::Wares Flag::get_wares() {
	Wares rv;

	for (int32_t i = 0; i < m_ware_filled; ++i)
		rv.push_back(m_wares[i].ware);

	return rv;
}

/**
 * Force a removal of the given ware from this flag.
 * Called by \ref WareInstance::cleanup()
*/
void Flag::remove_ware(Editor_Game_Base & egbase, WareInstance * const ware)
{
	for (int32_t i = 0; i < m_ware_filled; ++i) {
		if (m_wares[i].ware != ware)
			continue;

		--m_ware_filled;
		memmove
			(&m_wares[i], &m_wares[i + 1],
			 sizeof(m_wares[0]) * (m_ware_filled - i));

		if (upcast(Game, game, &egbase))
			wake_up_capacity_queue(*game);

		return;
	}

	throw wexception
		("MO(%u): Flag::remove_ware: ware %u not on flag",
		 serial(), ware->serial());
}

/**
 * If nextstep is not null, a carrier will be called to move this ware to
 * the given flag or building.
 *
 * If nextstep is null, the internal data will be reset to indicate that the
 * ware isn't going anywhere right now.
 *
 * nextstep is compared with the cached data, and a new carrier is only called
 * if that data hasn't changed.
 *
 * This behaviour is overridden by m_always_call_for_step, which is set by
 * update_wares() to ensure that new carriers are called when roads are
 * split, for example.
*/
void Flag::call_carrier
	(Game & game, WareInstance & ware, PlayerImmovable * const nextstep)
{
	PendingWare * pi = nullptr;
	int32_t i = 0;

	// Find the PendingWare entry
	for (; i < m_ware_filled; ++i) {
		if (m_wares[i].ware != &ware)
			continue;

		pi = &m_wares[i];
		break;
	}

	assert(pi);

	// Deal with the non-moving case quickly
	if (!nextstep) {
		pi->nextstep = nullptr;
		pi->pending = true;
		return;
	}

	// Find out whether we need to do anything
	if (pi->nextstep == nextstep && pi->nextstep != m_always_call_for_flag)
		return; // no update needed

	pi->nextstep = nextstep;
	pi->pending = false;

	// Deal with the building case
	if (nextstep == get_building()) {
		molog
			("Flag::call_carrier(%u): Tell building to fetch this ware\n",
			 ware.serial());

		if (!get_building()->fetch_from_flag(game)) {
			pi->ware->cancel_moving();
			pi->ware->update(game);
		}

		return;
	}

	// Deal with the normal (flag) case
	ref_cast<Flag const, PlayerImmovable const>(*nextstep);

	for (int32_t dir = 1; dir <= 6; ++dir) {
		Road * const road = get_road(dir);
		Flag *       other;
		Road::FlagId flagid;

		if (!road)
			continue;

		if (&road->get_flag(Road::FlagStart) == this) {
			flagid = Road::FlagStart;
			other = &road->get_flag(Road::FlagEnd);
		} else {
			flagid = Road::FlagEnd;
			other = &road->get_flag(Road::FlagStart);
		}

		if (other != nextstep)
			continue;

		// Yes, this is the road we want; inform it
		if (road->notify_ware(game, flagid))
			return;

		// If the road doesn't react to the ware immediately, we try other roads:
		// They might lead to the same flag!
	}

	// Nothing found, just let it be picked up by somebody
	pi->pending = true;
	return;
}

/**
 * Called whenever a road gets broken or split.
 * Make sure all wares on this flag are rerouted if necessary.
 *
 * \note When two roads connect the same two flags, and one of these roads
 * is removed, this might cause the carrier(s) on the other road to
 * move unnecessarily. Fixing this could potentially be very expensive and
 * fragile.
 * A similar thing can happen when a road is split.
*/
void Flag::update_wares(Game & game, Flag * const other)
{
	m_always_call_for_flag = other;

	for (int32_t i = 0; i < m_ware_filled; ++i)
		m_wares[i].ware->update(game);

	m_always_call_for_flag = nullptr;
}

void Flag::init(Editor_Game_Base & egbase)
{
	PlayerImmovable::init(egbase);

	set_position(egbase, m_position);

	m_animstart = egbase.get_gametime();
}

/**
 * Detach building and free roads.
*/
void Flag::cleanup(Editor_Game_Base & egbase)
{
	//molog("Flag::cleanup\n");

	while (!m_flag_jobs.empty()) {
		delete m_flag_jobs.begin()->request;
		m_flag_jobs.erase(m_flag_jobs.begin());
	}

	while (m_ware_filled) {
		WareInstance & ware = *m_wares[--m_ware_filled].ware;

		ware.set_location(egbase, nullptr);
		ware.destroy     (egbase);
	}

	//molog("  wares destroyed\n");

	if (m_building) {
		m_building->remove(egbase); //  immediate death
		assert(!m_building);
	}

	for (int8_t i = 0; i < 6; ++i) {
		if (m_roads[i]) {
			m_roads[i]->remove(egbase); //  immediate death
			assert(!m_roads[i]);
		}
	}

	if (Economy * e = get_economy())
		e->remove_flag(*this);

	unset_position(egbase, m_position);

	//molog("  done\n");

	PlayerImmovable::cleanup(egbase);
}

/**
 * Destroy the building as well.
 *
 * \note This is needed in addition to the call to m_building->remove() in
 * \ref Flag::cleanup(). This function is needed to ensure a fire is created
 * when a player removes a flag.
*/
void Flag::destroy(Editor_Game_Base & egbase)
{
	if (m_building) {
		m_building->destroy(egbase);
		assert(!m_building);
	}

	PlayerImmovable::destroy(egbase);
}

/**
 * Add a new flag job to request the worker with the given ID, and to execute
 * the given program once it's completed.
*/
void Flag::add_flag_job
	(Game &, Ware_Index const workerware, const std::string & programname)
{
	FlagJob j;

	j.request =
		new Request
			(*this, workerware, Flag::flag_job_request_callback, wwWORKER);
	j.program = programname;

	m_flag_jobs.push_back(j);
}

/**
 * This function is called when one of the flag job workers arrives on
 * the flag. Give him his job.
*/
void Flag::flag_job_request_callback
	(Game            &       game,
	 Request         &       rq,
	 Ware_Index,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	Flag & flag = ref_cast<Flag, PlayerImmovable>(target);

	assert(w);

	container_iterate(FlagJobs, flag.m_flag_jobs, i)
		if (i.current->request == &rq) {
			delete &rq;

			w->start_task_program(game, i.current->program);

			flag.m_flag_jobs.erase(i.current);
			return;
		}

	flag.molog("BUG: flag_job_request_callback: worker not found in list\n");
}

void Flag::log_general_info(const Widelands::Editor_Game_Base & egbase)
{
	molog("Flag at %i,%i\n", m_position.x, m_position.y);

	Widelands::PlayerImmovable::log_general_info(egbase);

	if (m_ware_filled) {
		molog("Wares at flag:\n");
		for (int i = 0; i < m_ware_filled; ++i) {
			PendingWare & pi = m_wares[i];
			molog
				(" %i/%i: %s(%i), nextstep %i, %s\n",
				 i + 1, m_ware_capacity,
				 pi.ware->descr().name().c_str(), pi.ware->serial(),
				 pi.nextstep.serial(),
				 pi.pending ? "pending" : "acked by carrier");
		}
	} else {
		molog("No wares at flag.\n");
	}
}


}
