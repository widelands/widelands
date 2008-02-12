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

#ifndef TRANSPORT_H
#define TRANSPORT_H

#ifdef __GNUC__
#define PRINTF_FORMAT(b, c) __attribute__ ((__format__ (__printf__, b, c)))
#else
#define PRINTF_FORMAT(b, c)
#endif

#include "immovable.h"
#include "item_ware_descr.h"
#include "map.h"
#include "tattribute.h"
#include "trackptr.h"
#include "warelist.h"

#include <list>

namespace Widelands {

class Building;
class Economy;
class Flag;
class IdleWareSupply;
class Item_Ware_Descr;
class Request;
class Road;
class Soldier;
class Transfer;
class Warehouse;
struct Map_Map_Object_Loader;
struct Map_Map_Object_Saver;

struct Neighbour {
	Flag * flag;
	Road * road;
	int32_t    cost;
};
typedef std::vector<Neighbour> Neighbour_list;

/**
 * WareInstance represents one item while it is being carried around.
 *
 * The WareInstance never draws itself; the carrying worker or the current flag
 * location are responsible for that.
 *
 * The location of a ware can be one of the following:
 * - a flag
 * - a worker that is currently carrying the ware
 * - a building; this should only be temporary until the ware is incorporated into
 *   the building somehow
 */
class WareInstance : public Map_Object {
	friend struct Map_Waredata_Data_Packet;

	MO_DESCR(Item_Ware_Descr);

public:
	WareInstance(Ware_Index, const Item_Ware_Descr * const);
	~WareInstance();

	virtual int32_t get_type() const throw ();

	Map_Object* get_location(Editor_Game_Base* g) {return m_location.get(g);}
	Economy* get_economy() const throw () {return m_economy;}
	Ware_Index descr_index() const throw () {return m_descr_index;}

	void init(Editor_Game_Base* g);
	void cleanup(Editor_Game_Base* g);
	void act(Game*, uint32_t data);
	void update(Game*);

	void set_location(Editor_Game_Base* g, Map_Object* loc);
	void set_economy(Economy* e);

	bool is_moving() const throw ();
	void cancel_moving();

	PlayerImmovable* get_next_move_step(Game* g);

	void set_transfer(Game* g, Transfer* t);
	void cancel_transfer(Game* g);

private:
	Object_Ptr        m_location;
	Economy         * m_economy;
	Ware_Index       m_descr_index;

	IdleWareSupply  * m_supply;
	Transfer*         m_transfer;
	Object_Ptr        m_transfer_nextstep; //  cached PlayerImmovable, can be 0

private:
};



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
class Flag : public PlayerImmovable {
	friend class Economy;
	friend class FlagQueue;
	friend struct Map_Ware_Data_Packet;     //  has to look at pending items
	friend struct Map_Waredata_Data_Packet; //  has to look at pending items
	friend struct Map_Flagdata_Data_Packet; //  has to read/write this to a file

	struct PendingItem {
		WareInstance    * item;     //  the item itself
		bool              pending;  //  if the item is pending
		PlayerImmovable * nextstep; //  next step that this item is sent to
	};

	struct FlagJob {
		Request *   request;
		std::string program;
	};

public:
	Flag();
	virtual ~Flag();

	static Flag *create(Editor_Game_Base *g, Player *owner, Coords coords);

	virtual int32_t  get_type    () const throw ();
	virtual int32_t  get_size    () const throw ();
	virtual bool get_passable() const throw ();
	std::string const & name() const throw ();

	virtual Flag *get_base_flag();

	const Coords &get_position() const {return m_position;}

	virtual void set_economy(Economy *e);

	Building *get_building() {return m_building;}
	void attach_building(Editor_Game_Base *g, Building *building);
	void detach_building(Editor_Game_Base *g);

	bool has_road() const {
		return
			m_roads[0] or m_roads[1] or m_roads[2] or
			m_roads[3] or m_roads[4] or m_roads[5];
	}
	Road *get_road(int32_t dir) {return m_roads[dir-1];}
	void attach_road(int32_t dir, Road *road);
	void detach_road(int32_t dir);

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

	void add_flag_job(Game* g, int32_t workerware, std::string programname);

	void conquered_by (Player * const who) {assert(who); set_owner(who);}
protected:
	virtual void init(Editor_Game_Base*);
	virtual void cleanup(Editor_Game_Base*);
	virtual void destroy(Editor_Game_Base*);

	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point);

	void wake_up_capacity_queue(Game* g);

	static void flag_job_request_callback
		(Game *, Request *, Ware_Index, Worker *, void * data);

private:
	Coords                  m_position;
	uint32_t                    m_anim;
	int32_t                     m_animstart;

	Building * m_building; //  attached building (replaces road WALK_NW)
	Road                  * m_roads[6];      //  Map_Object::WALK_xx - 1 as index
	int32_t                     m_items_pending[6];

	int32_t                     m_item_capacity; //  size of m_items array
	int32_t m_item_filled; //  number of items currently on the flag
	PendingItem           * m_items;         //  items currently on the flag

	//  call_carrier() will always call a carrier when the destination is the
	//  given flag
	Flag                  * m_always_call_for_flag;

	std::vector<Object_Ptr> m_capacity_wait; //  workers waiting for capacity

	std::list<FlagJob>      m_flag_jobs;

	// The following are only used during pathfinding
	uint32_t                    mpf_cycle;
	int32_t                     mpf_heapindex;
	int32_t                     mpf_realcost; //  real cost of getting to this flag
	Flag                  * mpf_backlink; //  flag where we came from
	int32_t                     mpf_estimate; //  estimate of cost to destination

	int32_t cost() const {return mpf_realcost+mpf_estimate;}
};

/**
 * Road is a special object which connects two flags.
 * The Road itself is never rendered; however, the appropriate Field::roads are
 * set to represent the road visually.
 * The actual steps involved in a road are stored as a Path from the staring flag
 * to the ending flag. Apart from that, however, the two flags are treated
 * exactly the same, as far as most transactions are concerned. There are minor
 * exceptions: placement of carriers if the path's length is odd, splitting
 * a road when a flag is inserted.
 *
 * Every road has one or more Carriers attached to it.
 *
 * All Workers on the Road are attached via add_worker()/remove_worker() in
 * PlayerImmovable.
 */
struct Road : public PlayerImmovable {
	friend struct Map_Roaddata_Data_Packet; // For saving
	friend struct Map_Road_Data_Packet; // For init()

	enum FlagId {
		FlagStart = 0,
		FlagEnd = 1
	};

	Road();
	virtual ~Road();

	static void  create
		(Editor_Game_Base &,
		 Flag & start, Flag & end, Path const &,
		 bool    create_carrier = false,
		 int32_t type           = Road_Normal);

	Flag* get_flag(FlagId flag) const {return m_flags[flag];}

	virtual int32_t  get_type    () const throw ();
	virtual int32_t  get_size    () const throw ();
	virtual bool get_passable() const throw ();
	std::string const & name() const throw ();

	virtual Flag* get_base_flag();

	virtual void set_economy(Economy *e);

	int32_t get_cost(FlagId fromflag);
	const Path &get_path() const {return m_path;}
	int32_t get_idle_index() const {return m_idle_index;}

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
	static void request_carrier_callback
		(Game *, Request *, Ware_Index, Worker *, void * data);

	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point);

private:
	int32_t        m_type;       //  use Field::Road_XXX
	Flag     * m_flags  [2]; //  start and end flag
	int32_t        m_flagidx[2]; //  index of this road in the flag's road array

	//  cost for walking this road (0 = from start to end, 1 = from end to start)
	int32_t        m_cost   [2];

	Path       m_path;       //  path goes from start to end
	int32_t        m_idle_index; //  index into path where carriers should idle

	//  total # of carriers we want (currently limited to 0 or 1)
	uint32_t       m_desire_carriers;

	Object_Ptr m_carrier;    //  our carrier
	Request *  m_carrier_request;
};


/**
 * Route stores a route from flag to flag.
 * The number of steps is the number of flags stored minus one.
 */
struct Route {
	friend class Economy;
	friend class Request;

	Route();

	void clear();

	int32_t get_totalcost() const {return m_totalcost;}
	int32_t get_nrsteps() const {return m_route.size()-1;}
	Flag * get_flag(Editor_Game_Base *, std::vector<Flag *>::size_type);

	void starttrim(int32_t count);
	void truncate(int32_t count);

	struct LoadData {
		std::vector<uint32_t> flags;
	};

	void load(LoadData& data, FileRead& fr);
	void load_pointers(const LoadData&, Map_Map_Object_Loader &);
	void save(FileWrite &, Editor_Game_Base *, Map_Map_Object_Saver *);

private:
	int32_t                     m_totalcost;
	std::vector<Object_Ptr> m_route; //  includes start and end flags
};


/**
 * Whenever an item or worker is transferred to fulfill a Request,
 * a Transfer is allocated to describe this transfer.
 *
 * Transfers are always created and destroyed by a Request instance.
 *
 * Call get_next_step() to find out where you should go next. If
 * get_next_step() returns 0, the transfer is complete or cannot be
 * completed. Call finish() if success is true, fail() otherwise.
 * Call fail() if something really bad has happened (e.g. the worker
 * or ware was destroyed).
 *
 * \todo The mentioned function fail() does not exist!
 */
struct Transfer {
	friend class Request;

	Transfer(Game* g, Request* req, WareInstance* it);
	Transfer(Game* g, Request* req, Worker* w);
	Transfer(Game* g, Request* req, Soldier* s);
	~Transfer();

	Request* get_request() const {return m_request;}
	bool is_idle() const {return m_idle;}

	void set_idle(bool idle);

public: // called by the controlled ware or worker
	PlayerImmovable* get_next_step(PlayerImmovable* location, bool* psuccess);
	void has_finished();
	void has_failed();

private:
	void tlog(const char* fmt, ...) PRINTF_FORMAT(2, 3);

	Game         * m_game;
	Request      * m_request;
	WareInstance * m_item;    //  non-null if ware is an item
	Worker       * m_worker;  //  non-null if ware is a worker
	Soldier      * m_soldier; //  non-null if ware is a soldier
	Route          m_route;

	bool m_idle; //  an idle transfer can be fail()ed if the item feels like it
};


struct MinMax {
	int32_t min;
	int32_t max;
};

struct Requeriments {
	Requeriments (); //  init to allow all

	void set (tAttribute at, int32_t min, int32_t max);

	bool check (int32_t hp, int32_t attack, int32_t defense, int32_t evade);

	// For Save/Load Games
	void Read (FileRead  *, Editor_Game_Base *, Map_Map_Object_Loader *);
	void Write(FileWrite *, Editor_Game_Base *, Map_Map_Object_Saver  *);

private:
	MinMax m_hp;
	MinMax m_attack;
	MinMax m_defense;
	MinMax m_evade;
	MinMax m_total;
};

/**
 * A Supply is a virtual base class representing something that can offer
 * wares of any type for any purpose.
 *
 * Subsequent calls to get_position() can return different results.
 * If a Supply is "active", it should be transferred to a possible Request
 * quickly. Basically, supplies in warehouses (or unused supplies that are
 * being carried into a warehouse) are inactive, and supplies that are just
 * sitting on a flag are active.
 *
 * Important note: The implementation of Supply is responsible for adding
 * and removing itself from Economies. This rule holds true for Economy
 * changes.
 */
struct Supply : public Trackable {
	virtual PlayerImmovable* get_position(Game* g) = 0;
	virtual int32_t get_amount(const int32_t ware) const = 0;
	virtual bool is_active() const throw () = 0;

	virtual WareInstance & launch_item(Game *, int32_t ware) = 0;
	virtual Worker* launch_worker(Game* g, int32_t ware) = 0;

	// This is only for Soldier Requests correct use !
	virtual Soldier* launch_soldier(Game* g, int32_t ware, Requeriments* req) = 0;
	virtual int32_t get_passing_requeriments(Game* g, int32_t ware, Requeriments* r) = 0;
	virtual void mark_as_used (Game* g, int32_t ware, Requeriments* r) = 0;

};


/**
 * SupplyList is used in the Economy to keep track of supplies.
 */
struct SupplyList {
	SupplyList();
	~SupplyList();

	void add_supply(Supply* supp);
	void remove_supply(Supply* supp);

	int32_t get_nrsupplies() const {return m_supplies.size();}
	Supply* get_supply(int32_t idx) const {return m_supplies[idx];}

private:
	std::vector<Supply *> m_supplies;
};


/**
 * A Request is issued whenever some object (road or building) needs a ware.
 *
 * Requests are always created and destroyed by their owner, i.e. the target
 * player immovable. The owner is also responsible for calling set_economy()
 * when its economy changes.
 *
 * Idle Requests need not be fulfilled; however, when there's a matching Supply
 * left, a transfer may be initiated.
 * The required time has no meaning for idle requests.
 */
struct Request : public Trackable {
	friend class Economy;
	friend class RequestList;

	typedef void (*callback_t)
		(Game *, Request *, Ware_Index, Worker *, void * data);

	enum Type {
		WARE    = 0,
		WORKER  = 1,
		SOLDIER = 2
	};

	Request
		(PlayerImmovable * target, Ware_Index, callback_t, void * cbdata, Type);
	~Request();

	PlayerImmovable * get_target() const throw () {return m_target;}
	Ware_Index get_index() const {return m_index;}
	int32_t get_type() const {return m_type;}
	bool is_idle() const {return m_idle;}
	int32_t get_count() const {return m_count;}
	bool is_open() const
	{return m_idle || m_count > static_cast<int32_t>(m_transfers.size());}
	Economy * get_economy() const throw () {return m_economy;}
	int32_t get_required_time();
	int32_t get_last_request_time() {return m_last_request_time;}
	int32_t get_priority(int32_t cost);

	Flag * get_target_flag();

	void set_economy(Economy* e);
	void set_idle(bool idle);
	void set_count(int32_t count);
	void set_required_time(int32_t time);
	void set_required_interval(int32_t interval);

	void set_last_request_time(int32_t time) {m_last_request_time = time;}

	void start_transfer(Game *g, Supply* supp, int32_t ware);


	void Write(FileWrite *, Editor_Game_Base *, Map_Map_Object_Saver  *);
	void Read (FileRead  *, Editor_Game_Base *, Map_Map_Object_Loader *);
	Worker * get_transfer_worker();

	//  callbacks for WareInstance/Worker code
	void transfer_finish(Game* g, Transfer* t);
	void transfer_fail(Game* g, Transfer* t);

private:
	int32_t get_base_required_time(Editor_Game_Base* g, int32_t nr);
public:
	void cancel_transfer(uint32_t idx);
private:
	void remove_transfer(uint32_t idx);
	uint32_t find_transfer(Transfer* t);

	bool has_requeriments () {return (m_requeriments != 0);}
public:
	void set_requeriments (Requeriments* r) {m_requeriments = r;}
private:
	Requeriments* get_requeriments () {return m_requeriments;}

	typedef std::vector<Transfer*> TransferList;

	Type              m_type;
	PlayerImmovable * m_target;            //  who requested it?
	Economy         * m_economy;
	Ware_Index        m_index;             //  the index of the ware descr
	bool              m_idle;
	int32_t               m_count;             //  how many do we need in total

	callback_t        m_callbackfn;        //  called on request success
	void            * m_callbackdata;

	int32_t m_required_time; //  when do we need the first ware (can be in the past)
	int32_t               m_required_interval; //  time between items
	int32_t m_last_request_time;

	TransferList      m_transfers;         //  maximum size is m_count

	Requeriments    * m_requeriments;      //  soldier requeriments
};


/*
WaresQueue
----------
This micro storage room can hold any number of items of a fixed ware.

Note that you must call update() after changing the queue's size or filled
state using one of the set_*() functions.
*/
struct WaresQueue {
	typedef void (callback_t)
		(Game *, WaresQueue *, Ware_Index ware, void * data);

	WaresQueue(PlayerImmovable* bld);
	~WaresQueue();

	int32_t get_ware() const {return m_ware;}
	uint32_t get_size            () const throw () {return m_size;}
	uint32_t get_filled          () const throw () {return m_filled;}
	uint32_t get_consume_interval() const throw () {return m_consume_interval;}

	void init(int32_t ware, uint32_t size);
	void cleanup();
	void update();

	void set_callback(callback_t* fn, void* data);

	void remove_from_economy(Economy* e);
	void add_to_economy(Economy* e);

	void set_size            (uint32_t) throw ();
	void set_filled          (uint32_t) throw ();
	void set_consume_interval(uint32_t) throw ();

	Player * get_owner() const throw () {return m_owner->get_owner();}

	void Write(FileWrite *, Editor_Game_Base *, Map_Map_Object_Saver  *);
	void Read (FileRead  *, Editor_Game_Base *, Map_Map_Object_Loader *);

private:
	static void request_callback
		(Game *, Request *, Ware_Index, Worker *, void * data);

	PlayerImmovable * m_owner;
	int32_t               m_ware; //  ware ID
	uint32_t m_size;             // # of items that fit into the queue
	uint32_t m_filled;           // # of items that are currently in the queue
	uint32_t m_consume_interval; // time in ms between consumption at full speed
	Request         * m_request; //  currently pending request

	callback_t      * m_callback_fn;
	void            * m_callback_data;
};


/*
Economy represents a network of Flag through which wares can be transported.
*/
struct RSPairStruct;

struct Economy {
	friend class Request;

	Economy(Player *player);
	~Economy();

	Player & owner() const throw () {return *m_owner;}
	Player *get_owner() const {return m_owner;}
	uint32_t get_serial() const {return m_trackserial;}

	static void check_merge(Flag *f1, Flag *f2);
	static void check_split(Flag *f1, Flag *f2);

	bool find_route(Flag *start, Flag *end, Route *route, bool wait, int32_t cost_cutoff = -1);
	Warehouse *find_nearest_warehouse(Flag *base, Route *route);

	int32_t get_nrflags() const {return m_flags.size();}
	void add_flag(Flag *flag);
	void remove_flag(Flag *flag);

	void add_wares(int32_t id, int32_t count = 1);
	void remove_wares(int32_t id, int32_t count = 1);

	void    add_workers(int32_t id, int32_t count = 1);
	void remove_workers(int32_t id, int32_t count = 1);

	void add_warehouse(Warehouse *wh);
	void remove_warehouse(Warehouse *wh);
	uint32_t get_nr_warehouses() const {return m_warehouses.size();}

	void add_request(Request* req);
	bool have_request(Request* req);
	void remove_request(Request* req);

	void       add_ware_supply(Ware_Index, Supply *);
	bool      have_ware_supply(Ware_Index, Supply *);
	void    remove_ware_supply(Ware_Index, Supply *);

	void     add_worker_supply(Ware_Index, Supply *);
	bool    have_worker_supply(Ware_Index, Supply *);
	void  remove_worker_supply(Ware_Index, Supply *);

// Soldier stuff
	void    add_soldier_supply(Ware_Index, Supply *);
	bool   have_soldier_supply(Ware_Index, Supply *, Requeriments * = 0);
	void remove_soldier_supply(Ware_Index, Supply *);

	bool should_run_balance_check(int32_t const gametime) {
		return m_request_timer && (gametime == m_request_timer_time);
	}

	int32_t stock_ware(Ware_Index const i) { /// informations over this economy
		return m_wares  .stock(i.value());
	}
	int32_t stock_worker(Ware_Index const i) {
		return m_workers.stock(i.value());
	}
	WareList const & get_wares  () {return m_wares;}
	WareList const & get_workers() {return m_workers;}

	void balance_requestsupply(); ///  called by cmd queue

	void rebalance_supply() {start_request_timer();}

private:
	void do_remove_flag(Flag *f);

	void do_merge(Economy *e);
	void do_split(Flag *f);

	void start_request_timer(int32_t delta = 200);

	Supply * find_best_supply
		(Game                    *,
		 Request                 *,
		 Ware_Index              & ware,
		 int32_t                 & pcost,
		 std::vector<SupplyList> &);
	void process_requests(Game* g, RSPairStruct* s);
	void create_requested_workers(Game* g);

	typedef std::vector<Request*> RequestList;

	Player*   m_owner;
	uint32_t      m_trackserial;

	//  true while rebuilding Economies (i.e. during split/merge)
	bool m_rebuilding;

	std::vector<Flag *>      m_flags;
	WareList m_wares;     //  virtual storage with all wares in this Economy
	WareList m_workers;   //  virtual storage with all wares in this Economy
	std::vector<Warehouse *> m_warehouses;

	RequestList              m_requests; // requests
	std::vector<SupplyList>  m_ware_supplies; // supplies by ware id
	std::vector<SupplyList>  m_worker_supplies; // supplies by ware id

	bool m_request_timer; //  true if we started the request timer
	int32_t                      m_request_timer_time;

	uint32_t mpf_cycle;       //  pathfinding cycle, see Flag::mpf_cycle
};

struct Cmd_Call_Economy_Balance : public GameLogicCommand {
	Cmd_Call_Economy_Balance () : GameLogicCommand (0) {} //  for load and save

	Cmd_Call_Economy_Balance
		(int32_t       const starttime,
		 Player_Number const player,
		 Economy     * const economy)
		:
		GameLogicCommand(starttime),
		m_force_balance (false),
		m_player        (player),
		m_economy       (economy)
	{}

	void execute (Game *);

	virtual int32_t get_id() {return QUEUE_CMD_CALL_ECONOMY_BALANCE;}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

private:
	bool          m_force_balance;
	Player_Number m_player;
	Economy     * m_economy;
};

};

#endif
