/*
 * Copyright (C) 2004, 2006-2010 by the Widelands Development Team
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

#ifndef ROAD_H
#define ROAD_H

#include <vector>

#include "logic/immovable.h"
#include "logic/path.h"
#include "logic/roadtype.h"

namespace Widelands {
struct Carrier;
class Request;

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
 * Every road has one or more Carriers attached to it. Carriers are attached
 * when they arrive via the callback function passed to the request. The
 * callback then calls assign_carrier which incorporates this carrier on this
 * road.
 */
struct Road : public PlayerImmovable {
	friend struct Map_Roaddata_Data_Packet; // For saving
	friend struct Map_Road_Data_Packet; // For init()

	static bool IsRoadDescr(Map_Object_Descr const *);

	enum FlagId {
		FlagStart = 0,
		FlagEnd = 1
	};

	struct CarrierSlot {
		CarrierSlot();

		OPtr<Carrier> carrier;
		Request * carrier_request;
		uint8_t carrier_type;
	};

	Road();
	virtual ~Road();

	static Road & create
		(Editor_Game_Base &,
		 Flag & start, Flag & end, const Path &);

	Flag & get_flag(FlagId const flag) const {return *m_flags[flag];}

	virtual int32_t  get_type    () const override;
	uint8_t get_roadtype() const {return m_type;}
	char const * type_name() const override {return "road";}
	virtual int32_t  get_size    () const override;
	virtual bool get_passable() const override;
	virtual PositionList get_positions(const Editor_Game_Base &) const override;
	const std::string & name() const override;

	virtual Flag & base_flag() override;

	virtual void set_economy(Economy *) override;

	int32_t get_cost(FlagId fromflag);
	const Path & get_path() const {return m_path;}
	int32_t get_idle_index() const {return m_idle_index;}

	void presplit(Game &, Coords split);
	void postsplit(Game &, Flag &);

	bool notify_ware(Game & game, FlagId flagid);

	virtual void remove_worker(Worker &) override;
	void assign_carrier(Carrier &, uint8_t);

	void log_general_info(const Editor_Game_Base &) override;

protected:
	virtual void init(Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;

	virtual void draw(const Editor_Game_Base &, RenderTarget &, const FCoords&, const Point&) override;

private:
	void _set_path(Editor_Game_Base &, const Path &);

	void _mark_map(Editor_Game_Base &);
	void _unmark_map(Editor_Game_Base &);

	void _link_into_flags(Editor_Game_Base &);

	void _request_carrier(CarrierSlot &);
	static void _request_carrier_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

private:

	/// Counter that is incremented when a ware does not get a carrier for this
	/// road immediately and decremented over time.
	uint32_t   m_busyness;

	/// holds the gametime when m_busyness was last updated
	uint32_t   m_busyness_last_update;

	uint8_t    m_type;       ///< RoadType, 2 bits used
	Flag     * m_flags  [2]; ///< start and end flag
	int32_t    m_flagidx[2]; ///< index of this road in the flag's road array

	/// cost for walking this road (0 = from start to end, 1 = from end to start)
	int32_t    m_cost   [2];

	Path       m_path;       ///< path goes from start to end
	uint32_t   m_idle_index; ///< index into path where carriers should idle

	typedef std::vector<CarrierSlot> SlotVector;
	SlotVector m_carrier_slots;
};

}

#endif
