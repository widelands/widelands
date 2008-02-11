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

#ifndef PRODUCTIONSITE_H
#define PRODUCTIONSITE_H

#include "building.h"
#include "wexception.h"

#include <map>
#include <set>
#include <string>
#include <cstring>
#include <vector>

namespace Widelands {

class Input;
class Item_Ware_Descr;
class ProductionProgram;
class Soldier;
class Request;
class WaresQueue;


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
	friend class ProductionProgram; // To add animations
	typedef std::map<std::string, ProductionProgram*> ProgramMap;

	ProductionSite_Descr
		(const Tribe_Descr & tribe, const std::string & productionsite_name);
	virtual ~ProductionSite_Descr();

	virtual void parse(char const * directory, Profile *, EncodeData const *);
	virtual Building * create_object() const;

	std::vector<std::string> const & workers() const throw () {
		return m_workers;
	}
	bool is_output(const std::string & warename) const throw ()
	{return m_output.find(warename) != m_output.end();}
	const std::set<std::string>* get_outputs() const {return &m_output;}
	const std::vector<Input>* get_inputs() const {return &m_inputs;}
	const ProductionProgram * get_program(const std::string &) const;
	const ProgramMap & get_all_programs() const throw () {return m_programs;}

	virtual bool is_only_production_site() const throw () {return true;}


private:
	std::vector<std::string> m_workers; // name of worker type
	std::vector<Input>    m_inputs;
	std::set<std::string> m_output;      // output wares type names
	ProgramMap            m_programs;
};

class ProductionSite : public Building {
	friend struct Map_Buildingdata_Data_Packet;
	MO_DESCR(ProductionSite_Descr);

public:
	ProductionSite(const ProductionSite_Descr & descr);

	virtual std::string get_statistics_string();
	int8_t get_statistics_percent() {return m_last_stat_percent;}

	void fill(Game &);

	virtual int32_t get_building_type() const throw ()
	{return Building::PRODUCTIONSITE;}
	virtual void init(Editor_Game_Base* g);
	virtual void cleanup(Editor_Game_Base* g);
	virtual void act(Game* g, uint32_t data);

	virtual void remove_worker(Worker* w);

	virtual bool fetch_from_flag(Game* g);
	virtual bool get_building_work(Game* g, Worker* w, bool success);

	virtual void set_economy(Economy* e);

	std::vector<WaresQueue*>* get_warequeues() {
		return &m_input_queues;
	}
	std::vector<Worker*>* get_workers() {
		return &m_workers;
	}

	__attribute__ ((noreturn)) virtual const std::vector<Soldier *> & get_soldiers() const
	{throw wexception ("ProductionSite::get_soldiers makes no sense");}
protected:
	virtual UI::Window * create_options_window
		(Interactive_Player *, UI::Window * * registry);

protected:
	struct State {
		const ProductionProgram * program; //  currently running program
		int32_t  ip;    //  instruction pointer
		uint32_t phase; //  micro-step index (instruction dependent)
		uint32_t flags; //  pfXXX flags
	};

	void request_worker(const char * const worker_name);
	static void request_worker_callback
		(Game *, Request *, Ware_Index, Worker *, void * data);

	State* get_current_program() {return m_program.size() ? &*m_program.rbegin() : 0;}
	void program_act(Game* g);
	void program_step(const uint32_t phase = 0);
	void program_start(Game* g, std::string name);
	void program_end(Game* g, bool success);
	void add_statistics_value(bool val);

	void calc_statistics();
	bool can_start_working() const throw ();
	void set_post_timer (int32_t t) {m_post_timer = t;}

protected:  // TrainingSite must have access to this stuff
	std::vector<Request*> m_worker_requests;
	std::vector<Worker *> m_workers;

	int32_t m_fetchfromflag; // # of items to fetch from flag

	std::vector<State>        m_program;       //  program stack
	bool                     m_program_timer; // execute next instruction based on pointer
	int32_t                       m_program_time;  //  timer time
	int32_t                      m_post_timer;    // Time to schedule after ends

	std::vector<WaresQueue*> m_input_queues; //  input queues for all inputs
	std::vector<bool>        m_statistics;
	bool                     m_statistics_changed;
	char                     m_statistics_buf[40];
   int8_t                     m_last_stat_percent;
};

/*
=============================

class Input

This class descripes, how many items of a certain
ware can be stored in a house.
This class will be extended to support ordering of
certain wares directly or releasing some wares
out of a building

=============================
*/
struct Input {
	Input(Item_Ware_Descr* ware, int32_t max) : m_ware(ware), m_max(max) {}
	~Input() {}

	const Item_Ware_Descr & ware_descr() const throw () {return *m_ware;}
	uint32_t get_max() const throw () {return m_max;}

private:
	Item_Ware_Descr* m_ware;
	int32_t m_max;
};

};

#endif
