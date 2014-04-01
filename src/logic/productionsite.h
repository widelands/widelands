/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef PRODUCTIONSITE_H
#define PRODUCTIONSITE_H

#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "logic/bill_of_materials.h"
#include "logic/building.h"
#include "logic/production_program.h"
#include "logic/program_result.h"

namespace Widelands {

struct WareDescr;
struct ProductionProgram;
class Soldier;
class Request;
struct WaresQueue;
class Worker_Descr;


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
		 const std::string & directory, Profile &, Section & global_s,
		 const Tribe_Descr &);
	virtual ~ProductionSite_Descr();

	virtual Building & create_object() const override;

	uint32_t nr_working_positions() const {
		uint32_t result = 0;
		container_iterate_const(BillOfMaterials, working_positions(), i)
			result += i.current->second;
		return result;
	}
	const BillOfMaterials & working_positions() const {
		return m_working_positions;
	}
	bool is_output_ware_type  (const Ware_Index& i) const {
		return m_output_ware_types  .count(i);
	}
	bool is_output_worker_type(const Ware_Index& i) const {
		return m_output_worker_types.count(i);
	}
	const BillOfMaterials & inputs() const {return m_inputs;}
	typedef std::set<Ware_Index>                       Output;
	const Output   & output_ware_types  () const {return m_output_ware_types;}
	const Output   & output_worker_types() const {return m_output_worker_types;}
	const ProductionProgram * get_program(const std::string &) const;
	typedef std::map<std::string, ProductionProgram *> Programs;
	const Programs & programs() const {return m_programs;}

private:
	BillOfMaterials m_working_positions;
	BillOfMaterials m_inputs;
	Output   m_output_ware_types;
	Output   m_output_worker_types;
	Programs m_programs;
};

class ProductionSite : public Building {
	friend struct Map_Buildingdata_Data_Packet;
	friend struct ProductionProgram::ActReturn;
	friend struct ProductionProgram::ActReturn::Workers_Need_Experience;
	friend struct ProductionProgram::ActCall;
	friend struct ProductionProgram::ActWorker;
	friend struct ProductionProgram::ActSleep;
	friend struct ProductionProgram::ActCheck_Map;
	friend struct ProductionProgram::ActAnimate;
	friend struct ProductionProgram::ActConsume;
	friend struct ProductionProgram::ActProduce;
	friend struct ProductionProgram::ActRecruit;
	friend struct ProductionProgram::ActMine;
	friend struct ProductionProgram::ActCheck_Soldier;
	friend struct ProductionProgram::ActTrain;
	friend struct ProductionProgram::ActPlayFX;
	friend struct ProductionProgram::ActConstruct;
	MO_DESCR(ProductionSite_Descr);

public:
	ProductionSite(const ProductionSite_Descr & descr);
	virtual ~ProductionSite();

	void log_general_info(const Editor_Game_Base &) override;

	bool is_stopped() const {return m_is_stopped;}
	void set_stopped(bool);

	struct Working_Position {
		Working_Position(Request * const wr = nullptr, Worker * const w = nullptr)
			: worker_request(wr), worker(w)
		{}
		Request * worker_request;
		Worker  * worker;
	};

	Working_Position const * working_positions() const {
		return m_working_positions;
	}

	virtual std::string get_statistics_string() override;
	virtual bool has_workers(Building_Index targetSite, Game & game);
	uint8_t get_statistics_percent() {return m_last_stat_percent;}
	char const * result_string() const {return m_result_buffer;}

	virtual WaresQueue & waresqueue(Ware_Index) override;

	char const * type_name() const override {return "productionsite";}
	virtual void init(Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;
	virtual void act(Game &, uint32_t data) override;

	virtual void remove_worker(Worker &) override;
	int warp_worker(Editor_Game_Base &, const Worker_Descr & wd);

	virtual bool fetch_from_flag(Game &) override;
	virtual bool get_building_work(Game &, Worker &, bool success) override;

	virtual void set_economy(Economy *) override;

	typedef std::vector<WaresQueue *> Input_Queues;
	const Input_Queues & warequeues() const {return m_input_queues;}
	const std::vector<Worker *>& workers() const;

	bool can_start_working() const;

	void set_default_anim(std::string);

protected:
	virtual void create_options_window
		(Interactive_GameBase &, UI::Window * & registry) override;

protected:
	struct State {
		const ProductionProgram * program; ///< currently running program
		int32_t  ip; ///< instruction pointer
		uint32_t phase; ///< micro-step index (instruction dependent)
		uint32_t flags; ///< pfXXX flags

		/**
		 * Instruction-dependent additional data.
		 */
		/*@{*/
		Object_Ptr objvar;
		Coords coord;
		/*@}*/

		State() :
			program(nullptr),
			ip(0),
			phase(0),
			flags(0),
			coord(Coords::Null()) {}
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
	State * get_state() {return m_stack.size() ? &*m_stack.rbegin() : nullptr;}
	void program_act(Game &);

	/// \param phase can be used to pass a value on to the next step in the
	/// program. For example if one step is a mine command, it can calculate
	/// how long it should take to mine, given the particular circumstances,
	/// and pass the result to the following animation command, to set the
	/// duration.
	void program_step(Game &, uint32_t delay = 10, uint32_t phase = 0);

	void program_start(Game &, const std::string & program_name);
	virtual void program_end(Game &, Program_Result);

	void calc_statistics();
	void try_start_working(Game &);
	void set_post_timer (int32_t const t) {m_post_timer = t;}

protected:  // TrainingSite must have access to this stuff
	Working_Position                   * m_working_positions;

	int32_t m_fetchfromflag; ///< Number of wares to fetch from flag

	/// If a program has ended with the result Skipped, that program may not
	/// start again until a certain time has passed. This is a map from program
	/// name to game time. When a program ends with the result Skipped, its name
	/// is added to this map, with the current game time. (When the program ends
	/// with any other result, its name is removed from the map.)
	typedef std::map<std::string, Time> Skipped_Programs;
	Skipped_Programs m_skipped_programs;

	typedef std::vector<State> Stack;
	Stack        m_stack; ///<  program stack
	bool         m_program_timer; ///< execute next instruction based on pointer
	int32_t      m_program_time; ///< timer time
	int32_t      m_post_timer;    ///< Time to schedule after ends

	ProductionProgram::ActProduce::Items m_produced_wares;
	ProductionProgram::ActProduce::Items m_recruited_workers;
	Input_Queues m_input_queues; ///< input queues for all inputs
	std::vector<bool>        m_statistics;
	bool                     m_statistics_changed;
	char                     m_statistics_buffer[128];
	char                     m_result_buffer   [213];
	uint8_t                  m_last_stat_percent;
	bool                     m_is_stopped;
	std::string              m_default_anim; // normally "idle", "empty", if empty mine.
};

/**
 * Describes, how many wares of a certain ware can be stored in a house.
 *
 * This class will be extended to support ordering of certain wares directly or
 * releasing some wares out of a building
*/
struct Input {
	Input(const Ware_Index& Ware, uint8_t const Max) : m_ware(Ware), m_max(Max)
	{}
	~Input() {}

	Ware_Index ware() const {return m_ware;}
	uint8_t     max() const {return m_max;}

private:
	Ware_Index m_ware;
	uint8_t    m_max;
};

}

#endif
