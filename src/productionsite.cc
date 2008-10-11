/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "productionsite.h"

#include "editor_game_base.h"
#include "game.h"
#include "helper.h"
#include "i18n.h"
#include "interactive_player.h"
#include "map.h"
#include "player.h"
#include "production_program.h"
#include "profile.h"
#include "request.h"
#include "transport.h"
#include "tribe.h"
#include "wexception.h"
#include "worker.h"
#include "warelist.h"
#include "world.h"

#include "upcast.h"

namespace Widelands {

static const size_t STATISTICS_VECTOR_LENGTH = 10;



/*
==============================================================================

ProductionSite BUILDING

==============================================================================
*/

ProductionSite_Descr::ProductionSite_Descr
	(Tribe_Descr const & tribe_descr, std::string const & productionsite_name)
	: Building_Descr(tribe_descr, productionsite_name)
{}

ProductionSite_Descr::~ProductionSite_Descr()
{
	while (m_programs.size()) {
		delete m_programs.begin()->second;
		m_programs.erase(m_programs.begin());
	}
}


/**
 * Parse the additional information necessary for production buildings
 */
void ProductionSite_Descr::parse
	(char         const * const directory,
	 Profile            * const prof,
	 enhancements_map_t &      enhancements_map,
	 EncodeData const * const encdata)
{
	Section & global_s = prof->get_safe_section("global");

	// Stopabple defaults to true for Production sites
	m_stopable = true;

	Building_Descr::parse(directory, prof, enhancements_map, encdata);

	while
		(Section::Value const * const op = global_s.get_next_val("output"))
		try {
			if (Ware_Index const idx = tribe().ware_index(op->get_string())) {
				if (m_output.count(idx))
					throw wexception
						("this ware type has already been declared as an output");
				m_output.insert(idx);
			} else
				throw wexception
					("tribe does not define a ware type with this name");
		} catch (_wexception const & e) {
			throw wexception("output \"%s\": %s", op->get_string(), e.what());
		}

	if (Section * const s = prof->get_section("inputs"))
		while (Section::Value const * const val = s->get_next_val(0))
			try {
				if (Ware_Index const idx = tribe().ware_index(val->get_name())) {
					if (m_inputs.count(idx))
						throw wexception
							("this ware type has already been declared as an input");
					int32_t const value = val->get_int();
					if (value < 1 or 255 < value)
						throw wexception("count is out of range 1 .. 255");
					m_inputs.insert(std::pair<Ware_Index, uint8_t>(idx, value));
				} else
					throw wexception
						("tribe does not define a ware type with this name");
			} catch (_wexception const & e) {
				throw wexception
					("input \"%s=%s\": %s",
					 val->get_name(), val->get_string(), e.what());
			}

	// Are we only a production site?
	// If not, we might not have a worker
	std::string workerstr =
		is_only_production_site() ?
		global_s.get_safe_string("worker") : global_s.get_string("worker", "");

	std::vector<std::string> workernames(split_string(workerstr, ","));
	std::vector<std::string>::const_iterator const workernames_end =
		workernames.end();
	for
		(std::vector<std::string>::iterator it = workernames.begin();
		 it != workernames_end;
		 ++it)
		m_workers.push_back(*it);

	// Get programs
	while (Section::Value const * const v = global_s.get_next_val("program")) {
		std::string const program_name = v->get_string();
		ProductionProgram * program = 0;
		try {
			if (m_programs.count(program_name))
				throw wexception("this program has already been declared");
			program = new ProductionProgram(program_name);
			program->parse(directory, prof, program_name.c_str(), this, encdata);
			m_programs[program_name.c_str()] = program;
		} catch (std::exception const & e) {
			delete program;
			throw wexception("program %s: %s", program_name.c_str(), e.what());
		}
	}
}


/**
 * Get the program of the given name.
 */
const ProductionProgram * ProductionSite_Descr::get_program
	(std::string const & program_name) const
{
	const ProgramMap::const_iterator it = m_programs.find(program_name);
	if (it == m_programs.end())
		throw wexception
			("%s has no program '%s'", name().c_str(), program_name.c_str());
	return it->second;
}

/**
 * Create a new building of this type
 */
Building* ProductionSite_Descr::create_object() const
{return new ProductionSite(*this);}


/*
==============================

IMPLEMENTATION

==============================
*/

ProductionSite::ProductionSite(const ProductionSite_Descr & ps_descr) :
Building            (ps_descr),
m_fetchfromflag     (0),
m_program_timer     (false),
m_program_time      (0),
m_post_timer        (50),
m_statistics        (STATISTICS_VECTOR_LENGTH, false),
m_statistics_changed(true),
m_last_stat_percent (0)
{}


/**
 * Display whether we're occupied.
 */
std::string ProductionSite::get_statistics_string()
{
	if (!m_workers.size())
		return _("(not occupied)");
	else if (m_worker_requests.size()) {
		char buffer[1000];
		snprintf
			(buffer, sizeof(buffer),
			 _("Waiting for %u workers!"), m_worker_requests.size());
		return buffer;
	}

	if (m_statistics_changed)
		calc_statistics();

	if (m_stop)
		return _("(stopped)");

	return m_statistics_buf;
}


void ProductionSite::fill(Game & game) {
	Building::fill(game);
	Tribe_Descr const & tribe = owner().tribe();
	std::vector<std::string> const & worker_types = descr().workers();
	std::vector<std::string>::const_iterator const worker_types_end =
		worker_types.end();
	for
		(std::vector<std::string>::const_iterator it = worker_types.begin();
		 it != worker_types_end;
		 ++it)
	{
		Worker & worker =
			tribe.get_worker_descr(tribe.worker_index(it->c_str()))->create
			(game, owner(), *get_base_flag(), get_position());
		worker.start_task_buildingwork(&game);
		m_workers.push_back(&worker);
	}
}

/**
 * Calculate statistic.
 */
void ProductionSite::calc_statistics()
{
	uint32_t pos;
	uint32_t ok = 0;
	uint32_t lastOk = 0;

	for (pos = 0; pos < STATISTICS_VECTOR_LENGTH; ++pos) {
		if (m_statistics[pos]) {
			++ok;
			if (pos >= STATISTICS_VECTOR_LENGTH / 2)
				++lastOk;
		}
	}
	double percOk = (ok * 100) / STATISTICS_VECTOR_LENGTH;
	double lastPercOk = (lastOk * 100) / (STATISTICS_VECTOR_LENGTH / 2);

	const std::string trend =
		lastPercOk > percOk ? _("UP") : lastPercOk < percOk ? _("DOWN") : "=";

	if (0 < percOk and percOk < 100)
		snprintf
			(m_statistics_buf, sizeof(m_statistics_buf),
			 "%.0f%% %s", percOk, trend.c_str());
	else snprintf(m_statistics_buf, sizeof(m_statistics_buf), "%.0f%%", percOk);

	m_last_stat_percent = static_cast<char>(percOk); //FIXME: ARGH!

	m_statistics_changed = false;
}


/**
 * Initialize the production site.
 */
void ProductionSite::init(Editor_Game_Base* g)
{
	Building::init(g);

	if (upcast(Game, game, g)) {

		if (m_workers.size()) {
			std::vector<Worker *>::const_iterator const workers_end =
				m_workers.end();
			for
				(std::vector<Worker *>::const_iterator it = m_workers.begin();
				 it != workers_end;
				 ++it)
				(*it)->set_location(this);
		} else {//  request workers
			std::vector<std::string> const & worker_types = descr().workers();
			std::vector<std::string>::const_iterator const worker_types_end =
				worker_types.end();
			for
				(std::vector<std::string>::const_iterator it =
				 	worker_types.begin();
				 it != worker_types_end;
				 ++it)
				request_worker(it->c_str());
		}

		// Init input ware queues
		std::map<Ware_Index, uint8_t> const & inputs = descr().inputs();
		std::map<Ware_Index, uint8_t>::const_iterator const inputs_end =
			inputs.end();
		m_input_queues.reserve(inputs.size());
		for
			(std::map<Ware_Index, uint8_t>::const_iterator it = inputs.begin();
			 it != inputs_end;
			 ++it)
			m_input_queues.push_back(&(new WaresQueue(this))->init(*it));
	}
}

/**
 * Change the economy for the wares queues.
 *
 * \note Workers are dealt with in the PlayerImmovable code.
 */
void ProductionSite::set_economy(Economy* e)
{
	if (Economy * const old = get_economy()) {
		for (uint32_t i = 0; i < m_input_queues.size(); ++i)
			m_input_queues[i]->remove_from_economy(old);
	}

	Building::set_economy(e);
	{
		std::vector<Request *>::const_iterator m_worker_requests_end =
			m_worker_requests.end();
		for
			(std::vector<Request *>::const_iterator it = m_worker_requests.begin();
			 it != m_worker_requests_end;
			 ++it)
			if (*it) (*it)->set_economy(e);
	}

	if (e) {
		std::vector<WaresQueue *>::const_iterator m_input_queues_end =
			m_input_queues.end();
		for
			(std::vector<WaresQueue *>::const_iterator it = m_input_queues.begin();
			 it != m_input_queues_end;
			 ++it)
			(*it)->add_to_economy(e);
	}
}

/**
 * Cleanup after a production site is removed
 */
void ProductionSite::cleanup(Editor_Game_Base* g)
{
	// Release worker
	if (m_worker_requests.size()) {
		for (size_t i = 0; i < m_worker_requests.size(); ++i) {
			delete m_worker_requests[i];
			m_worker_requests[i]=0;
		}
		m_worker_requests.clear();
	}

	if (m_workers.size()) {
		for (size_t i = 0; i < m_workers.size(); ++i) {
			Worker* w = m_workers[i];

			// Ensure we don't re-request the worker when remove_worker is called
			m_workers[i] = 0;

			// Actually remove the worker
			if (g->objects().object_still_available(w))
				w->set_location(0);
		}
		m_workers.clear();
	}

	// Cleanup the wares queues
	for (uint32_t i = 0; i < m_input_queues.size(); ++i) {
		m_input_queues[i]->cleanup();
		delete m_input_queues[i];
	}
	m_input_queues.clear();


	Building::cleanup(g);
}


/**
 * Intercept remove_worker() calls to unassign our worker, if necessary.
 */
void ProductionSite::remove_worker(Worker* w)
{
	for (size_t i = 0; i < m_workers.size(); ++i) {
		if (m_workers[i] == w) {
			m_workers[i] = 0;
			request_worker(w->name().c_str());
			m_workers.erase(m_workers.begin() + i);
			break;
		}
	}

	Building::remove_worker(w);
}


/**
 * Issue the worker requests
 */
void ProductionSite::request_worker(const char * const worker_name) {
	assert(worker_name);

	Ware_Index wareid = owner().tribe().safe_worker_index(worker_name);

	m_worker_requests.push_back(new Request(this, wareid, &ProductionSite::request_worker_callback, this, Request::WORKER));
}


/**
 * Called when our worker arrives.
 */
void ProductionSite::request_worker_callback
	(Game * g, Request * rq, Ware_Index, Worker * const w, void * const data)
{
	ProductionSite & psite = *static_cast<ProductionSite *>(data);

	assert(w);
	assert(w->get_location(g) == &psite);

	{
		std::vector<Request *> & worker_requests = psite.m_worker_requests;
		std::vector<Request *>::iterator it = worker_requests.begin();
		//  Assume that rq must be in worker_requests.
		assert(worker_requests.size());
		while (*it != rq) {
			++it;
			assert(it != worker_requests.end());
		}
		worker_requests.erase(it);
	}

	psite.m_workers.push_back(w);

	delete rq;

	// It's always the first worker doing building work,
	// the others only idle. Still, we need to wake up the
	// primary worker if the worker that has just arrived is
	// the last one we need to start working.
	if (w == psite.m_workers[0]) {
		w->start_task_buildingwork(g);
	} else {
		w->start_task_idle(g, 0, -1);

		if (psite.can_start_working()) {
			// This is for compatibility with older savegames, where the first
			// worker was set to idle if more workers were needed.
			if (psite.m_workers[0]->top_state().task != &Worker::taskBuildingwork) {
				psite.m_workers[0]->reset_tasks(g);
				psite.m_workers[0]->start_task_buildingwork(g);
			} else {
				psite.m_workers[0]->update_task_buildingwork(g);
			}
		}
	}
}


/**
 * Advance the program state if applicable.
 */
void ProductionSite::act(Game* g, uint32_t data)
{
	Building::act(g, data);

	if
		(m_program_timer
		 and
		 static_cast<int32_t>(g->get_gametime() - m_program_time) >= 0)
	{
		m_program_timer = false;

		if (!m_program.size())
			return find_and_start_next_program(*g);

		State* state = get_current_program();

		assert(state);

		if (state->ip >= state->program->get_size())
			return program_end(*g, Completed);

		if (m_anim != descr().get_animation("idle")) {
			// Restart idle animation, which is the default
			start_animation(g, descr().get_animation("idle"));
		}

		return program_act(*g);
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
	assert(get_current_program());
	State & state = *get_current_program();
#if 0
	molog
		("PSITE: program %s#%i\n", state.program->get_name().c_str(), state.ip);
#endif
	if (m_stop) {
		program_end(game, Failed);
		m_program_timer = true;
		m_program_time = schedule_act(&game, 20000);
	} else
		state.program->get_action(state.ip)->execute(game, *this);
}


/**
 * Remember that we need to fetch an item from the flag.
 */
bool ProductionSite::fetch_from_flag(Game* g)
{
	++m_fetchfromflag;

	if (m_workers.size())
		m_workers[0]->update_task_buildingwork(g);

	return true;
}

/**
 * \return True if this production site could theoretically start working (if
 * all workers are present)
 */
bool ProductionSite::can_start_working() const throw ()
{
	return not m_worker_requests.size();
}

/**
 * There's currently nothing to do for the worker.
 *
 * \note We assume that the worker is inside the building when this is called.
 */
bool ProductionSite::get_building_work(Game* g, Worker* w, bool success)
{
	assert(w == m_workers[0]);

	State* state = get_current_program();

	// If unsuccessful: Check if we need to abort current program
	if (!success && state)
	{
		const ProductionAction* action = state->program->get_action(state->ip);

		if (dynamic_cast<ActWorker const *>(action)) {
			program_end(*g, Failed);
			state = 0;
		}
	}

	// Default actions first
	if (WareInstance * const item = w->fetch_carried_item(g)) {

		molog("ProductionSite::get_building_work: start dropoff\n");

		w->start_task_dropoff(*g, *item);
		return true;
	}

	if (m_fetchfromflag) {
		--m_fetchfromflag;
		w->start_task_fetchfromflag(g);
		return true;
	}

	if (m_produced_items.size()) {
		//  There is still a produced item waiting for delivery. Carry it out
		//  before continuing with the program.
		std::pair<Ware_Index, uint8_t> & ware_type_with_count =
			*m_produced_items.rbegin();
		{
			Ware_Index const ware_index = ware_type_with_count.first;
			Tribe_Descr const & tribe = owner().tribe();
			Item_Ware_Descr const & item_ware_descr =
				*tribe.get_ware_descr(ware_type_with_count.first);
			{
				WareInstance & item =
					*new WareInstance(ware_index, &item_ware_descr);
				item.init(g);
				w->start_task_dropoff(*g, item);
			}
			owner().ware_produced(ware_index); //  for statistics
		}
		assert(ware_type_with_count.second);
		if (--ware_type_with_count.second == 0)
			m_produced_items.pop_back();
		return true;
	}

	// Check if all workers are there
	if (!can_start_working())
		return false;

	// Start program if we haven't already done so
	if (!state)
	{
		m_program_timer = true;
		m_program_time = schedule_act(g, 10);
	}
	else if (state->ip < state->program->get_size())
	{
		const ProductionAction* action = state->program->get_action(state->ip);

		if (upcast(ActWorker const, worker_action, action)) {
			if (state->phase == 0) {
				w->start_task_program(g, worker_action->program());
				++state->phase;
				return true;
			} else
				program_step(*g);
		} else if (dynamic_cast<ActProduce const *>(action)) {
			//  All the wares that we produced have been carried out, so continue
			//  with the program.
			program_step(*g);
		}
	}
	return false;
}


/**
 * Advance the program to the next step, but does not schedule anything.
 */
void ProductionSite::program_step
	(Game & game, uint32_t const delay, uint32_t const phase)
{
	State* state = get_current_program();

	assert(state);

	++state->ip;
	state->phase = phase;
	m_program_timer = true;
	m_program_time  = schedule_act(&game, delay);
}


/**
 * Push the given program onto the stack and schedule acting.
 */
void ProductionSite::program_start
	(Game & game, std::string const & program_name)
{
	State state;

	state.program = descr().get_program(program_name);
	state.ip = 0;
	state.phase = 0;

	m_program.push_back(state);

	m_program_timer = true;
	m_program_time = schedule_act(&game, 10);
}


/**
 * Ends the current program now and updates the productivity statistics.
 *
 * \pre Any program is running
 * \post No program is running, acting is scheduled
 */
void ProductionSite::program_end(Game & game, Program_Result const result)
{
	assert(m_program.size());

	m_program.pop_back();
	if (m_program.size())
		m_program.rbegin()->phase = result;

	if (result != Skipped) {
		m_statistics_changed = true;
		m_statistics.erase(m_statistics.begin(), m_statistics.begin() + 1);
		m_statistics.push_back(result == Completed);

		if (result == Completed)
			for (size_t i = 0; i < m_workers.size(); ++i)
				m_workers[i]->gain_experience(game);
	}

	m_program_timer = true;
	m_program_time = schedule_act(&game, m_post_timer);
}

};
