/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
#define PRINTF_FORMAT(b, c) __attribute__ ((__format__ (__printf__, b, c)))
#else
#define PRINTF_FORMAT(b, c)
#endif

#include "immovable.h"
#include "item_ware_descr.h"
#include "map.h"
#include "trackptr.h"
#include "warelist.h"

#include <list>

class Building;
class Economy;
class FileRead;
class FileWrite;
class Flag;
class IdleWareSupply;
class Item_Ware_Descr;
class Request;
class Road;
class Soldier;
class Transfer;
class Warehouse;
class Widelands_Map_Map_Object_Loader;
class Widelands_Map_Map_Object_Saver;
class FileRead;
class FileWrite;

struct Neighbour {
	Flag * flag;
	Road * road;
	int    cost;
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
   friend class Widelands_Map_Waredata_Data_Packet;

	MO_DESCR(Item_Ware_Descr);

public:
	WareInstance
		(const Item_Ware_Descr::Index, const Item_Ware_Descr * const);
	~WareInstance();

	virtual int get_type() const throw ();

	Map_Object* get_location(Editor_Game_Base* g) {return m_location.get(g);}
	Economy* get_economy() const throw () {return m_economy;}
	Item_Ware_Descr::Index descr_index() const throw () {return m_descr_index;}
	__attribute__ ((deprecated)) const Item_Ware_Descr * get_ware_descr() const throw () {assert(m_ware_descr); return m_ware_descr;}

	void init(Editor_Game_Base* g);
	void cleanup(Editor_Game_Base* g);
	void act(Game*, uint data);
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
	Item_Ware_Descr::Index m_descr_index;
	const Item_Ware_Descr * m_ware_descr;

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
   friend class Widelands_Map_Ware_Data_Packet; // has to look at pending items
   friend class Widelands_Map_Waredata_Data_Packet; // has to look at pending items
   friend class Widelands_Map_Flagdata_Data_Packet; // has to read/write this to a file

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

	virtual int  get_type    () const throw ();
	virtual int  get_size    () const throw ();
	virtual bool get_passable() const throw ();

	virtual Flag *get_base_flag();

	inline const Coords &get_position() const {return m_position;}

	virtual void set_economy(Economy *e);

	inline Building *get_building() {return m_building;}
	void attach_building(Editor_Game_Base *g, Building *building);
	void detach_building(Editor_Game_Base *g);

	inline Road *get_road(int dir) {return m_roads[dir-1];}
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

   void conquered_by (Player* who) {assert(who); set_owner(who);}
protected:
	virtual void init(Editor_Game_Base*);
	virtual void cleanup(Editor_Game_Base*);
	virtual void destroy(Editor_Game_Base*);

	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point);

	void wake_up_capacity_queue(Game* g);

	static void flag_job_request_callback(Game* g, Request* rq, int ware, Worker* w, void* data);

private:
	Coords                  m_position;
	uint                    m_anim;
	int                     m_animstart;

	Building * m_building; //  attached building (replaces road WALK_NW)
	Road                  * m_roads[6];      //  Map_Object::WALK_xx - 1 as index
	int                     m_items_pending[6];

	int                     m_item_capacity; //  size of m_items array
	int m_item_filled; //  number of items currently on the flag
	PendingItem           * m_items;         //  items currently on the flag

	//  call_carrier() will always call a carrier when the destination is the
	//  given flag
	Flag                  * m_always_call_for_flag;

	std::vector<Object_Ptr> m_capacity_wait; //  workers waiting for capacity

	std::list<FlagJob>      m_flag_jobs;

	// The following are only used during pathfinding
	uint                    mpf_cycle;
	int                     mpf_heapindex;
	int                     mpf_realcost; //  real cost of getting to this flag
	Flag                  * mpf_backlink; //  flag where we came from
	int                     mpf_estimate; //  estimate of cost to destination

	inline int cost() const {return mpf_realcost+mpf_estimate;}
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
   friend class Widelands_Map_Roaddata_Data_Packet; // For saving
   friend class Widelands_Map_Road_Data_Packet; // For init()

	enum FlagId {
		FlagStart = 0,
		FlagEnd = 1
	};

	Road();
	virtual ~Road();

	static Road* create(Editor_Game_Base *g, int type, Flag* start, Flag* end, const Path &path);

	inline Flag* get_flag(FlagId flag) const {return m_flags[flag];}

	virtual int  get_type    () const throw ();
	virtual int  get_size    () const throw ();
	virtual bool get_passable() const throw ();

	virtual Flag* get_base_flag();

	virtual void set_economy(Economy *e);

	int get_cost(FlagId fromflag);
	inline const Path &get_path() const {return m_path;}
	inline int get_idle_index() const {return m_idle_index;}

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

	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point);

private:
	int        m_type;       //  use Field::Road_XXX
	Flag     * m_flags  [2]; //  start and end flag
	int        m_flagidx[2]; //  index of this road in the flag's road array

	//  cost for walking this road (0 = from start to end, 1 = from end to start)
	int        m_cost   [2];

	Path       m_path;       //  path goes from start to end
	int        m_idle_index; //  index into path where carriers should idle

	//  total # of carriers we want (currently limited to 0 or 1)
	uint       m_desire_carriers;

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
   friend class Widelands_Map_Bobdata_Data_Packet; // This is in the state structure

	Route();

	void clear();

	inline int get_totalcost() const {return m_totalcost;}
	inline int get_nrsteps() const {return m_route.size()-1;}
	Flag * get_flag
		(Editor_Game_Base * const, const std::vector<Flag *>::size_type) const;

	void starttrim(int count);
	void truncate(int count);

private:
	int                     m_totalcost;
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


struct Requeriments {
	Requeriments (); //  init to allow all

	void set (tAttribute at, int min, int max);

	bool check (int hp, int attack, int defense, int evade);

	// For Save/Load Games
	void Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol);
	void Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos);

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
	virtual int get_amount(const int ware) const = 0;
	virtual bool is_active() const throw () = 0;

	virtual WareInstance* launch_item(Game* g, int ware) = 0;
	virtual Worker* launch_worker(Game* g, int ware) = 0;

	// This is only for Soldier Requests correct use !
	virtual Soldier* launch_soldier(Game* g, int ware, Requeriments* req) = 0;
	virtual int get_passing_requeriments(Game* g, int ware, Requeriments* r) = 0;
	virtual void mark_as_used (Game* g, int ware, Requeriments* r) = 0;

};


/**
 * SupplyList is used in the Economy to keep track of supplies.
 */
struct SupplyList {
	SupplyList();
	~SupplyList();

	void add_supply(Supply* supp);
	void remove_supply(Supply* supp);

	inline int get_nrsupplies() const {return m_supplies.size();}
	inline Supply* get_supply(int idx) const {return m_supplies[idx];}

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

	typedef void (*callback_t)(Game*, Request*, int ware, Worker*, void* data);

   enum Type {
      WARE = 0,
      WORKER = 1,
	   SOLDIER = 2
	};

	Request(PlayerImmovable *target, int index, callback_t cbfn, void* cbdata, Type);
	~Request();

	PlayerImmovable * get_target() const throw () {return m_target;}
	int get_index() const {return m_index;}
   int get_type() const {return m_type;}
	bool is_idle() const {return m_idle;}
	int get_count() const {return m_count;}
	bool is_open() const
	{return m_idle || m_count > static_cast<int>(m_transfers.size());}
	Economy * get_economy() const throw () {return m_economy;}
	int get_required_time();
	int get_last_request_time() {return m_last_request_time;}
	int get_priority(int cost);

	Flag * get_target_flag();

	void set_economy(Economy* e);
	void set_idle(bool idle);
	void set_count(int count);
	void set_required_time(int time);
	void set_required_interval(int interval);

	void set_last_request_time(int time) {m_last_request_time = time;}

	void start_transfer(Game *g, Supply* supp, int ware);


   // For savegames
   void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);
   Worker* get_transfer_worker();

	//  callbacks for WareInstance/Worker code
	void transfer_finish(Game* g, Transfer* t);
	void transfer_fail(Game* g, Transfer* t);

private:
	int get_base_required_time(Editor_Game_Base* g, int nr);
public:
	void cancel_transfer(uint idx);
private:
	void remove_transfer(uint idx);
	uint find_transfer(Transfer* t);

	bool has_requeriments () {return (m_requeriments != 0);}
public:
	void set_requeriments (Requeriments* r) {m_requeriments = r;}
private:
	Requeriments* get_requeriments () {return m_requeriments;}

	typedef std::vector<Transfer*> TransferList;

   Type              m_type;
	PlayerImmovable * m_target;            //  who requested it?
	Economy         * m_economy;
	int               m_index;             //  the index of the ware descr
	bool              m_idle;
	int               m_count;             //  how many do we need in total

	callback_t        m_callbackfn;        //  called on request success
	void            * m_callbackdata;

	int m_required_time; //  when do we need the first ware (can be in the past)
	int               m_required_interval; //  time between items
	int m_last_request_time;

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
	typedef void (callback_t)(Game* g, WaresQueue* wq, int ware, void* data);

	WaresQueue(PlayerImmovable* bld);
	~WaresQueue();

	int get_ware() const {return m_ware;}
	uint get_size            () const throw () {return m_size;}
	uint get_filled          () const throw () {return m_filled;}
	uint get_consume_interval() const throw () {return m_consume_interval;}

	void init(const int ware, const uint size);
	void cleanup();
	void update();

	void set_callback(callback_t* fn, void* data);

	void remove_from_economy(Economy* e);
	void add_to_economy(Economy* e);

	void set_size            (const uint) throw ();
	void set_filled          (const uint) throw ();
	void set_consume_interval(const uint) throw ();

	Player * get_owner() const throw () {return m_owner->get_owner();}

   // For savegames
   void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

private:
	static void request_callback(Game* g, Request* rq, int ware, Worker* w, void* data);

	PlayerImmovable * m_owner;
	int               m_ware; //  ware ID
	uint m_size;             // # of items that fit into the queue
	uint m_filled;           // # of items that are currently in the queue
	uint m_consume_interval; // time in ms between consumption at full speed
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
	inline Player *get_owner() const {return m_owner;}
	uint get_serial() const {return m_trackserial;}

	static void check_merge(Flag *f1, Flag *f2);
	static void check_split(Flag *f1, Flag *f2);

	bool find_route(Flag *start, Flag *end, Route *route, bool wait, int cost_cutoff = -1);
	Warehouse *find_nearest_warehouse(Flag *base, Route *route);

	inline int get_nrflags() const {return m_flags.size();}
	void add_flag(Flag *flag);
	void remove_flag(Flag *flag);

	void add_wares(int id, int count = 1);
	void remove_wares(int id, int count = 1);

   void add_workers(int id, int count = 1);
	void remove_workers(int id, int count = 1);

	void add_warehouse(Warehouse *wh);
	void remove_warehouse(Warehouse *wh);
	uint get_nr_warehouses() const {return m_warehouses.size();}

	void add_request(Request* req);
	bool have_request(Request* req);
	void remove_request(Request* req);

	void add_ware_supply(int ware, Supply* supp);
	bool have_ware_supply(int ware, Supply* supp);
	void remove_ware_supply(int ware, Supply* supp);

	void add_worker_supply(int worker, Supply* supp);
	bool have_worker_supply(int worker, Supply* supp);
	void remove_worker_supply(int worker, Supply* supp);

// Soldier stuff
	void add_soldier_supply(int soldier, Supply* supp);
	bool have_soldier_supply(int soldier, Supply* supp, Requeriments* r = 0);
	void remove_soldier_supply(int soldier, Supply* supp);

   inline bool should_run_balance_check(int gametime) {
      if (m_request_timer && (gametime == m_request_timer_time)) return true;
      return false;
	}

   // Informations over this economy
   int stock_ware(int id) {return m_wares.stock(id);}
   int stock_worker(int id) {return m_workers.stock(id);}
   const WareList& get_wares() {return m_wares;}
   const WareList& get_workers() {return m_workers;}

   // Called by cmd queue
   void balance_requestsupply();

   void rebalance_supply() {start_request_timer();}

private:
	void do_remove_flag(Flag *f);

	void do_merge(Economy *e);
	void do_split(Flag *f);

	void start_request_timer(int delta = 200);

	Supply* find_best_supply(Game* g, Request* req, int* ware, int* pcost, std::vector<SupplyList>*);
	int  get_ware_substitute(Request* req, int ware);
	void process_requests(Game* g, RSPairStruct* s);
	void create_requested_workers(Game* g);

	typedef std::vector<Request*> RequestList;

	Player*   m_owner;
	uint      m_trackserial;

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
	int                      m_request_timer_time;

	uint mpf_cycle;       //  pathfinding cycle, see Flag::mpf_cycle
};

struct Cmd_Call_Economy_Balance : public GameLogicCommand {
      Cmd_Call_Economy_Balance () : GameLogicCommand (0) {} // For load and save

      Cmd_Call_Economy_Balance (int starttime, int player, Economy* economy) :
         GameLogicCommand(starttime)
      {
         m_player=player;
         m_economy=economy;
         m_force_balance = false;
		}

      void execute (Game* g);

      virtual int get_id() {return QUEUE_CMD_CALL_ECONOMY_BALANCE;}

	virtual void Write
		(FileWrite             &,
		 Editor_Game_Base               &,
		 Widelands_Map_Map_Object_Saver &);
	virtual void Read
		(FileRead               &,
		 Editor_Game_Base                &,
		 Widelands_Map_Map_Object_Loader &);

   private:
      bool     m_force_balance;
      int      m_player;
      Economy* m_economy;
};


#endif // included_transport_h
