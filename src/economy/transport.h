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

#include "ui_unique_window.h"

#include <list>

struct Interactive_Player;

namespace Widelands {

class Building;
struct Economy;
class Flag;
class IdleWareSupply;
struct Item_Ware_Descr;
struct Request;
struct Requirements;
struct Road;
class Soldier;
struct Transfer;
class Warehouse;
struct Map_Map_Object_Loader;
struct Map_Map_Object_Saver;

}

#include "ware_instance.h"
#include "flag.h"

namespace Widelands {


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

	Flag & get_flag(FlagId const flag) const {return *m_flags[flag];}

	virtual int32_t  get_type    () const throw ();
	char const * type_name() const throw () {return "road";}
	virtual int32_t  get_size    () const throw ();
	virtual bool get_passable() const throw ();
	std::string const & name() const throw ();

	virtual Flag & base_flag();

	virtual void set_economy(Economy *);

	int32_t get_cost(FlagId fromflag);
	const Path &get_path() const {return m_path;}
	int32_t get_idle_index() const {return m_idle_index;}

	void presplit(Editor_Game_Base &, Coords split);
	void postsplit(Editor_Game_Base &, Flag &);

	bool notify_ware(Game & game, FlagId flagid);
	virtual void remove_worker(Worker &);

protected:
	void set_path(Editor_Game_Base &, Path const &);

	void mark_map(Editor_Game_Base &);
	void unmark_map(Editor_Game_Base &);

	virtual void init(Editor_Game_Base &);
	void link_into_flags(Editor_Game_Base &);
	void check_for_carrier(Editor_Game_Base &);
	virtual void cleanup(Editor_Game_Base &);

	void request_carrier(Game &);
	static void request_carrier_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point);

private:
	int32_t        m_type;       ///< use Field::Road_XXX
	Flag     * m_flags  [2]; ///< start and end flag
	int32_t        m_flagidx[2]; ///< index of this road in the flag's road array

	/// cost for walking this road (0 = from start to end, 1 = from end to start)
	int32_t        m_cost   [2];

	Path       m_path;       ///< path goes from start to end
	uint32_t   m_idle_index; ///< index into path where carriers should idle

	///< total number of carriers we want (currently limited to 0 or 1)
	uint32_t       m_desire_carriers;

	Object_Ptr m_carrier;    ///< our carrier
	Request *  m_carrier_request;
};

}


#include "route.h"
#include "transfer.h"

namespace Widelands {

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
	virtual PlayerImmovable * get_position(Game &) = 0;
	virtual bool is_active() const throw () = 0;

	/**
	 * \return the number of items or workers that can be launched right
	 * now for the thing requested by the given request
	 */
	virtual uint32_t nr_supplies(Game const &, Request const &) const = 0;

	/**
	 * Prepare an item to satisfy the given request. Note that the caller
	 * must assign a transfer to the launched item.
	 *
	 * \throw wexception if the request is not an item request or no such
	 * item is available in the supply.
	 */
	virtual WareInstance & launch_item(Game &, Request const &) = 0;

	/**
	 * Prepare a worker to satisfy the given request. Note that the caller
	 * must assign a transfer to the launched item.
	 *
	 * \throw wexception if the request is not a worker request or no such
	 * worker is available in the supply.
	 */
	virtual Worker & launch_worker(Game &, Request const &) = 0;
};


/**
 * SupplyList is used in the Economy to keep track of supplies.
 */
struct SupplyList {
	void add_supply(Supply &);
	void remove_supply(Supply &);

	size_t get_nrsupplies() const {return m_supplies.size();}
	Supply const & operator[](size_t const idx) const {return *m_supplies[idx];}
	Supply & operator[](size_t const idx) {return *m_supplies[idx];}

private:
	typedef std::vector<Supply *> Supplies;
	Supplies m_supplies;
};


/**
 * This micro storage room can hold any number of items of a fixed ware.
 *
 * You must call update() after changing the queue's size or filled state using
 * one of the set_*() functions.
 */
struct WaresQueue {
	typedef void (callback_t)
		(Game &, WaresQueue *, Ware_Index ware, void * data);

	WaresQueue(PlayerImmovable &, Ware_Index, uint8_t size, uint8_t filled = 0);

#ifndef NDEBUG
	~WaresQueue() {assert(not m_ware);}
#endif

	Ware_Index get_ware() const {return m_ware;}
	uint32_t get_size            () const throw () {return m_size;}
	uint32_t get_filled          () const throw () {return m_filled;}
	uint32_t get_consume_interval() const throw () {return m_consume_interval;}

	void cleanup();
	void update();

	void set_callback(callback_t *, void * data);

	void remove_from_economy(Economy &);
	void add_to_economy(Economy &);

	void set_size            (uint32_t) throw ();
	void set_filled          (uint32_t) throw ();
	void set_consume_interval(uint32_t) throw ();

	Player & owner() const throw () {return m_owner.owner();}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  *);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader *);

private:
	static void request_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

	PlayerImmovable & m_owner;
	Ware_Index        m_ware;    ///< ware ID
	uint32_t m_size;             ///< number of items that fit into the queue
	uint32_t m_filled;           ///< number of items that are currently in the queue
	uint32_t m_consume_interval; ///< time in ms between consumption at full speed
	Request         * m_request; ///< currently pending request

	callback_t      * m_callback_fn;
	void            * m_callback_data;
};


/**
 * Economy represents a network of Flag through which wares can be transported.
 */
struct RSPairStruct;
struct Cmd_SetTargetQuantity;
struct Cmd_ResetTargetQuantity;

struct Economy {
	friend struct Request;
	friend struct Cmd_SetTargetQuantity;
	friend struct Cmd_ResetTargetQuantity;

	/// Configurable target quantity for the supply of a ware type in the
	/// economy.
	///
	/// This affects the result of \ref needs_ware and thereby the demand checks
	/// in production programs. A ware type is considered to be needed if there
	/// are less than the temporary target quantity stored in warehouses in the
	/// economy.
	///
	/// The temporary quantity will be decreased when a ware of the type is
	/// consumed, but not below the permanent quantity. This allows the
	/// following use case:
	///   The player has built up a lot of economic infrastructure but lacks
	///   mines. Then he discovers a mountain. He plans to expand to the
	///   mountain and build some mines. Therefore he anticipates that he will
	///   soon need 10 picks. Suppose that he has the target quantity (both
	///   permanent and temporary) for pick set to 2. He sets the temporary
	///   target quantity for pick to 12.  For each pick that is consumed in the
	///   economy, the target quantity for pick drops back 1 step, until it
	///   reaches the permanent level of 2. So when the player has built his
	///   mines and used his 10 picks, the targe quantity setting for pick is
	///   what it was before he changed it. That is the rationale for the name
	///   "temporary".
	///
	/// The last_modified time is used to determine which setting to use when
	/// economies are merged. The setting that was modified most recently will
	/// be used for the merged economy.
	struct Target_Quantity {
		uint32_t permanent, temporary;
		Time     last_modified;
	};

	Economy(Player &);
	~Economy();

	Player & owner() const throw () {return m_owner;}

	static void check_merge(Flag &, Flag &);
	static void check_split(Flag &, Flag &);

	bool find_route
		(Flag & start, Flag & end,
		 Route * route,
		 bool    wait,
		 int32_t cost_cutoff = -1);

	std::vector<Flag *>::size_type get_nrflags() const {return m_flags.size();}
	void    add_flag(Flag &);
	void remove_flag(Flag &);
	Flag & get_arbitrary_flag();

	void    add_wares  (Ware_Index, uint32_t count = 1);
	void remove_wares  (Ware_Index, uint32_t count = 1);

	void    add_workers(Ware_Index, uint32_t count = 1);
	void remove_workers(Ware_Index, uint32_t count = 1);

	void add_warehouse(Warehouse *wh);
	void remove_warehouse(Warehouse *wh);
	uint32_t get_nr_warehouses() const {return m_warehouses.size();}

	void    add_request(Request &);
	bool   have_request(Request &);
	void remove_request(Request &);

	void    add_supply(Supply &);
	void remove_supply(Supply &);

	/// information about this economy
	WareList::count_type stock_ware  (Ware_Index const i) {
		return m_wares  .stock(i);
	}
	WareList::count_type stock_worker(Ware_Index const i) {
		return m_workers.stock(i);
	}

	/// Whether the economy needs more of this ware type.
	/// Productionsites may ask this before they produce, to avoid depleting a
	/// ware type by overproducing another from it.
	bool needs_ware(Ware_Index) const;

	Target_Quantity const & target_quantity(Ware_Index const i) const {
		return m_target_quantities[i.value()];
	}
	Target_Quantity       & target_quantity(Ware_Index const i)       {
		return m_target_quantities[i.value()];
	}

	void show_options_window();
	UI::UniqueWindow::Registry m_optionswindow_registry;


	WareList const & get_wares  () {return m_wares;}
	WareList const & get_workers() {return m_workers;}

	void balance_requestsupply(uint32_t timerid); ///< called by \ref Cmd_Call_Economy_Balance

	void rebalance_supply() {start_request_timer();}

	void Read (FileRead  &, Game &, Map_Map_Object_Loader *);
	void Write(FileWrite &, Game &, Map_Map_Object_Saver  *);

private:
	void do_remove_flag(Flag &);

	void do_merge(Economy &);
	void do_split(Flag &);

	void start_request_timer(int32_t delta = 200);

	Supply * find_best_supply(Game &, Request const &, int32_t & cost);
	void process_requests(Game &, RSPairStruct &);
	void create_requested_workers(Game &);

	typedef std::vector<Request *> RequestList;

	Player & m_owner;

	/// True while rebuilding Economies (i.e. during split/merge)
	bool m_rebuilding;

	typedef std::vector<Flag *> Flags;
	Flags m_flags;
	WareList m_wares;     ///< virtual storage with all wares in this Economy
	WareList m_workers;   ///< virtual storage with all workers in this Economy
	std::vector<Warehouse *> m_warehouses;

	RequestList m_requests; ///< requests
	SupplyList m_supplies;

	Target_Quantity        * m_target_quantities;

	/**
	 * ID for the next request balancing timer. Used to throttle
	 * excessive calls to the request/supply balancing logic.
	 */
	uint32_t m_request_timerid;

	uint32_t mpf_cycle;       ///< pathfinding cycle, see Flag::mpf_cycle
};

struct Cmd_Call_Economy_Balance : public GameLogicCommand {
	Cmd_Call_Economy_Balance () : GameLogicCommand (0) {} ///< for load and save

	Cmd_Call_Economy_Balance (int32_t starttime, Economy *, uint32_t timerid);

	void execute (Game &);

	virtual uint8_t id() const {return QUEUE_CMD_CALL_ECONOMY_BALANCE;}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

private:
	OPtr<Flag> m_flag;
	uint32_t m_timerid;
};

};

#endif
