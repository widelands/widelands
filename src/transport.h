/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#ifndef included_transport_h
#define included_transport_h

#include "instances.h"
#include "map.h"


class Flag;
class Road;
class Request;
class Economy;
class Item_Ware_Descr;
class IdleWareSupply;

struct Neighbour {
	Flag	*flag;
	Road	*road;
	int	cost;
};
typedef std::vector<Neighbour> Neighbour_list;


/*
WareInstance
------------
WareInstance represents one item while it is being carried around.

The WareInstance never draws itself; the carrying worker or the current flag
location are responsible for that.

The location of a ware can be one of the following:
- a flag
- a worker that is currently carrying the ware
- a building; this should only be temporary until the ware is incorporated into
  the building somehow
*/
class WareInstance : public Map_Object {
public:
	WareInstance(int ware);
	~WareInstance();

	virtual int get_type();

	Map_Object* get_location(Editor_Game_Base* g) { return m_location.get(g); }
	Economy* get_economy() { return m_economy; }
	int get_ware() const { return m_ware; }
	Item_Ware_Descr* get_ware_descr() const { return m_ware_descr; }

	void init(Editor_Game_Base* g);
	void cleanup(Editor_Game_Base* g);
	void act(Game*, uint data);
	void update(Game*);

	void set_location(Game* g, Map_Object* loc);
	void set_economy(Economy* e);

	bool is_moving(Game* g);
	void cancel_moving(Game* g);
	PlayerImmovable* get_next_move_step(Game* g);
	PlayerImmovable* get_final_move_step(Game* g);

	void set_request(Game* g, Request* rq, const Route* route);
	void cancel_request(Game* g);

private:
	Object_Ptr			m_location;
	Economy*				m_economy;
	int					m_ware;
	Item_Ware_Descr*	m_ware_descr;

	Request*				m_request;
	IdleWareSupply*	m_supply;

	bool			m_return_watchdog;	// scheduled return-to-warehouse watchdog
	bool			m_flag_dirty;			// true if we need to tell the flag to take care of us

	bool			m_moving;
	Object_Ptr	m_move_destination;
	Route*		m_move_route;

private:
	static Map_Object_Descr	s_description;
};



/*
Flag represents a flag, obviously.
A flag itself doesn't do much. However, it can have up to 6 roads attached
to it. Instead of the WALK_NW road, it can also have a building attached to
it.
Flags also have a store of up to 8 wares.

Important: Do not access m_roads directly. get_road() and others use
Map_Object::WALK_xx in all "direction" parameters.
*/
class Flag : public PlayerImmovable {
	friend class Economy;
	friend class FlagQueue;

private:
	struct PendingItem {
		WareInstance*		item;			// the item itself
		bool					pending;		// if the item is pending
		PlayerImmovable*	nextstep;	// next step that this item is sent to
	};

public:
	Flag();
	virtual ~Flag();

	static Flag *create(Editor_Game_Base *g, Player *owner, Coords coords);

	virtual int get_type();
	virtual int get_size();
	virtual bool get_passable();

	virtual Flag *get_base_flag();

	inline const Coords &get_position() const { return m_position; }

	virtual void set_economy(Economy *e);

	inline Building *get_building() { return m_building; }
	void attach_building(Editor_Game_Base *g, Building *building);
	void detach_building(Editor_Game_Base *g);

	inline Road *get_road(int dir) { return m_roads[dir-1]; }
	void attach_road(int dir, Road *road);
	void detach_road(int dir);

	void get_neighbours(Neighbour_list *neighbours);
	Road *get_road(Flag *flag);

	bool has_capacity();
	void wait_for_capacity(Game* g, Worker* bob);
	void add_item(Game* g, WareInstance* item);
	bool has_pending_item(Game* g, Flag* destflag);
	bool ack_pending_item(Game* g, Flag* destflag);
	WareInstance* fetch_pending_item(Game* g, PlayerImmovable* dest);

	void call_carrier(Game* g, WareInstance* item, PlayerImmovable* nextstep);
	void update_items(Game* g, Flag* other);

protected:
	virtual void init(Editor_Game_Base*);
	virtual void cleanup(Editor_Game_Base*);
	virtual void destroy(Editor_Game_Base*);

	virtual void draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords, Point pos);

	void wake_up_capacity_queue(Game* g);

private:
	Coords			m_position;
	uint				m_anim;
	int				m_animstart;

	Building			*m_building;		// attached building (replaces road WALK_NW)
	Road*				m_roads[6];			// Map_Object::WALK_xx-1 as index
	int				m_items_pending[6];

	int				m_item_capacity;	// size of m_items array
	int				m_item_filled;		// number of items currently on the flag
	PendingItem*	m_items;				// items currently on the flag

	Flag*				m_always_call_for_flag;	// call_carrier() will always call a carrier when
														// the destination is the given flag

	std::vector<Object_Ptr>	m_capacity_wait;	// workers waiting for capacity

	// The following are only used during pathfinding
	uint				mpf_cycle;
	int				mpf_heapindex;
	int				mpf_realcost;	// real cost of getting to this flag
	Flag*				mpf_backlink;	// flag where we came from
	int				mpf_estimate;	// estimate of cost to destination

	inline int cost() const { return mpf_realcost+mpf_estimate; }
};

/*
Road is a special object which connects two flags.
The Road itself is never rendered; however, the appropriate Field::roads are
set to represent the road visually.
The actual steps involved in a road are stored as a Path from the staring flag
to the ending flag. Apart from that, however, the two flags are treated
exactly the same, as far as most transactions are concerned. There are minor
exceptions: placement of carriers if the path's length is odd, splitting
a road when a flag is inserted.

Every road has one or more Carriers attached to it.

All Workers on the Road are attached via add_worker()/remove_worker() in
PlayerImmovable.
*/
class Road : public PlayerImmovable {
public:
	enum FlagId {
		FlagStart = 0,
		FlagEnd = 1
	};

public:
	Road();
	virtual ~Road();

	static Road* create(Editor_Game_Base *g, int type, Flag* start, Flag* end, const Path &path);

	inline Flag* get_flag(FlagId flag) const { return m_flags[flag]; }

	virtual int get_type();
	virtual int get_size();
	virtual bool get_passable();

	virtual Flag* get_base_flag();

	virtual void set_economy(Economy *e);

	int get_cost(FlagId fromflag);
	inline const Path &get_path() const { return m_path; }
	inline int get_idle_index() const { return m_idle_index; }

	void presplit(Editor_Game_Base *g, Coords split);
	void postsplit(Editor_Game_Base *g, Flag *flag);

	bool notify_ware(Game* g, FlagId flagid);
	virtual void remove_worker(Worker *w);

protected:
	void set_path(Editor_Game_Base *g, const Path &path);

	void mark_map(Editor_Game_Base *g);
	void unmark_map(Editor_Game_Base *g);

	virtual void init(Editor_Game_Base *g);
	virtual void cleanup(Editor_Game_Base *g);

	void request_carrier(Game* g);
	static void request_carrier_callback(Game* g, Request* rq, int ware, Worker* w, void* data);

	virtual void draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords, Point pos);

private:
	int		m_type;			// use Field::Road_XXX
	Flag*		m_flags[2];		// start and end flag
	int		m_flagidx[2];	// index of this road in the flag's road array
	int		m_cost[2];		// cost for walking this road (0 = from start to end, 1 = from end to start)
	Path		m_path;			// path goes from start to end
	int		m_idle_index;	// index into path where carriers should idle

	uint			m_desire_carriers;	// total # of carriers we want (currently limited to 0 or 1)
	Object_Ptr	m_carrier;				// our carrier
	Request*		m_carrier_request;
};


/*
Route stores a route from flag to flag.
The number of steps is the number of flags stored minus one.
*/
class Route {
	friend class Economy;

public:
	Route();

	void clear();
	bool verify(Game *g);

	inline int get_totalcost() const { return m_totalcost; }
	inline int get_nrsteps() const { return m_route.size()-1; }
	Flag *get_flag(Game *g, int idx);

	void starttrim(int count);

private:
	int				m_totalcost;
	std::vector<Object_Ptr>		m_route;	// includes start and end flags
};


/*
A Supply is a virtual base class representing something that can offer
wares of any type for any purpose.

Subsequent calls to get_position() can return different results.

Important note: The implementation of Supply is responsible for adding
and removing itself from Economies. This rule holds true for Economy
changes.
*/
class Supply {
public:
	virtual PlayerImmovable* get_position(Game* g) = 0;
	virtual int get_amount(Game* g, int ware) = 0;

	virtual WareInstance* launch_item(Game* g, int ware) = 0;
	virtual Worker* launch_worker(Game* g, int ware) = 0;
};


/*
SupplyList is used in the Economy to keep track of supplies.
*/
class SupplyList {
public:
	SupplyList();
	~SupplyList();

	void add(Supply* supp);
	void remove(Supply* supp);

	inline int get_nrsupplies() const { return m_supplies.size(); }
	inline Supply* get_supply(int idx) const { return m_supplies[idx]; }

private:
	std::vector<Supply*>	m_supplies;
};


/*
A Request is issued whenever some object (road or building) needs a ware.

Requests are always created and destroyed by their owner, i.e. the target
building. The owner is also responsible for calling set_economy() when
its economy changes.
*/
class Request {
	friend class Economy;
	friend class RequestList;

public:
	enum {
		OPEN = 0,	// not fulfilled yet
		TRANSFER,	// corresponding ware/worker is on its way
		CLOSED,		// request successful, waiting for its deletion
	};

	typedef void (*callback_t)(Game*, Request*, int ware, Worker*, void* data);

public:
	Request(PlayerImmovable *target, int ware, callback_t cbfn, void* cbdata);
	~Request();

	PlayerImmovable* get_target(Game* g) { return m_target; }
	int get_ware() const { return m_ware; }
	int get_state() const { return m_state; }
	Economy* get_economy() const { return m_economy; }

	Flag *get_target_flag(Game *g);

	void set_economy(Economy* e);

	void start_transfer(Game *g, Supply* supp, Route *route);

public: // callbacks for WareInstance/Worker code
	void transfer_finish(Game *g);
	void transfer_fail(Game *g);

private:
	PlayerImmovable*	m_target;	// who requested it?
	Economy*				m_economy;
	int					m_ware;		// the ware type

	callback_t	m_callbackfn;	// called on request success
	void*			m_callbackdata;

	int			m_state;

	WareInstance*	m_item;			// non-null if ware is an item and transferring
	Worker*			m_worker;		// non-null if ware is a worker and transferring
};


/*
RequestList is used in the Economy to keep track of requests.
Maybe we can one day introduce prioritizing of requests.
*/
class RequestList {
public:
	RequestList();
	~RequestList();

	void add(Request *req);
	void remove(Request *req);

	inline int get_nrrequests() const { return m_requests.size(); }
	inline Request* get_request(int idx) const { return m_requests[idx]; }

private:
	std::vector<Request*>	m_requests;
};


/*
WaresQueue
----------
This micro storage room can hold any number of items of a fixed ware.

Note that you must call update() after changing the queue's size or filled
state using one of the set_*() functions.
*/
class WaresQueue {
public:
	typedef void (callback_t)(Game* g, WaresQueue* wq, int ware, void* data);

public:
	WaresQueue(PlayerImmovable* bld);
	~WaresQueue();

	int get_ware() const { return m_ware; }
	int get_size() const { return m_size; }
	int get_filled() const { return m_filled; }

	void init(Game*, int ware, int size);
	void cleanup(Game*);
	void update(Game*);

	void set_callback(callback_t* fn, void* data);

	void remove_from_economy(Economy* e);
	void add_to_economy(Economy* e);

	void set_size(int size);
	void set_filled(int size);

private:
	static void request_callback(Game* g, Request* rq, int ware, Worker* w, void* data);

private:
	PlayerImmovable*	m_owner;
	int					m_ware;		// ware ID
	int					m_size;		// # of items that fit into the queue
	int					m_filled;	// # of items that are currently in the queue
	Request*				m_request;	// currently pending request

	callback_t*			m_callback_fn;
	void*					m_callback_data;
};


/*
Economy represents a network of Flag through which wares can be transported.
*/
class Economy {
	friend class Request;

public:
	Economy(Player *player);
	~Economy();

	inline Player *get_owner() const { return m_owner; }
	uint get_serial() const { return m_trackserial; }

	static void check_merge(Flag *f1, Flag *f2);
	static void check_split(Flag *f1, Flag *f2);

	bool find_route(Flag *start, Flag *end, Route *route, int cost_cutoff = -1);
	Warehouse *find_nearest_warehouse(Flag *base, Route *route);

	inline int get_nrflags() const { return m_flags.size(); }
	void add_flag(Flag *flag);
	void remove_flag(Flag *flag);

	void add_wares(int id, int count = 1);
	void remove_wares(int id, int count = 1);

	void add_warehouse(Warehouse *wh);
	void remove_warehouse(Warehouse *wh);

	void add_request(Request* req);
	void remove_request(Request* req);

	void add_supply(int ware, Supply* supp);
	void remove_supply(int ware, Supply* supp);

private:
	void do_remove_flag(Flag *f);

	void do_merge(Economy *e);
	void do_split(Flag *f);

	void start_request_timer();

	bool process_request(Request *req);
	void process_requests();

	static void request_timer_cb(Game* g, int serial, int unused);

private:
	Player*	m_owner;
	uint		m_trackserial;
	bool		m_rebuilding;	// true while rebuilding Economies (i.e. during split/merge)

	std::vector<Flag*>		m_flags;
	WareList						m_wares;		// virtual storage with all wares in this Economy
	std::vector<Warehouse*>	m_warehouses;

	std::vector<RequestList>	m_requests; // requests by ware id
	std::vector<SupplyList>		m_supplies; // supplies by ware id

	bool		m_request_timer;	// true if we started the request timer

	uint		mpf_cycle;		// pathfinding cycle, see Flag::mpf_cycle
};

#endif // included_transport_h
