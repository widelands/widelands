/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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

#ifndef S__FLAG_H
#define S__FLAG_H

#include <vector>
#include <list>

#include "logic/immovable.h"
#include "routing_node.h"

namespace Widelands {
struct Building;
struct Request;
struct Road;
struct WareInstance;



/**
 * Flag represents a flag, obviously.
 * A flag itself doesn't do much. However, it can have up to 6 roads attached
 * to it. Instead of the WALK_NW road, it can also have a building attached to
 * it.
 * Flags also have a store of up to 8 wares.
 *
 * You can also assign an arbitrary number of "jobs" for a flag.
 * A job consists of a request for a worker, and the name of a program that the
 * worker is to execute. Once execution of the program has finished, the worker
 * will return to a warehouse.
 *
 * Important: Do not access m_roads directly. get_road() and others use
 * Map_Object::WALK_xx in all "direction" parameters.
 */
struct Flag : public PlayerImmovable, public RoutingNode {
	friend struct Economy;
	friend struct Router;
	friend class FlagQueue;
	friend struct Map_Ware_Data_Packet;     // has to look at pending items
	friend struct Map_Waredata_Data_Packet; // has to look at pending items
	friend struct Map_Flagdata_Data_Packet; // has to read/write this to a file

	Flag(); /// empty flag for savegame loading
	Flag(Game &, Player & owner, Coords); /// create a new flag
	virtual ~Flag();


	virtual int32_t  get_type    () const throw ();
	char const * type_name() const throw () {return "flag";}
	virtual int32_t  get_size    () const throw ();
	virtual bool get_passable() const throw ();
	std::string const & name() const throw ();

	virtual Flag & base_flag();

	Coords get_position() const {return m_position;}
	void get_neighbours(RoutingNodeNeighbours &);
	int32_t get_waitcost() const {return m_item_filled;}

	virtual void set_economy(Economy *);

	Building * get_building() const {return m_building;}
	void attach_building(Editor_Game_Base &, Building &);
	void detach_building(Editor_Game_Base &);

	bool has_road() const {
		return
			m_roads[0] or m_roads[1] or m_roads[2] or
			m_roads[3] or m_roads[4] or m_roads[5];
	}
	Road * get_road(uint8_t const dir) const {return m_roads[dir - 1];}
	void attach_road(int32_t dir, Road *);
	void detach_road(int32_t dir);

	Road * get_road(Flag &);

	bool is_dead_end() const;

	bool has_capacity();
	void wait_for_capacity(Game &, Worker &);
	void skip_wait_for_capacity(Game &, Worker &);
	void add_item(Game &, WareInstance &);
	bool has_pending_item(Game &, Flag & destflag);
	bool ack_pending_item(Game &, Flag & destflag);
	WareInstance * fetch_pending_item(Game &, PlayerImmovable & dest);

	void call_carrier(Game &, WareInstance &, PlayerImmovable * nextstep);
	void update_items(Game &, Flag * other);

	void remove_item(Game &, WareInstance *);

	void add_flag_job
		(Game &, Ware_Index workerware, std::string const & programname);


protected:
	virtual void init(Editor_Game_Base &);
	virtual void cleanup(Editor_Game_Base &);
	virtual void destroy(Editor_Game_Base &);

	virtual void draw(Editor_Game_Base const &, RenderTarget &, FCoords, Point);

	void wake_up_capacity_queue(Game &);

	static void flag_job_request_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

	void set_flag_position(Coords coords);

private:
	struct PendingItem {
		WareInstance    * item;     ///< the item itself
		bool              pending;  ///< if the item is pending
		OPtr<PlayerImmovable> nextstep; ///< next step that this item is sent to
	};

	struct FlagJob {
		Request *   request;
		std::string program;
	};

	Coords       m_position;
	int32_t      m_animstart;

	Building    * m_building; ///< attached building (replaces road WALK_NW)
	Road        * m_roads[6]; ///< Map_Object::WALK_xx - 1 as index
	int32_t      m_items_pending[6];

	int32_t      m_item_capacity; ///< size of m_items array
	int32_t      m_item_filled; ///< number of items currently on the flag
	PendingItem * m_items;    ///< items currently on the flag

	/// call_carrier() will always call a carrier when the destination is
	/// the given flag
	Flag        * m_always_call_for_flag;

	typedef std::vector<OPtr<Worker> > CapacityWaitQueue;
	CapacityWaitQueue m_capacity_wait; ///< workers waiting for capacity

	typedef std::list<FlagJob> FlagJobs;
	FlagJobs m_flag_jobs;
};

}

#endif
