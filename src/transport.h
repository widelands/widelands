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

#ifdef __GNUC__
#define PRINTF_FORMAT(b,c) __attribute__ (( __format__ (__printf__,b,c) ))
#else
#define PRINTF_FORMAT(b,c)
#endif

#include <set>
#include <list>

#include "immovable.h"
#include "map.h"
#include "trackptr.h"
#include "ware.h"

class Building;
class Economy;
class FileRead;
class FileWrite;
class Flag;
class IdleWareSupply;
class Item_Ware_Descr;
class Request;
class Road;
class Transfer;
class Warehouse;
class Widelands_Map_Map_Object_Loader;
class Widelands_Map_Map_Object_Saver;


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
   friend class Widelands_Map_Waredata_Data_Packet;
   
public:
	WareInstance(int, Item_Ware_Descr*);
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

	void set_location(Editor_Game_Base* g, Map_Object* loc);
	void set_economy(Economy* e);

	bool is_moving(Game* g);
	void cancel_moving(Game* g);

	PlayerImmovable* get_next_move_step(Game* g);

	void set_transfer(Game* g, Transfer* t);
	void cancel_transfer(Game* g);

private:
	Object_Ptr			m_location;
	Economy*				m_economy;
	int					m_ware;
	Item_Ware_Descr*	m_ware_descr;

	IdleWareSupply*	m_supply;
	Transfer*			m_transfer;
	Object_Ptr			m_transfer_nextstep; // cached PlayerImmovable, can be 0

private:
};



/*
Flag represents a flag, obviously.
A flag itself doesn't do much. However, it can have up to 6 roads attached
to it. Instead of the WALK_NW road, it can also have a building attached to
it.
Flags also have a store of up to 8 wares.

You can also assign an arbitrary number of "jobs" for a flag.
A job consists of a request for a worker, and the name of a program that the
worker is to execute. Once execution of the program has finished, the worker
will return to a warehouse.

Important: Do not access m_roads directly. get_road() and others use
Map_Object::WALK_xx in all "direction" parameters.
*/
class Flag : public PlayerImmovable {
	friend class Economy;
	friend class FlagQueue;
   friend class Widelands_Map_Ware_Data_Packet; // has to look at pending items
   friend class Widelands_Map_Waredata_Data_Packet; // has to look at pending items 
   friend class Widelands_Map_Flagdata_Data_Packet; // has to read/write this to a file 

private:
	struct PendingItem {
		WareInstance*		item;			// the item itself
		bool					pending;		// if the item is pending
		PlayerImmovable*	nextstep;	// next step that this item is sent to
	};

	struct FlagJob {
		Request*		request;
		std::string	program;
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

	void remove_item(Editor_Game_Base* g, WareInstance* item);

	void add_flag_job(Game* g, int workerware, std::string programname);

protected:
	virtual void init(Editor_Game_Base*);
	virtual void cleanup(Editor_Game_Base*);
	virtual void destroy(Editor_Game_Base*);

	virtual void draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords, Point pos);

	void wake_up_capacity_queue(Game* g);

	static void flag_job_request_callback(Game* g, Request* rq, int ware, Worker* w, void* data);

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

	std::list<FlagJob>	m_flag_jobs;

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
   friend class Widelands_Map_Roaddata_Data_Packet; // For saving 
   friend class Widelands_Map_Road_Data_Packet; // For init() 

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
	void link_into_flags(Editor_Game_Base *g);
	void check_for_carrier(Editor_Game_Base *g);
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
   friend class Request;
   friend class Widelands_Map_Bobdata_Data_Packet; // This is in the state structure
   
public:
	Route();

	void clear();
	bool verify(Game *g);

	inline int get_totalcost() const { return m_totalcost; }
	inline int get_nrsteps() const { return m_route.size()-1; }
	Flag *get_flag(Editor_Game_Base *g, int idx);

	void starttrim(int count);
	void truncate(int count);

private:
	int				m_totalcost;
	std::vector<Object_Ptr>		m_route;	// includes start and end flags
};


/*
Whenever an item or worker is transferred to fulfill a Request,
a Transfer is allocated to describe this transfer.

Transfers are always created and destroyed by a Request instance.

Call get_next_step() to find out where you should go next. If
get_next_step() returns 0, the transfer is complete or cannot be
completed. Call finish() if success is true, fail() otherwise.
Call fail() if something really bad has happened (e.g. the worker
or ware was destroyed).
*/
class Transfer {
	friend class Request;

public:
	Transfer(Game* g, Request* req, WareInstance* it);
	Transfer(Game* g, Request* req, Worker* w);
	~Transfer();

	Request* get_request() const { return m_request; }
	bool is_idle() const { return m_idle; }

	void set_idle(bool idle);
      
public: // called by the controlled ware or worker
	PlayerImmovable* get_next_step(PlayerImmovable* location, bool* psuccess);
	void has_finished();
	void has_failed();

private:
	void tlog(const char* fmt, ...) PRINTF_FORMAT(2,3);

private:
	Game*				m_game;
	Request*			m_request;
	WareInstance*	m_item;			// non-null if ware is an item
	Worker*			m_worker;		// non-null if ware is a worker
	Route				m_route;

	bool				m_idle;		// an idle transfer can be fail()ed if the item feels like it
};


/*
A Supply is a virtual base class representing something that can offer
wares of any type for any purpose.

Subsequent calls to get_position() can return different results.
If a Supply is "active", it should be transferred to a possible Request
quickly. Basically, supplies in warehouses (or unused supplies that are
being carried into a warehouse) are inactive, and supplies that are just
sitting on a flag are active.

Important note: The implementation of Supply is responsible for adding
and removing itself from Economies. This rule holds true for Economy
changes.
*/
class Supply : public Trackable {
public:
	virtual PlayerImmovable* get_position(Game* g) = 0;
	virtual int get_amount(Game* g, int ware) = 0;
	virtual bool is_active(Game* g) = 0;

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

	void add_supply(Supply* supp);
	void remove_supply(Supply* supp);

	inline int get_nrsupplies() const { return m_supplies.size(); }
	inline Supply* get_supply(int idx) const { return m_supplies[idx]; }

private:
	std::vector<Supply*>	m_supplies;
};


/*
A Request is issued whenever some object (road or building) needs a ware.

Requests are always created and destroyed by their owner, i.e. the target
player immovable. The owner is also responsible for calling set_economy()
when its economy changes.

Idle Requests need not be fulfilled; however, when there's a matching Supply
left, a transfer may be initiated.
The required time has no meaning for idle requests.
*/
class Request : public Trackable {
	friend class Economy;
	friend class RequestList;

public:
	typedef void (*callback_t)(Game*, Request*, int ware, Worker*, void* data);

   enum Type {
      WORKER,
      WARE,
   };

public:
	Request(PlayerImmovable *target, int index, callback_t cbfn, void* cbdata, Type);
	~Request();

	PlayerImmovable* get_target(Game* g) { return m_target; }
	int get_index() const { return m_index; }
   int get_type() const { return m_type; }
	bool is_idle() const { return m_idle; }
	int get_count() const { return m_count; }
	bool is_open() const { return m_idle || m_count > (int)m_transfers.size(); }
	Economy* get_economy() const { return m_economy; }
	int get_required_time();

	Flag *get_target_flag(Game *g);

	void set_economy(Economy* e);
	void set_idle(bool idle);
	void set_count(int count);
	void set_required_time(int time);
	void set_required_interval(int interval);

	void start_transfer(Game *g, Supply* supp);
   

   // For savegames
   void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);
   Worker* get_transfer_worker(void);

public: // callbacks for WareInstance/Worker code
	void transfer_finish(Game* g, Transfer* t);
	void transfer_fail(Game* g, Transfer* t);

private:
	int get_base_required_time(Editor_Game_Base* g, int nr);

	void cancel_transfer(uint idx);
	void remove_transfer(uint idx);
	uint find_transfer(Transfer* t);


private:
	typedef std::vector<Transfer*> TransferList;

   Type              m_type;
	PlayerImmovable*	m_target;	// who requested it?
	Economy*				m_economy;
	int					m_index;		// the index of the ware descr
	bool					m_idle;
	int					m_count;		// how many do we need in total

	callback_t		m_callbackfn;		// called on request success
	void*				m_callbackdata;

	int				m_required_time;	// when do we need the first ware (can be in the past)
	int				m_required_interval;	// time between items

	TransferList	m_transfers;	// maximum size is m_count
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
	int get_consume_interval() const { return m_consume_interval; }

	void init(Game*, int ware, int size);
	void cleanup(Game*);
	void update(Game*);

	void set_callback(callback_t* fn, void* data);

	void remove_from_economy(Economy* e);
	void add_to_economy(Economy* e);

	void set_size(int size);
	void set_filled(int size);
	void set_consume_interval(int time);

   Player* get_owner(void) { return m_owner->get_owner(); }
   
   // For savegames
   void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

private:
	static void request_callback(Game* g, Request* rq, int ware, Worker* w, void* data);

private:
	PlayerImmovable*	m_owner;
	int					m_ware;		// ware ID
	int					m_size;		// # of items that fit into the queue
	int					m_filled;	// # of items that are currently in the queue
	int					m_consume_interval; // time in ms between consumption at full speed
	Request*				m_request;	// currently pending request

	callback_t*			m_callback_fn;
	void*					m_callback_data;
};


/*
Economy represents a network of Flag through which wares can be transported.
*/
struct RSPairStruct;

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
	
   void add_workers(int id, int count = 1);
	void remove_workers(int id, int count = 1);

	void add_warehouse(Warehouse *wh);
	void remove_warehouse(Warehouse *wh);
	uint get_nr_warehouses() const { return m_warehouses.size(); }

	void add_request(Request* req);
	bool have_request(Request* req);
	void remove_request(Request* req);

	void add_ware_supply(int ware, Supply* supp);
	bool have_ware_supply(int ware, Supply* supp);
	void remove_ware_supply(int ware, Supply* supp);
	void add_worker_supply(int worker, Supply* supp);
	bool have_worker_supply(int worker, Supply* supp);
	void remove_worker_supply(int worker, Supply* supp);

   inline bool should_run_balance_check(int gametime) { 
      if (m_request_timer && (gametime == m_request_timer_time)) return true;
      return false;
   }

   // Called by cmd queue
   void balance_requestsupply();

private:
	void do_remove_flag(Flag *f);

	void do_merge(Economy *e);
	void do_split(Flag *f);

	void start_request_timer(int delta = 200);

	Supply* find_best_supply(Game* g, Request* req, int ware, int* pcost, std::vector<SupplyList>*);
	void process_requests(Game* g, RSPairStruct* s);
	void create_requested_workers(Game* g);

private:
	typedef std::vector<Request*> RequestList;

	Player*	m_owner;
	uint		m_trackserial;
	bool		m_rebuilding;	// true while rebuilding Economies (i.e. during split/merge)

	std::vector<Flag*>		m_flags;
	WareList						m_wares;		// virtual storage with all wares in this Economy
	WareList						m_workers;		// virtual storage with all wares in this Economy
	std::vector<Warehouse*>	m_warehouses;

	RequestList						m_requests; // requests
	std::vector<SupplyList>		m_ware_supplies; // supplies by ware id
	std::vector<SupplyList>		m_worker_supplies; // supplies by ware id

	bool		m_request_timer;	// true if we started the request timer
	int		m_request_timer_time;

	uint		mpf_cycle;		// pathfinding cycle, see Flag::mpf_cycle
};

class Cmd_Call_Economy_Balance : public BaseCommand {
   public:
      Cmd_Call_Economy_Balance (void) : BaseCommand (0) { } // For load and save 

      Cmd_Call_Economy_Balance (int starttime, int player, Economy* economy) :
         BaseCommand(starttime)
      {  
         m_player=player;
         m_economy=economy;
         m_force_balance = false;
      }     

      void execute (Game* g);

      virtual int get_id(void) { return QUEUE_CMD_CALL_ECONOMY_BALANCE; } 

      virtual void Write(FileWrite* fw, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
      virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

   private:
      bool     m_force_balance;
      int      m_player;
      Economy* m_economy;
};


#endif // included_transport_h
