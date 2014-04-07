/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef REQUEST_H
#define REQUEST_H

#include "logic/requirements.h"
#include "logic/wareworker.h"
#include "logic/widelands.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "trackptr.h"

namespace Widelands {

class Economy;
class Editor_Game_Base;
struct Flag;
class Game;
class Map_Map_Object_Loader;
struct Map_Map_Object_Saver;
struct PlayerImmovable;
class RequestList;
struct Requirements;
struct Supply;
struct Transfer;
class Worker;
class Building;
class ProductionSite;
class Warehouse;
class ConstructionSite;

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
class Request : public Trackable {
public:
	friend class Economy;
	friend class RequestList;

	typedef void (*callback_t)
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

	Request(PlayerImmovable & target, Ware_Index, callback_t, WareWorker);
	~Request();

	PlayerImmovable & target() const {return m_target;}
	Ware_Index get_index() const {return m_index;}
	WareWorker get_type() const {return m_type;}
	uint32_t get_count() const {return m_count;}
	uint32_t get_open_count() const {return m_count - m_transfers.size();}
	bool is_open() const {return m_transfers.size() < m_count;}
	Economy * get_economy() const {return m_economy;}
	int32_t get_required_time() const;
	int32_t get_last_request_time() const {return m_last_request_time;}
	int32_t get_priority(int32_t cost) const;
	uint32_t get_transfer_priority() const;
	uint32_t get_num_transfers() const {return m_transfers.size();}

	Flag & target_flag() const;

	void set_economy(Economy *);
	void set_count(uint32_t);
	void set_required_time(int32_t time);
	void set_required_interval(int32_t interval);

	void set_last_request_time(int32_t const time) {m_last_request_time = time;}

	void start_transfer(Game &, Supply &);

	void Read (FileRead  &, Game &, Map_Map_Object_Loader &);
	void Write(FileWrite &, Game &, Map_Map_Object_Saver  &) const;
	Worker * get_transfer_worker();

	//  callbacks for WareInstance/Worker code
	void transfer_finish(Game &, Transfer &);
	void transfer_fail  (Game &, Transfer &);

	void set_requirements (const Requirements & r) {m_requirements = r;}
	const Requirements & get_requirements () const {return m_requirements;}

private:
	int32_t get_base_required_time(Editor_Game_Base &, uint32_t nr) const;
public:
	void cancel_transfer(uint32_t idx);
private:
	void remove_transfer(uint32_t idx);
	uint32_t find_transfer(Transfer &);

	typedef std::vector<Transfer *> TransferList;

	WareWorker m_type;

	PlayerImmovable & m_target;            //  who requested it?
	//  Copies of m_target of various pointer types, to avoid expensive
	//  dynamic casting at runtime. Variables with an incompatible type
	//  are filled with nulls.
	Building        * m_target_building;
	ProductionSite  * m_target_productionsite;
	Warehouse       * m_target_warehouse;
	ConstructionSite * m_target_constructionsite;

	Economy         * m_economy;
	Ware_Index        m_index;             //  the index of the ware descr
	uint32_t          m_count;             //  how many do we need in total

	callback_t        m_callbackfn;        //  called on request success

	//  when do we need the first ware (can be in the past)
	int32_t           m_required_time;
	int32_t           m_required_interval; //  time between wares
	int32_t           m_last_request_time;

	TransferList      m_transfers;         //  maximum size is m_count

	Requirements m_requirements;
};

}

#endif
