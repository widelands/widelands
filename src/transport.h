/*
 * Copyright (C) 2002 by the Widelands Development Team
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
#include "ware.h"


class Flag;
class Road;
class Request;
class Economy;

struct Neighbour {
	Flag	*flag;
	Road	*road;
	int	cost;
};
typedef std::vector<Neighbour> Neighbour_list;

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

public:
	Flag();
	virtual ~Flag();
	
	static Flag *create(Game *g, Player *owner, Coords coords);
	
	virtual int get_type();
	virtual int get_size();
	virtual bool get_passable();
	
	virtual Flag *get_base_flag();
	
	inline const Coords &get_position() const { return m_position; }
	
	virtual void set_economy(Economy *e);
	
	inline Building *get_building() { return m_building; }
	void attach_building(Game *g, Building *building);
	void detach_building(Game *g);
	
	inline Road *get_road(int dir) { return m_roads[dir-1]; }
	void attach_road(int dir, Road *road);
	void detach_road(int dir);

	void get_neighbours(Neighbour_list *neighbours);
	Road *get_road(Flag *flag);
	
protected:
	virtual void init(Game*);
	virtual void cleanup(Game*);
	
	virtual void draw(Game* game, RenderTarget* dst, FCoords coords, int posx, int posy);

private:
	Coords		m_position;
	Animation	*m_anim;
	int			m_animstart;
	
	Building		*m_building;	// attached building (replaces road WALK_NW)
	Road			*m_roads[6];	// Map_Object::WALK_xx-1 as index
	
	// The following are only used during pathfinding
	uint			mpf_cycle;
	int			mpf_heapindex;
	int			mpf_realcost;	// real cost of getting to this flag
	Flag*			mpf_backlink;	// flag where we came from
	int			mpf_estimate;	// estimate of cost to destination
	
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
*/
class Road : public PlayerImmovable {
public:
	Road();
	virtual ~Road();

	static Road *create(Game *g, int type, Flag *start, Flag *end, const Path &path);

	inline Flag *get_flag_start() const { return m_start; }
	inline Flag *get_flag_end() const { return m_end; }

	virtual int get_type();
	virtual int get_size();
	virtual bool get_passable();

	virtual Flag *get_base_flag();
	
	int get_cost(bool reverse);
	inline const Path &get_path() const { return m_path; }
	
	void presplit(Game *g, Coords split);
	void postsplit(Game *g, Flag *flag);
	
protected:
	void set_path(Game *g, const Path &path);

	void mark_map(Game *g);
	void unmark_map(Game *g);

	virtual void init(Game *g);
	virtual void cleanup(Game *g);
	
	virtual void request_success(Request *req);
	
	virtual void draw(Game* game, RenderTarget* dst, FCoords coords, int posx, int posy);

private:
	int		m_type;		// use Field::Road_XXX
	Flag		*m_start;
	Flag		*m_end;
	int		m_cost_forward;	// cost for walking this road from start to end
	int		m_cost_backward;	// dito, from end to start
	Path		m_path;		// path goes from m_start to m_end

	Object_Ptr	m_carrier;	// our carrier
	Request		*m_carrier_request;
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
A Request is issued whenever some object (road or building) needs a ware.

Important: While you must reassign the wares that e.g. a building owns when
the building moves to another economy (in a split/merge), DO NOT reassign
requests. Requests are treated in a special way by the Economy split/merge
code since it can be done more efficiently there.
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

	Request(PlayerImmovable *target, int ware);
	~Request();
	
	inline PlayerImmovable* get_target(Game*g) { return (PlayerImmovable*)m_target.get(g); }
	inline int get_ware() const { return m_ware; }
	inline int get_state() const { return m_state; }
	
	Flag *get_target_flag(Game *g);
	Economy *get_target_economy(Game *g);
	
	void start_transfer(Game *g, Warehouse *wh, Route *route);
	void start_transfer(Game *g, Worker *worker, Route *route);
	
	void check_transfer(Game *g);
	void cancel_transfer(Game *g);
	
	void transfer_finish(Game *g);
	void transfer_fail(Game *g);
	
private:
	Object_Ptr	m_target;	// who requested it?
	int			m_ware;		// the ware type
	
	int			m_state;
	union {
		Worker*	worker;		// worker that is supposed to fulfill the request
	} m_transfer;
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
WareBuffer can be used in Buildings to conveniently store and request wares.
*/
/*
class WareBuffer {
public:
	WareBuffer(int ware, int size);
	~WareBuffer();
	
	void add_to_economy(Economy *e);
	void remove_from_economy(Economy *e);

private:
	int		m_ware;		// the type of ware that is buffered
	int		m_size;		// maximum number of items we can hold
	int		m_amount;	// number of item the buffer currently holds
};
*/

/*
Economy represents a network of Flag through which wares can be transported.
*/
class Economy {
	friend class Request;

public:
	Economy(Player *player);
	~Economy();

	inline Player *get_owner() const { return m_owner; }	
	
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
	
	void add_request(Request *req);
	void remove_request(Request *req);
	
private:
	void do_remove_flag(Flag *f);
	
	void do_merge(Economy *e);
	void do_split(Flag *f);

	bool process_request(Request *req);
	int match_requests(Warehouse *wh);
	
private:
	Player	*m_owner;
	std::vector<Flag*>		m_flags;
	WareList						m_wares;	// virtual storage with all wares in this Economy
	std::vector<Warehouse*>	m_warehouses;
	
	std::vector<RequestList>	m_requests; // requests by ware id
	
	uint		mpf_cycle;		// pathfinding cycle, see Flag::mpf_cycle
};

#endif // included_transport_h
