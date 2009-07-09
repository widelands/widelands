/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef PRODUCTIONSITE_H
#define PRODUCTIONSITE_H

#include "logic/building.h"
#include "production_program.h"
#include "program_result.h"
#include "ware_types.h"
#include "wexception.h"

#include <map>
#include <set>
#include <string>
#include <cstring>
#include <vector>

namespace Widelands {

struct Item_Ware_Descr;
struct ProductionProgram;
class Soldier;
struct Request;
struct WaresQueue;


/**
 * Every building that is part of the economics system is a production site.
 *
 * A production site has a worker.
 * A production site can have one (or more) output wares types (in theory it
 * should be possible to burn wares for some virtual result such as "mana", or
 *  maybe even just for the fun of it, although that's not planned).
 * A production site can have one (or more) input wares types. Every input
 * wares type has an associated store.
 */
struct ProductionSite_Descr : public Building_Descr {
	friend struct ProductionProgram; // To add animations

	ProductionSite_Descr
		(char const * name, char const * descname,
		 std::string const & directory, Profile &, Section & global_s,
		 Tribe_Descr const &, EncodeData const *);
	virtual ~ProductionSite_Descr();

#ifdef WRITE_GAME_DATA_AS_HTML
	void writeHTMLProduction(::FileWrite &) const;
#endif
	virtual Building & create_object() const;

	uint32_t nr_working_positions() const {
		uint32_t result = 0;
		container_iterate_const(Ware_Types, working_positions(), i)
			result += i.current->second;
		return result;
	}
	Ware_Types const & working_positions() const throw () {
		return m_working_positions;
	}
	bool is_output(Ware_Index const i) const throw () {
		return m_output.count(i);
	}
	Ware_Types const & inputs() const throw () {return m_inputs;}
	typedef std::set<Ware_Index>                       Output;
	Output   const & output  () const throw () {return m_output;}
	const ProductionProgram * get_program(const std::string &) const;
	typedef std::map<std::string, ProductionProgram *> Programs;
	Programs const & programs() const throw () {return m_programs;}

private:
	Ware_Types m_working_positions;
	Ware_Types m_inputs;
	Output   m_output;
	Programs m_programs;
};

class ProductionSite : public Building {
	friend struct Map_Buildingdata_Data_Packet;
	friend struct ProductionProgram::ActReturn;
	friend struct ProductionProgram::ActReturn::Workers_Need_Experience;
	friend struct ProductionProgram::ActCall;
	friend struct ProductionProgram::ActWorker;
	friend struct ProductionProgram::ActSleep;
	friend struct ProductionProgram::ActAnimate;
	friend struct ProductionProgram::ActConsume;
	friend struct ProductionProgram::ActProduce;
	friend struct ProductionProgram::ActMine;
	friend struct ProductionProgram::ActCheck_Soldier;
	friend struct ProductionProgram::ActTrain;
	friend struct ProductionProgram::ActPlayFX;
	MO_DESCR(ProductionSite_Descr);

public:
	ProductionSite(const ProductionSite_Descr & descr);
	virtual ~ProductionSite();

	void log_general_info(Editor_Game_Base &);

	bool is_stopped() const {return m_is_stopped;}
	void set_stopped(bool);

	struct Working_Position {
		Working_Position(Request * const wr = 0, Worker * const w = 0)
			: worker_request(wr), worker(w)
		{}
		Request * worker_request;
		Worker  * worker;
	};

	Working_Position const * working_positions() const {
		return m_working_positions;
	}

	virtual std::string get_statistics_string();
	int8_t get_statistics_percent() {return m_last_stat_percent;}

	virtual WaresQueue & waresqueue(Ware_Index);

	void prefill
		(Game &, uint32_t const *, uint32_t const *, Soldier_Counts const *);

	char const * type_name() const throw () {return "productionsite";}
	virtual void init(Editor_Game_Base &);
	virtual void cleanup(Editor_Game_Base &);
	virtual void act(Game &, uint32_t data);

	virtual void remove_worker(Worker &);

	virtual bool fetch_from_flag(Game &);
	virtual bool get_building_work(Game &, Worker &, bool success);

	virtual void set_economy(Economy *);

	typedef std::vector<WaresQueue *> Input_Queues;
	Input_Queues const & warequeues() const {return m_input_queues;}
	std::vector<Worker *> const & workers() const;

protected:
	virtual void create_options_window
		(Interactive_GameBase &, UI::Window * & registry);

protected:
	struct State {
		const ProductionProgram * program; ///< currently running program
		int32_t  ip; ///< instruction pointer
		uint32_t phase; ///< micro-step index (instruction dependent)
		uint32_t flags; ///< pfXXX flags
	};

	Request & request_worker(Ware_Index);
	static void request_worker_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

	/**
	 * Determine the next program to be run when the last program has finished.
	 * The default implementation starts program "work".
	 */
	virtual void find_and_start_next_program(Game &);

	State & top_state() {assert(m_stack.size()); return *m_stack.rbegin();}
	State * get_state() {return m_stack.size() ? &*m_stack.rbegin() : 0;}
	void program_act(Game &);

	/// \param phase can be used to pass a value on to the next step in the
	/// program. For example if one step is a mine command, it can calculate
	/// how long it should take to mine, given the particular circumstances,
	/// and pass the result to the following animation command, to set the
	/// duration.
	void program_step(Game &, uint32_t delay = 10, uint32_t phase = 0);

	void program_start(Game &, std::string const & program_name);
	virtual void program_end(Game &, Program_Result);

	void calc_statistics();
	bool can_start_working() const throw ();
	void try_start_working(Game &);
	void set_post_timer (int32_t t) {m_post_timer = t;}

protected:  // TrainingSite must have access to this stuff
	Working_Position                   * m_working_positions;

	int32_t m_fetchfromflag; ///< Number of items to fetch from flag

	typedef std::vector<State> Stack;
	Stack        m_stack; ///<  program stack
	bool         m_program_timer; ///< execute next instruction based on pointer
	int32_t      m_program_time; ///< timer time
	int32_t      m_post_timer;    ///< Time to schedule after ends

	ProductionProgram::ActProduce::Items m_produced_items;
	Input_Queues m_input_queues; ///< input queues for all inputs
	std::vector<bool>        m_statistics;
	bool                     m_statistics_changed;
	char                     m_statistics_buf[40];
	int8_t                   m_last_stat_percent;
	bool                      m_is_stopped;
};

/**
 * Describes, how many items of a certain ware can be stored in a house.
 *
 * This class will be extended to support ordering of certain wares directly or
 * releasing some wares out of a building
*/
struct Input {
	Input(Ware_Index const Ware, uint8_t const Max) : m_ware(Ware), m_max(Max)
	{}
	~Input() {}

	Ware_Index ware() const throw () {return m_ware;}
	uint8_t     max() const throw () {return m_max;}

private:
	Ware_Index m_ware;
	int32_t m_max;
};

};

#endif
