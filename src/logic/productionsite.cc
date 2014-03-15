/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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


#include "logic/productionsite.h"

#include <boost/format.hpp>
#include <libintl.h>

#include "economy/economy.h"
#include "economy/request.h"
#include "economy/ware_instance.h"
#include "economy/wares_queue.h"
#include "helper.h"
#include "i18n.h"
#include "logic/carrier.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/warelist.h"
#include "logic/world.h"
#include "profile/profile.h"
#include "upcast.h"
#include "wexception.h"

namespace Widelands {

static const size_t STATISTICS_VECTOR_LENGTH = 20;



/*
==============================================================================

ProductionSite BUILDING

==============================================================================
*/

ProductionSite_Descr::ProductionSite_Descr
	(char const * const _name, char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 const Tribe_Descr & _tribe)
	:
	Building_Descr(_name, _descname, directory, prof, global_s, _tribe)
{
	while
		(Section::Value const * const op = global_s.get_next_val("output"))
		try {
			if (Ware_Index idx = tribe().ware_index(op->get_string())) {
				if (m_output_ware_types.count(idx))
					throw wexception("this ware type has already been declared as an output");
				m_output_ware_types.insert(idx);
			} else if ((idx = tribe().worker_index(op->get_string()))) {
				if (m_output_worker_types.count(idx))
					throw wexception("this worker type has already been declared as an output");
				m_output_worker_types.insert(idx);
			} else
				throw wexception("tribe does not define a ware or worker type with this name");
		} catch (const _wexception & e) {
			throw wexception("output \"%s\": %s", op->get_string(), e.what());
		}

	if (Section * const s = prof.get_section("inputs"))
		while (Section::Value const * const val = s->get_next_val())
			try {
				if (Ware_Index const idx = tribe().ware_index(val->get_name())) {
					container_iterate_const(BillOfMaterials, inputs(), i)
						if (i.current->first == idx)
							throw wexception("duplicated");
					int32_t const value = val->get_int();
					if (value < 1 or 255 < value)
						throw wexception("count is out of range 1 .. 255");
					m_inputs.push_back(std::pair<Ware_Index, uint8_t>(idx, value));
				} else
					throw wexception
						("tribe does not define a ware type with this name");
			} catch (const _wexception & e) {
				throw wexception("input \"%s=%s\": %s", val->get_name(), val->get_string(), e.what());
			}

	// Are we only a production site?
	// If not, we might not have a worker
	if (Section * const working_positions_s = prof.get_section("working positions"))
		while (Section::Value const * const v = working_positions_s->get_next_val())
			try {
				if (Ware_Index const woi = tribe().worker_index(v->get_name())) {
					container_iterate_const(BillOfMaterials, working_positions(), i)
						if (i.current->first == woi)
							throw wexception("duplicated");
					m_working_positions.push_back(std::pair<Ware_Index, uint32_t>(woi, v->get_positive()));
				} else
					throw wexception("invalid");
			} catch (const _wexception & e) {
				throw wexception("%s=\"%s\": %s", v->get_name(), v->get_string(), e.what());
			}
	if (working_positions().empty() and not global_s.has_val("max_soldiers"))
		throw wexception("no working/soldier positions");

	// Get programs
	if (Section * const programs_s = prof.get_section("programs"))
	while (Section::Value const * const v = programs_s->get_next_val()) {
		std::string program_name = v->get_name();
		std::transform
			(program_name.begin(), program_name.end(), program_name.begin(),
			 tolower);
		try {
			if (m_programs.count(program_name))
				throw wexception("this program has already been declared");
			m_programs[program_name] =
				new ProductionProgram
					(directory, prof, program_name, v->get_string(), this);
		} catch (const std::exception & e) {
			throw wexception("program %s: %s", program_name.c_str(), e.what());
		}
	}
}

ProductionSite_Descr::~ProductionSite_Descr()
{
	while (m_programs.size()) {
		delete m_programs.begin()->second;
		m_programs.erase(m_programs.begin());
	}
}


/**
 * Get the program of the given name.
 */
const ProductionProgram * ProductionSite_Descr::get_program
	(const std::string & program_name) const
{
	Programs::const_iterator const it = programs().find(program_name);
	if (it == m_programs.end())
		throw wexception
			("%s has no program '%s'", name().c_str(), program_name.c_str());
	return it->second;
}

/**
 * Create a new building of this type
 */
Building & ProductionSite_Descr::create_object() const {
	return *new ProductionSite(*this);
}


/*
==============================

IMPLEMENTATION

==============================
*/

ProductionSite::ProductionSite(const ProductionSite_Descr & ps_descr) :
	Building            (ps_descr),
	m_working_positions (new Working_Position[ps_descr.nr_working_positions()]),
	m_fetchfromflag     (0),
	m_program_timer     (false),
	m_program_time      (0),
	m_post_timer        (50),
	m_statistics        (STATISTICS_VECTOR_LENGTH, false),
	m_statistics_changed(true),
	m_last_stat_percent (0),
	m_is_stopped        (false),
	m_default_anim      ("idle")
{
	m_statistics_buffer[0] = '\0';
	m_result_buffer[0] = '\0';
}

ProductionSite::~ProductionSite() {
	delete[] m_working_positions;
}


/**
 * Display whether we're occupied.
 */
std::string ProductionSite::get_statistics_string()
{
	uint32_t const nr_working_positions = descr().nr_working_positions();
	uint32_t       nr_workers           = 0;
	for (uint32_t i = nr_working_positions; i;)
		nr_workers += m_working_positions[--i].worker ? 1 : 0;
	if (!nr_workers) {
		 return
			(boost::format("<font color=%s>%s</font>") % UI_FONT_CLR_BAD_HEX % _("(not occupied)")).str();
	} else if (uint32_t const nr_requests = nr_working_positions - nr_workers) {
		return
			(boost::format("<font color=%s>%s</font>") % UI_FONT_CLR_BAD_HEX %
				ngettext("Worker missing", "Workers missing", nr_requests))
			.str();
	}

	if (m_statistics_changed) {
		calc_statistics();
	}

	if (m_is_stopped) {
		return (boost::format("<font color=%s>%s</font>") % UI_FONT_CLR_BRIGHT_HEX % _("(stopped)")).str();
	}

	return m_statistics_buffer;
}

/**
 * Detect if the workers are experienced enough for an upgrade
 * @param idx Index of the enhanchement
 */
bool ProductionSite::has_workers(Building_Index targetSite, Game & /* game */)
{
	// bld holds the description of the building we want to have
	if (upcast(ProductionSite_Descr const, bld, tribe().get_building_descr(targetSite))) {
		// if he has workers
		if (bld->nr_working_positions()) {
			Ware_Index need = bld->working_positions()[0].first;
			for (unsigned int i = 0; i < descr().nr_working_positions(); ++i) {
				if (!working_positions()[i].worker) {
					return false; // no one is in this house
				} else {
					Ware_Index have = working_positions()[i].worker->worker_index();
					if (tribe().get_worker_descr(have)->can_act_as(need)) {
						return true; // he found a lead worker
					}
				}
			}
			return false;
		}
		return true;
	} else return true;
}



WaresQueue & ProductionSite::waresqueue(Ware_Index const wi) {
	container_iterate_const(Input_Queues, m_input_queues, i)
		if ((*i.current)->get_ware() == wi)
			return **i.current;
	throw wexception
		("%s (%u) has no WaresQueue for %u",
		 name().c_str(), serial(), wi.value());
}

/**
 * Calculate statistic.
 */
void ProductionSite::calc_statistics()
{
	uint8_t pos;
	uint8_t ok = 0;
	uint8_t lastOk = 0;

	for (pos = 0; pos < STATISTICS_VECTOR_LENGTH; ++pos) {
		if (m_statistics[pos]) {
			++ok;
			if (pos >= STATISTICS_VECTOR_LENGTH / 2)
				++lastOk;
		}
	}
	// Somehow boost::format doesn't handle correctly uint8_t in this case
	unsigned int percOk = (ok * 100) / STATISTICS_VECTOR_LENGTH;
	unsigned int lastPercOk = (lastOk * 100) / (STATISTICS_VECTOR_LENGTH / 2);

	std::string color;
	if (percOk < 33)
		color = UI_FONT_CLR_BAD_HEX;
	else if (percOk < 66)
		color = UI_FONT_CLR_OK_HEX;
	else
		color = UI_FONT_CLR_GOOD_HEX;
	const std::string perc_str =
		(boost::format("<font color=%1$s>%2$i%%</font>") % color % percOk).str();

	std::string trend;
	if (lastPercOk > percOk) {
		color = UI_FONT_CLR_GOOD_HEX;
		trend = "+";
	} else if (lastPercOk < percOk) {
		color = UI_FONT_CLR_BAD_HEX;
		trend = "-";
	} else {
		color = UI_FONT_CLR_BRIGHT_HEX;
		trend = "=";
	}
	const std::string trend_str =
		(boost::format("<font color=%s>%s</font>") % color % trend).str();

	if (0 < percOk and percOk < 100) {
		snprintf
			(m_statistics_buffer, sizeof(m_statistics_buffer),
			 "%s %s", perc_str.c_str(), trend_str.c_str());
	} else {
		snprintf
			(m_statistics_buffer, sizeof(m_statistics_buffer),
			 "%s", perc_str.c_str());
	}
	m_last_stat_percent = percOk;

	m_statistics_changed = false;
}


/**
 * Initialize the production site.
 */
void ProductionSite::init(Editor_Game_Base & egbase)
{
	Building::init(egbase);

	const BillOfMaterials & inputs = descr().inputs();
	m_input_queues.resize(inputs.size());
	for (ware_range i(inputs); i; ++i)
		m_input_queues[i.i] =
			new WaresQueue
			(*this,
			 i.current->first,
			 i.current->second);

	//  Request missing workers.
	Working_Position * wp = m_working_positions;
	container_iterate_const(BillOfMaterials, descr().working_positions(), i) {
		Ware_Index const worker_index = i.current->first;
		for (uint32_t j = i.current->second; j; --j, ++wp)
			if (Worker * const worker = wp->worker)
				worker->set_location(this);
			else
				wp->worker_request = &request_worker(worker_index);
	}

	if (upcast(Game, game, &egbase))
		try_start_working(*game);
}

/**
 * Change the economy for the wares queues.
 *
 * \note Workers are dealt with in the PlayerImmovable code.
 */
void ProductionSite::set_economy(Economy * const e)
{
	if (Economy * const old = get_economy())
		container_iterate_const(Input_Queues, m_input_queues, i)
			(*i.current)->remove_from_economy(*old);

	Building::set_economy(e);
	for (uint32_t i = descr().nr_working_positions(); i;)
		if (Request * const r = m_working_positions[--i].worker_request)
			r->set_economy(e);

	if (e)
		container_iterate_const(Input_Queues, m_input_queues, i)
			(*i.current)->add_to_economy(*e);
}

/**
 * Cleanup after a production site is removed
 */
void ProductionSite::cleanup(Editor_Game_Base & egbase)
{
	for (uint32_t i = descr().nr_working_positions(); i;) {
		--i;
		delete m_working_positions[i].worker_request;
		m_working_positions[i].worker_request = nullptr;
		Worker * const w = m_working_positions[i].worker;

		//  Ensure we do not re-request the worker when remove_worker is called.
		m_working_positions[i].worker = nullptr;

		// Actually remove the worker
		if (egbase.objects().object_still_available(w))
			w->set_location(nullptr);
	}

	// Cleanup the wares queues
	for (uint32_t i = 0; i < m_input_queues.size(); ++i) {
		m_input_queues[i]->cleanup();
		delete m_input_queues[i];
	}
	m_input_queues.clear();


	Building::cleanup(egbase);
}

/**
 * Create a new worker inside of us out of thin air
 *
 * returns 0 on success -1 if there is no room for this worker
 */
int ProductionSite::warp_worker
	(Editor_Game_Base & egbase, const Worker_Descr & wdes)
{
	bool assigned = false;
	Working_Position * current = m_working_positions;
	for
		(Working_Position * const end = current + descr().nr_working_positions();
		 current < end;
		 ++current)
	{
		if (current->worker)
			continue;

		assert(current->worker_request);
		if (current->worker_request->get_index() != wdes.worker_index())
			continue;

		// Okay, space is free and worker is fitting. Let's create him
		Worker & worker = wdes.create(egbase, owner(), this, get_position());

		if (upcast(Game, game, &egbase))
			worker.start_task_idle(*game, 0, -1);
		current->worker = &worker;
		delete current->worker_request;
		current->worker_request = nullptr;
		assigned = true;
		break;
	}
	if (not assigned)
		return -1;

	if (upcast(Game, game, &egbase))
		try_start_working(*game);
	return 0;
}

/**
 * Intercept remove_worker() calls to unassign our worker, if necessary.
 */
void ProductionSite::remove_worker(Worker & w)
{
	molog("%s leaving\n", w.descname().c_str());
	Working_Position * wp = m_working_positions;

	container_iterate_const(BillOfMaterials, descr().working_positions(), i) {
		Ware_Index const worker_index = i.current->first;
		for (uint32_t j = i.current->second; j; --j, ++wp) {
			Worker * const worker = wp->worker;
			if (worker && worker == &w) {
				// do not request the type of worker that is currently assigned - maybe a trained worker was
				// evicted to make place for a level 0 worker.
				// Therefore we again request the worker from the Working_Position of descr()
				*wp = Working_Position(&request_worker(worker_index), nullptr);
				Building::remove_worker(w);
				return;
			}
		}
	}

	Building::remove_worker(w);
}


/**
 * Issue the worker requests
 */
Request & ProductionSite::request_worker(Ware_Index const wareid) {
	return
		*new Request
			(*this,
			 wareid,
			 ProductionSite::request_worker_callback,
			 wwWORKER);
}


/**
 * Called when our worker arrives.
 */
void ProductionSite::request_worker_callback
	(Game            &       game,
	 Request         &       rq,
	 Ware_Index              /* widx */,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	ProductionSite & psite = ref_cast<ProductionSite, PlayerImmovable>(target);

	assert(w);
	assert(w->get_location(game) == &psite);

	// If there is more than one working position, it's possible, that different level workers are
	// requested and therefor possible, that a higher qualified worker answers a request for a lower
	// leveled worker, although a worker with equal level (as the arrived worker has) is needed as well.
	// Therefor, we first check whether the worker exactly fits the requested one. If yes, we place the
	// worker and everything is fine, else we shuffle through the working positions, whether one of them
	// needs a worker like the one just arrived. That way it is of course still possible, that the worker is
	// placed on the slot that originally requested the arrived worker.
	bool worker_placed = false;
	Ware_Index     idx = w->worker_index();
	for (Working_Position * wp = psite.m_working_positions;; ++wp) {
		if (wp->worker_request == &rq) {
			if (wp->worker_request->get_index() == idx) {
				// Place worker
				delete &rq;
				*wp = Working_Position(nullptr, w);
				worker_placed = true;
			} else {
				// Set new request for this slot
				Ware_Index workerid = wp->worker_request->get_index();
				delete wp->worker_request;
				wp->worker_request = &psite.request_worker(workerid);
			}
			break;
		}
	}
	while (!worker_placed) {
		{
			uint8_t nwp = psite.descr().nr_working_positions();
			uint8_t pos = 0;
			Working_Position * wp = psite.m_working_positions;
			for (; pos < nwp; ++wp, ++pos) {
				// Find a fitting slot
				if (!wp->worker && !worker_placed)
					if (wp->worker_request->get_index() == idx) {
						delete wp->worker_request;
						*wp = Working_Position(nullptr, w);
						worker_placed = true;
						break;
					}
			}
		}
		if (!worker_placed) {
			// Find the next smaller version of this worker
			Ware_Index nuwo    = psite.tribe().get_nrworkers();
			Ware_Index current = Ware_Index(static_cast<size_t>(0));
			for (; current < nuwo; ++current) {
				Worker_Descr const * worker = psite.tribe().get_worker_descr(current);
				if (worker->becomes() == idx) {
					idx = current;
					break;
				}
			}
			if (current == nuwo)
				throw
					wexception
						("Something went wrong! No fitting place for worker %s in %s at (%u, %u) found!",
						 w->descr().descname().c_str(), psite.descr().descname().c_str(),
						 psite.get_position().x, psite.get_position().y);
		}
	}

	// It's always the first worker doing building work,
	// the others only idle. Still, we need to wake up the
	// primary worker if the worker that has just arrived is
	// the last one we need to start working.
	w->start_task_idle(game, 0, -1);
	psite.try_start_working(game);
	psite.workers_changed();
}


/**
 * Advance the program state if applicable.
 */
void ProductionSite::act(Game & game, uint32_t const data)
{
	Building::act(game, data);

	if
		(m_program_timer
		 and
		 static_cast<int32_t>(game.get_gametime() - m_program_time) >= 0)
	{
		m_program_timer = false;

		if (!can_start_working()) {
			while (!m_stack.empty())
				program_end(game, Failed);
		} else {
			if (m_stack.empty()) {
				m_working_positions[0].worker->update_task_buildingwork(game);
				return;
			}

			State & state = top_state();
			if (state.program->get_size() <= state.ip)
				return program_end(game, Completed);

			if (m_anim != descr().get_animation(m_default_anim)) {
				// Restart idle animation, which is the default
				start_animation(game, descr().get_animation(m_default_anim));
			}

			return program_act(game);
		}
	}
}


void ProductionSite::find_and_start_next_program(Game & game)
{
	program_start(game, "work");
}


/**
 * Perform the current program action.
 *
 * \pre The program is running and in a valid state.
 * \post (Potentially indirect) scheduling for the next step has been done.
 */
void ProductionSite::program_act(Game & game)
{
	State & state = top_state();

	if (m_is_stopped) {
		program_end(game, Failed);
		m_program_timer = true;
		m_program_time = schedule_act(game, 20000);
	} else
		(*state.program)[state.ip].execute(game, *this);
}


/**
 * Remember that we need to fetch an ware from the flag.
 */
bool ProductionSite::fetch_from_flag(Game & game)
{
	++m_fetchfromflag;

	if (can_start_working())
		m_working_positions[0].worker->update_task_buildingwork(game);

	return true;
}


void ProductionSite::log_general_info(const Editor_Game_Base & egbase) {
	Building::log_general_info(egbase);

	molog("m_is_stopped: %u\n", m_is_stopped);
}


void ProductionSite::set_stopped(bool const stopped) {
	m_is_stopped = stopped;
	get_economy()->rebalance_supply();
}

/**
 * \return True if this production site could theoretically start working (if
 * all workers are present)
 */
bool ProductionSite::can_start_working() const
{
	for (uint32_t i = descr().nr_working_positions(); i;)
		if (m_working_positions[--i].worker_request)
			return false;
	return true;
}


void ProductionSite::try_start_working(Game & game) {
	if (can_start_working() and descr().working_positions().size()) {
		Worker & main_worker = *m_working_positions[0].worker;
		main_worker.reset_tasks(game);
		main_worker.start_task_buildingwork(game);
	}
}

/**
 * There's currently nothing to do for the worker.
 *
 * \note We assume that the worker is inside the building when this is called.
 */
bool ProductionSite::get_building_work
	(Game & game, Worker & worker, bool const success)
{
	assert(descr().working_positions().size());
	assert(&worker == m_working_positions[0].worker);

	// If unsuccessful: Check if we need to abort current program
	if (!success) {
		State * state = get_state();
		if (state->ip < state->program->get_size())
			(*state->program)[state->ip].building_work_failed(game, *this, worker);
	}

	// Default actions first
	if (WareInstance * const ware = worker.fetch_carried_ware(game)) {
		worker.start_task_dropoff(game, *ware);
		return true;
	}

	if (m_fetchfromflag) {
		--m_fetchfromflag;
		worker.start_task_fetchfromflag(game);
		return true;
	}

	if (!m_produced_wares.empty()) {
		//  There is still a produced ware waiting for delivery. Carry it out
		//  before continuing with the program.
		std::pair<Ware_Index, uint8_t> & ware_type_with_count =
			*m_produced_wares.rbegin();
		{
			Ware_Index const ware_index = ware_type_with_count.first;
			const WareDescr & ware_ware_descr =
				*tribe().get_ware_descr(ware_type_with_count.first);
			{
				WareInstance & ware =
					*new WareInstance(ware_index, &ware_ware_descr);
				ware.init(game);
				worker.start_task_dropoff(game, ware);
			}
			owner().ware_produced(ware_index); //  for statistics
		}
		assert(ware_type_with_count.second);
		if (--ware_type_with_count.second == 0)
			m_produced_wares.pop_back();
		return true;
	}

	if (!m_recruited_workers.empty()) {
		//  There is still a recruited worker waiting to be released. Send it
		//  out.
		std::pair<Ware_Index, uint8_t> & worker_type_with_count =
			*m_recruited_workers.rbegin();
		{
			const Worker_Descr & worker_descr =
				*tribe().get_worker_descr(worker_type_with_count.first);
			{
				Worker & recruit =
					ref_cast<Worker, Bob>(worker_descr.create_object());
				recruit.set_owner(&worker.owner());
				recruit.set_position(game, worker.get_position());
				recruit.init(game);
				recruit.set_location(this);
				recruit.start_task_leavebuilding(game, true);
				worker.start_task_releaserecruit(game, recruit);
			}
		}
		assert(worker_type_with_count.second);
		if (--worker_type_with_count.second == 0)
			m_recruited_workers.pop_back();
		return true;
	}

	// Drop all the wares that are too much out to the flag.
	container_iterate(Input_Queues, m_input_queues, iqueue) {
		WaresQueue * queue = *iqueue;
		if (queue->get_filled() > queue->get_max_fill()) {
			queue->set_filled(queue->get_filled() - 1);
			const WareDescr & wd = *tribe().get_ware_descr(queue->get_ware());
			WareInstance & ware = *new WareInstance(queue->get_ware(), &wd);
			ware.init(game);
			worker.start_task_dropoff(game, ware);
			return true;
		}
	}

	// Check if all workers are there
	if (!can_start_working())
		return false;

	// Start program if we haven't already done so
	State * state = get_state();
	if (!state) {
		//m_program_timer = true;
		find_and_start_next_program(game);
		// m_program_time = schedule_act(game, 10);
	} else if (state->ip < state->program->get_size()) {
		const ProductionProgram::Action & action = (*state->program)[state->ip];
		return action.get_building_work(game, *this, worker);
	}

	return false;
}


/**
 * Advance the program to the next step.
 */
void ProductionSite::program_step
	(Game & game, uint32_t const delay, uint32_t const phase)
{
	State & state = top_state();
	++state.ip;
	state.phase = phase;
	m_program_timer = true;
	m_program_time  = schedule_act(game, delay);
}


/**
 * Push the given program onto the stack and schedule acting.
 */
void ProductionSite::program_start
	(Game & game, const std::string & program_name)
{
	State state;

	state.program = descr().get_program(program_name);
	state.ip = 0;
	state.phase = 0;

	m_stack.push_back(state);

	m_program_timer = true;
	uint32_t tdelta = 10;
	Skipped_Programs::const_iterator i = m_skipped_programs.find(program_name);
	if (i != m_skipped_programs.end()) {
		uint32_t const gametime = game.get_gametime();
		uint32_t const earliest_allowed_start_time = i->second + 10000;
		if (gametime + tdelta < earliest_allowed_start_time)
			tdelta = earliest_allowed_start_time - gametime;
	}
	m_program_time = schedule_act(game, tdelta);
}


/**
 * Ends the current program now and updates the productivity statistics.
 *
 * \pre Any program is running
 * \post No program is running, acting is scheduled
 */
void ProductionSite::program_end(Game & game, Program_Result const result)
{
	assert(m_stack.size());

	const std::string & program_name = top_state().program->name();

	m_stack.pop_back();
	if (!m_stack.empty())
		top_state().phase = result;

	switch (result) {
	case Failed:
	case Completed:
		m_skipped_programs.erase(program_name);
		m_statistics_changed = true;
		m_statistics.erase(m_statistics.begin(), m_statistics.begin() + 1);
		m_statistics.push_back(result == Completed);
		if (result == Completed) {
			for (uint32_t i = descr().nr_working_positions(); i;)
				m_working_positions[--i].worker->gain_experience(game);
			m_result_buffer[0] = '\0';
			Building::workers_changed();
		}
		calc_statistics();
		break;
	case Skipped:
		m_skipped_programs[program_name] = game.get_gametime();
		break;
	case None:
		break;
	default:
		throw wexception("[ProductionSite::program_end]: impossible result");
	}

	m_program_timer = true;
	m_program_time = schedule_act(game, m_post_timer);
}

/// Changes the default anim string to \li anim
void ProductionSite::set_default_anim(std::string anim)
{
	if (m_default_anim == anim)
		return;

	if (!descr().is_animation_known(anim))
		return;

	m_default_anim = anim;
}

}
