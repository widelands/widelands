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

#ifndef S__ROAD_H
#define S__ROAD_H

#include "immovable.h"
#include "logic/path.h"
#include "roadtype.h"

namespace Widelands {
struct Request;

/**
 * Road is a special object which connects two flags.
 * The Road itself is never rendered; however, the appropriate Field::roads are
 * set to represent the road visually.
 * The actual steps involved in a road are stored as a Path from the starting
 * flag to the ending flag. Apart from that, however, the two flags are treated
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
	virtual void init(Editor_Game_Base &);
	virtual void cleanup(Editor_Game_Base &);

	virtual void draw(Editor_Game_Base const &, RenderTarget &, FCoords, Point);

private:
	void _set_path(Editor_Game_Base &, Path const &);

	void _mark_map(Editor_Game_Base &);
	void _unmark_map(Editor_Game_Base &);

	void _link_into_flags(Editor_Game_Base &);

	void _request_carrier(Game &);
	static void _request_carrier_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

private:
	int32_t    m_type;       ///< use Field::Road_XXX
	Flag     * m_flags  [2]; ///< start and end flag
	int32_t    m_flagidx[2]; ///< index of this road in the flag's road array

	/// cost for walking this road (0 = from start to end, 1 = from end to start)
	int32_t    m_cost   [2];

	Path       m_path;       ///< path goes from start to end
	uint32_t   m_idle_index; ///< index into path where carriers should idle

	///< total number of carriers we want (currently limited to 0 or 1)
	uint32_t   m_desire_carriers;

	Object_Ptr m_carrier;    ///< our carrier
	Request *  m_carrier_request;
};

}

#endif


