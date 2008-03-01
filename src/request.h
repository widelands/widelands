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

#ifndef REQUEST_H
#define REQUEST_H

#include "trackptr.h"
#include "transport.h" // for Requirements
#include "widelands.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"

namespace Widelands {

class Economy;
class Editor_Game_Base;
class Flag;
class Game;
class Map_Map_Object_Loader;
class Map_Map_Object_Saver;
class PlayerImmovable;
class RequestList;
class Requirements;
class Supply;
class Transfer;
class Worker;

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

public:
	void set_requirements (const Requirements& r) {m_requirements = r;}
private:
	const Requirements& get_requirements () {return m_requirements;}

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

	Requirements m_requirements;
};



}

#endif // REQUEST_H
