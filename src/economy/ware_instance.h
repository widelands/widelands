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

#ifndef S__WARE_INSTANCE_H
#define S__WARE_INSTANCE_H

// For Ware Index
#include "widelands.h"
// For MO_DESCR
#include "instances.h"
// For Item_Ware_Descr
#include "item_ware_descr.h"


namespace Widelands {

class Economy;
class Editor_Game_Base;
class Game;
class IdleWareSupply;
class Map_Object;
class PlayerImmovable;
class Transfer;

/**
 * WareInstance represents one item while it is being carried around.
 *
 * The WareInstance never draws itself; the carrying worker or the current flag
 * location are responsible for that.
 *
 * The location of a ware can be one of the following:
 * \li a flag
 * \li a worker that is currently carrying the ware
 * \li a building; this should only be temporary until the ware is incorporated
 *     into the building somehow
 */
class WareInstance : public Map_Object {
	friend struct Map_Waredata_Data_Packet;

	MO_DESCR(Item_Ware_Descr);

public:
	WareInstance(Ware_Index, const Item_Ware_Descr * const);
	~WareInstance();

	virtual int32_t get_type() const throw ();
	char const * type_name() const throw () {return "ware";}

	Map_Object * get_location(Editor_Game_Base & egbase) {
		return m_location.get(egbase);
	}
	Economy * get_economy() const throw () {return m_economy;}
	Ware_Index descr_index() const throw () {return m_descr_index;}

	void init(Editor_Game_Base &);
	void cleanup(Editor_Game_Base &);
	void act(Game &, uint32_t data);
	void update(Game &);

	void set_location(Editor_Game_Base &, Map_Object * loc);
	void set_economy(Economy *);

	bool is_moving() const throw ();
	void cancel_moving();

	PlayerImmovable * get_next_move_step(Game &);

	void set_transfer(Game &, Transfer &);
	void cancel_transfer(Game &);

private:
	Object_Ptr        m_location;
	Economy         * m_economy;
	Ware_Index       m_descr_index;

	IdleWareSupply  * m_supply;
	Transfer       * m_transfer;
	Object_Ptr        m_transfer_nextstep; ///< cached PlayerImmovable, can be 0
};

}


#endif 

