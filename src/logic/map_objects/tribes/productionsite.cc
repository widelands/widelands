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


#include "logic/map_objects/tribes/productionsite.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/request.h"
#include "economy/ware_instance.h"
#include "economy/wares_queue.h"
#include "graphic/text_constants.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warelist.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"

namespace Widelands {

static const size_t STATISTICS_VECTOR_LENGTH = 20;

/*
==============================================================================

ProductionSite BUILDING

==============================================================================
*/

ProductionSiteDescr::ProductionSiteDescr
	(const std::string& init_descname, const std::string& msgctxt, MapObjectType _type,
	 const LuaTable& table, const EditorGameBase& egbase)
	: BuildingDescr(init_descname, _type, table, egbase),
	  m_out_of_resource_title(""),
	  m_out_of_resource_heading(""),
	  m_out_of_resource_message(""),
	  out_of_resource_productivity_threshold_(100)
{
	i18n::Textdomain td("tribes");
	std::unique_ptr<LuaTable> items_table;

	if (table.has_key("out_of_resource_notification")) {
		items_table = table.get_table("out_of_resource_notification");
		m_out_of_resource_title = _(items_table->get_string("title"));
		m_out_of_resource_heading = _(items_table->get_string("heading"));
		m_out_of_resource_message = pgettext_expr(msgctxt.c_str(), items_table->get_string("message").c_str());
		if (items_table->has_key("productivity_threshold")) {
			out_of_resource_productivity_threshold_ = items_table->get_int("productivity_threshold");
		}
	}

	// TODO(GunChleoc): This should not be here for Militarysites.
	// Check if they can inherit from Building directly.
	if (table.has_key("outputs")) {
		for (const std::string& output : table.get_table("outputs")->array_entries<std::string>()) {
			try {
				DescriptionIndex idx = egbase.tribes().ware_index(output);
				if (egbase.tribes().ware_exists(idx)) {
					if (m_output_ware_types.count(idx)) {
						throw wexception("this ware type has already been declared as an output");
					}
					m_output_ware_types.insert(idx);
				} else {
					idx = egbase.tribes().worker_index(output);
					if (egbase.tribes().worker_exists(idx)) {
						if (m_output_worker_types.count(idx)) {
							throw wexception("this worker type has already been declared as an output");
						}
						m_output_worker_types.insert(idx);
					} else {
						throw wexception("tribes do not define a ware or worker type with this name");
					}
				}
			} catch (const WException & e) {
				throw wexception("output \"%s\": %s", output.c_str(), e.what());
			}
		}
	}

	if (table.has_key("inputs")) {
		items_table = table.get_table("inputs");
		for (const std::string& ware_name : items_table->keys<std::string>()) {
			int amount = items_table->get_int(ware_name);
			try {
				if (amount < 1 || 255 < amount) {
					throw wexception("count is out of range 1 .. 255");
				}
				DescriptionIndex const idx = egbase.tribes().ware_index(ware_name);
				if (egbase.tribes().ware_exists(idx)) {
					for (const WareAmount& temp_inputs : inputs()) {
						if (temp_inputs.first == idx) {
							throw wexception("duplicated");
						}
					}
					m_inputs.push_back(WareAmount(idx, amount));
				} else {
					throw wexception
						("tribes do not define a ware type with this name");
				}
			} catch (const WException & e) {
				throw wexception("input \"%s=%d\": %s", ware_name.c_str(), amount, e.what());
			}
		}
	}


	// Are we only a production site?
	// If not, we might not have a worker
	if (table.has_key("working_positions")) {
		items_table = table.get_table("working_positions");
		for (const std::string& worker_name : items_table->keys<std::string>()) {
			int amount = items_table->get_int(worker_name);
			try {
				if (amount < 1 || 255 < amount) {
					throw wexception("count is out of range 1 .. 255");
				}
				DescriptionIndex const woi = egbase.tribes().worker_index(worker_name);
				if (egbase.tribes().worker_exists(woi)) {
					for (const WareAmount& wp : working_positions()) {
						if (wp.first == woi) {
							throw wexception("duplicated");
						}
					}
					m_working_positions.push_back(std::pair<DescriptionIndex, uint32_t>(woi, amount));
				} else {
					throw wexception("invalid");
				}
			} catch (const WException & e) {
				throw wexception("%s=\"%d\": %s", worker_name.c_str(), amount, e.what());
			}
		}
	}

	// TODO(SirVer): this mixes militarysite concepts into the production site
	// - maybe those building should not be in a inheritance relationship.
	if (working_positions().empty() && !table.has_key("max_soldiers")) {
		throw wexception("no working/soldier positions");
	}

	// Get programs
	if (table.has_key("programs")) {
		items_table = table.get_table("programs");
		for (std::string program_name : items_table->keys<std::string>()) {
			std::transform
				(program_name.begin(), program_name.end(), program_name.begin(),
				 tolower);
			try {
				if (m_programs.count(program_name)) {
					throw wexception("this program has already been declared");
				}
				std::unique_ptr<LuaTable> program_table = items_table->get_table(program_name);
				m_programs[program_name] =
						std::unique_ptr<ProductionProgram>(
							new ProductionProgram(program_name,
														 program_table->get_string("descname"),
														 program_table->get_table("actions"),
														 egbase,
														 this));
			} catch (const std::exception & e) {
				throw wexception("program %s: %s", program_name.c_str(), e.what());
			}
		}
	}
}

ProductionSiteDescr::ProductionSiteDescr
	(const std::string& init_descname,
	 const std::string& msgctxt,
	 const LuaTable& table,
	 const EditorGameBase& egbase)
	: ProductionSiteDescr(init_descname, msgctxt, MapObjectType::PRODUCTIONSITE, table, egbase)
{}


/**
 * Get the program of the given name.
 */
const ProductionProgram * ProductionSiteDescr::get_program
	(const std::string & program_name) const
{
	Programs::const_iterator const it = programs().find(program_name);
	if (it == m_programs.end())
		throw wexception
			("%s has no program '%s'", name().c_str(), program_name.c_str());
	return it->second.get();
}

/**
 * Create a new building of this type
 */
Building & ProductionSiteDescr::create_object() const {
	return *new ProductionSite(*this);
}


/*
==============================

IMPLEMENTATION

==============================
*/

ProductionSite::ProductionSite(const ProductionSiteDescr & ps_descr) :
	Building            (ps_descr),
	m_working_positions (new WorkingPosition[ps_descr.nr_working_positions()]),
	m_fetchfromflag     (0),
	m_program_timer     (false),
	m_program_time      (0),
	m_post_timer        (50),
	m_statistics        (STATISTICS_VECTOR_LENGTH, false),
	m_last_stat_percent (0),
	m_crude_percent     (0),
	m_is_stopped        (false),
	m_default_anim      ("idle"),
	m_production_result ("")
{
	calc_statistics();
}

ProductionSite::~ProductionSite() {
	// TODO(sirver): Use std::vector<std::unique_ptr<>> to avoid naked delete.
	delete[] m_working_positions;
}

void ProductionSite::load_finish(EditorGameBase & egbase) {
	Building::load_finish(egbase);
	calc_statistics();
}


/**
 * Display whether we're occupied.
 */
void ProductionSite::update_statistics_string(std::string* s) {
	uint32_t const nr_working_positions = descr().nr_working_positions();
	uint32_t nr_workers = 0;
	for (uint32_t i = nr_working_positions; i;)
		nr_workers += m_working_positions[--i].worker ? 1 : 0;

	if (nr_workers == 0) {
		*s = (boost::format("<font color=%s>%s</font>") % UI_FONT_CLR_BAD.hex_value() % _("(not occupied)"))
		        .str();
		return;
	}

	if (uint32_t const nr_requests = nr_working_positions - nr_workers) {
		*s = (boost::format("<font color=%s>%s</font>") % UI_FONT_CLR_BAD.hex_value() %
		      ngettext("Worker missing", "Workers missing", nr_requests)).str();
		return;
	}

	if (m_is_stopped) {
		*s = (boost::format("<font color=%s>%s</font>") % UI_FONT_CLR_BRIGHT.hex_value() % _("(stopped)"))
		        .str();
		return;
	}
	*s = m_statistics_string_on_changed_statistics;
}

/**
 * Detect if the workers are experienced enough for an target building
 * Buildable workers are skipped, but upgraded ones (required be target site) are tested
 * @param idx Index of the enhancement
 */
bool ProductionSite::has_workers(DescriptionIndex targetSite, Game & /* game */)
{
	// bld holds the description of the building we want to have
	if (upcast(ProductionSiteDescr const, bld, owner().tribe().get_building_descr(targetSite))) {

		if (bld->nr_working_positions()) {

			// Iterating over workers positions in target building
			for (const auto& wp : bld->working_positions()) {

				// If worker for this position is buildable, just skip him
				if (owner().tribe().get_worker_descr(wp.first)->is_buildable()){
					continue;
				}

				// This position needs promoted worker, so trying to find out if there is such worker
				// currently available in this site
				const DescriptionIndex needed_worker = wp.first;
				bool worker_available =  false;
				for (unsigned int i = 0; i < descr().nr_working_positions(); ++i) {
					const Worker* cw = working_positions()[i].worker;
					if (cw) {
						DescriptionIndex current_worker = cw->descr().worker_index();
						if (owner().tribe().get_worker_descr(current_worker)->can_act_as(needed_worker)) {
							worker_available = true; // We found a worker for the position
							break;
						}
					}
				}
				if (!worker_available) {
					// We dont have needed workers in the site :(
					return false;
				}

			}

			//if we are here, all needs are satisfied
			return true;

		} else {
			throw wexception("Building, index: %d, needs no workers!\n", targetSite);
		}
	} else {
		throw wexception("No such building, index: %d\n", targetSite);
	}
}

WaresQueue & ProductionSite::waresqueue(DescriptionIndex const wi) {
	for (WaresQueue * ip_queue : m_input_queues) {
		if (ip_queue->get_ware() == wi) {
			return *ip_queue;
		}
	}
	throw wexception("%s (%u) has no WaresQueue for %u", descr().name().c_str(), serial(), wi);
}

/**
 * Calculate statistic.
 */
void ProductionSite::calc_statistics()
{
	// TODO(sirver): this method does too much: it calculates statistics for the
	// last few cycles, but it also formats them as a string and persists them
	// into a string for reuse when the class is asked for the statistics
	// string. However this string should only then be constructed.
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
	// boost::format would treat uint8_t as char
	const unsigned int percOk = (ok * 100) / STATISTICS_VECTOR_LENGTH;
	m_last_stat_percent = percOk;

	const unsigned int lastPercOk = (lastOk * 100) / (STATISTICS_VECTOR_LENGTH / 2);

	std::string color;
	if (percOk < 33)
		color = UI_FONT_CLR_BAD.hex_value();
	else if (percOk < 66)
		color = UI_FONT_CLR_OK.hex_value();
	else
		color = UI_FONT_CLR_GOOD.hex_value();
	const std::string perc_str =
		(boost::format("<font color=%s>%s</font>") % color % (boost::format(_("%i%%")) % percOk)).str();

	std::string trend;
	if (lastPercOk > percOk) {
		trend_ = Trend::kRising;
		color = UI_FONT_CLR_GOOD.hex_value();
		trend = "+";
	} else if (lastPercOk < percOk) {
		trend_ = Trend::kFalling;
		color = UI_FONT_CLR_BAD.hex_value();
		trend = "-";
	} else {
		trend_ = Trend::kUnchanged;
		color = UI_FONT_CLR_BRIGHT.hex_value();
		trend = "=";
	}
	const std::string trend_str =
		(boost::format("<font color=%s>%s</font>") % color % trend).str();

	if (0 < percOk && percOk < 100) {
		// TODO(GunChleoc): We might need to reverse the order here for RTL languages
		m_statistics_string_on_changed_statistics = (boost::format("%s %s") % perc_str % trend_str).str();
	} else {
		m_statistics_string_on_changed_statistics = perc_str;
	}
}


/**
 * Initialize the production site.
 */
void ProductionSite::init(EditorGameBase & egbase)
{
	Building::init(egbase);

	const BillOfMaterials & inputs = descr().inputs();
	m_input_queues.resize(inputs.size());
	for (WareRange i(inputs); i; ++i)
		m_input_queues[i.i] =
			new WaresQueue
			(*this,
			 i.current->first,
			 i.current->second);

	//  Request missing workers.
	WorkingPosition * wp = m_working_positions;
	for (const WareAmount& temp_wp : descr().working_positions()) {
		DescriptionIndex const worker_index = temp_wp.first;
		for (uint32_t j =  temp_wp.second; j; --j, ++wp)
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
	if (Economy * const old = get_economy()) {
		for (WaresQueue * ip_queue : m_input_queues) {
			ip_queue->remove_from_economy(*old);
		}
	}

	Building::set_economy(e);
	for (uint32_t i = descr().nr_working_positions(); i;)
		if (Request * const r = m_working_positions[--i].worker_request)
			r->set_economy(e);

	if (e) {
		for (WaresQueue * ip_queue : m_input_queues) {
			ip_queue->add_to_economy(*e);
		}
	}
}

/**
 * Cleanup after a production site is removed
 */
void ProductionSite::cleanup(EditorGameBase & egbase)
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
	(EditorGameBase & egbase, const WorkerDescr & wdes)
{
	bool assigned = false;
	WorkingPosition * current = m_working_positions;
	for
		(WorkingPosition * const end = current + descr().nr_working_positions();
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
	if (!assigned)
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
	molog("%s leaving\n", w.descr().descname().c_str());
	WorkingPosition * wp = m_working_positions;

	for (const WareAmount& temp_wp : descr().working_positions()) {
		DescriptionIndex const worker_index = temp_wp.first;
		for (uint32_t j = temp_wp.second; j; --j, ++wp) {
			Worker * const worker = wp->worker;
			if (worker && worker == &w) {
				// do not request the type of worker that is currently assigned - maybe a trained worker was
				// evicted to make place for a level 0 worker.
				// Therefore we again request the worker from the WorkingPosition of descr()
				*wp = WorkingPosition(&request_worker(worker_index), nullptr);
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
Request & ProductionSite::request_worker(DescriptionIndex const wareid) {
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
	 DescriptionIndex              /* widx */,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	ProductionSite & psite = dynamic_cast<ProductionSite&>(target);

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
	DescriptionIndex     idx = w->descr().worker_index();
	for (WorkingPosition * wp = psite.m_working_positions;; ++wp) {
		if (wp->worker_request == &rq) {
			if (wp->worker_request->get_index() == idx) {
				// Place worker
				delete &rq;
				*wp = WorkingPosition(nullptr, w);
				worker_placed = true;
			} else {
				// Set new request for this slot
				DescriptionIndex workerid = wp->worker_request->get_index();
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
			WorkingPosition * wp = psite.m_working_positions;
			for (; pos < nwp; ++wp, ++pos) {
				// Find a fitting slot
				if (!wp->worker && !worker_placed)
					if (wp->worker_request->get_index() == idx) {
						delete wp->worker_request;
						*wp = WorkingPosition(nullptr, w);
						worker_placed = true;
						break;
					}
			}
		}
		if (!worker_placed) {
			// Find the next smaller version of this worker
			DescriptionIndex nuwo    = game.tribes().nrworkers();
			DescriptionIndex current = 0;
			for (; current < nuwo; ++current) {
				WorkerDescr const * worker = game.tribes().get_worker_descr(current);
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
		 &&
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
			if (state.program->size() <= state.ip)
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

	// 'Stop' of building is considered only when starting
	// new productions cycle. Otherwise it can lead to consumption
	// of input wares without producing anything
	if (m_is_stopped && state.ip == 0) {
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


void ProductionSite::log_general_info(const EditorGameBase & egbase) {
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
	if (can_start_working() && descr().working_positions().size()) {
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
		if (state->ip < state->program->size())
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
		WareAmount& ware_type_with_count = *m_produced_wares.rbegin();
		{
			DescriptionIndex const ware_index = ware_type_with_count.first;
			const WareDescr & ware_ware_descr =
				*owner().tribe().get_ware_descr(ware_type_with_count.first);
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
		WareAmount& worker_type_with_count = *m_recruited_workers.rbegin();
		{
			const WorkerDescr & worker_descr =
				*owner().tribe().get_worker_descr(worker_type_with_count.first);
			{
				Worker & recruit =
					dynamic_cast<Worker&>(worker_descr.create_object());
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
	for (WaresQueue * queue : m_input_queues) {
		if (queue->get_filled() > queue->get_max_fill()) {
			queue->set_filled(queue->get_filled() - 1);
			const WareDescr & wd = *owner().tribe().get_ware_descr(queue->get_ware());
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
	} else if (state->ip < state->program->size()) {
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
	SkippedPrograms::const_iterator i = m_skipped_programs.find(program_name);
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
void ProductionSite::program_end(Game & game, ProgramResult const result)
{
	assert(m_stack.size());

	const std::string & program_name = top_state().program->name();

	m_stack.pop_back();
	if (!m_stack.empty())
		top_state().phase = result;

	switch (result) {
	case Failed:
		//changed by TB below
		m_statistics.erase(m_statistics.begin(), m_statistics.begin() + 1);
		m_statistics.push_back(false);
		calc_statistics();
		m_crude_percent = m_crude_percent * 8 / 10;
		break;
		//end of changed by TB
	case Completed:
		m_skipped_programs.erase(program_name);
		m_statistics.erase(m_statistics.begin(), m_statistics.begin() + 1);
		m_statistics.push_back(true);
		train_workers(game);
		m_crude_percent = m_crude_percent  * 8 / 10 + 1000000 * 2 / 10;
		calc_statistics();
		break;
	case Skipped:
		m_skipped_programs[program_name] = game.get_gametime();
		//changed by TB below
		m_crude_percent = m_crude_percent * 98 / 100;
		//end of changed by TB
		break;
	case None:
		break;
	}

	m_program_timer = true;
	m_program_time = schedule_act(game, m_post_timer);
}

void ProductionSite::train_workers(Game & game)
{
	for (uint32_t i = descr().nr_working_positions(); i;)
		m_working_positions[--i].worker->gain_experience(game);
	Building::workers_changed();
}


void ProductionSite::notify_player(Game & game, uint8_t minutes)
{
	if (m_last_stat_percent == 0 ||
		 (m_last_stat_percent <= descr().out_of_resource_productivity_threshold()
		  && trend_ == Trend::kFalling)) {
		if (descr().out_of_resource_heading().empty())
		{
			set_production_result(_("Can’t find any more resources!"));
		}
		else {
			set_production_result(descr().out_of_resource_heading());

			assert(!descr().out_of_resource_message().empty());
			send_message
				(game,
				 Message::Type::kEconomy,
				 descr().out_of_resource_title(),
				 descr().icon_filename(),
				 descr().out_of_resource_heading(),
				 descr().out_of_resource_message(),
				 true,
				 minutes * 60000, 0);
		}
		// The following sends "out of resources" messages to be picked up by AI
		// used as information for dismantling and upgrading buildings
		if (descr().get_ismine()) {
			Notifications::publish(NoteProductionSiteOutOfResources(this, get_owner()));
		}
	}
}

void ProductionSite::unnotify_player() {
	 set_production_result("");
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
