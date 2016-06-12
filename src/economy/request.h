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

#ifndef WL_ECONOMY_REQUEST_H
#define WL_ECONOMY_REQUEST_H

#include "economy/trackptr.h"
#include "logic/map_objects/tribes/requirements.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/widelands.h"

class FileRead;
class FileWrite;

namespace Widelands {

class Economy;
class EditorGameBase;
struct Flag;
class Game;
class MapObjectLoader;
struct MapObjectSaver;
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

	using CallbackFn = void (*)
		(Game &, Request &, DescriptionIndex, Worker *, PlayerImmovable &);

	Request(PlayerImmovable & target, DescriptionIndex, CallbackFn, WareWorker);
	~Request();

	PlayerImmovable & target() const {return target_;}
	DescriptionIndex get_index() const {return index_;}
	WareWorker get_type() const {return type_;}
	Quantity get_count() const {return count_;}
	uint32_t get_open_count() const {return count_ - transfers_.size();}
	bool is_open() const {return transfers_.size() < count_;}
	Economy * get_economy() const {return economy_;}
	int32_t get_required_time() const;
	int32_t get_last_request_time() const {return last_request_time_;}
	int32_t get_priority(int32_t cost) const;
	uint32_t get_transfer_priority() const;
	uint32_t get_num_transfers() const {return transfers_.size();}

	Flag & target_flag() const;

	void set_economy(Economy *);
	void set_count(Quantity);
	void set_required_time(int32_t time);
	void set_required_interval(int32_t interval);

	void set_last_request_time(int32_t const time) {last_request_time_ = time;}

	void start_transfer(Game &, Supply &);

	void read (FileRead  &, Game &, MapObjectLoader &);
	void write(FileWrite &, Game &, MapObjectSaver  &) const;
	Worker * get_transfer_worker();

	//  callbacks for WareInstance/Worker code
	void transfer_finish(Game &, Transfer &);
	void transfer_fail  (Game &, Transfer &);

	void set_requirements (const Requirements & r) {requirements_ = r;}
	const Requirements & get_requirements () const {return requirements_;}

private:
	int32_t get_base_required_time(EditorGameBase &, uint32_t nr) const;
public:
	void cancel_transfer(uint32_t idx);
private:
	void remove_transfer(uint32_t idx);
	uint32_t find_transfer(Transfer &);

	using TransferList = std::vector<Transfer *>;

	WareWorker type_;

	PlayerImmovable & target_;            //  who requested it?
	//  Copies of target_ of various pointer types, to avoid expensive
	//  dynamic casting at runtime. Variables with an incompatible type
	//  are filled with nulls.
	Building        * target_building_;
	ProductionSite  * target_productionsite_;
	Warehouse       * target_warehouse_;
	ConstructionSite * target_constructionsite_;

	Economy         * economy_;
	DescriptionIndex        index_;             //  the index of the ware descr
	Quantity          count_;             //  how many do we need in total

	CallbackFn        callbackfn_;        //  called on request success

	//  when do we need the first ware (can be in the past)
	int32_t           required_time_;
	int32_t           required_interval_; //  time between wares
	int32_t           last_request_time_;

	TransferList      transfers_;         //  maximum size is count_

	Requirements requirements_;
};

}

#endif  // end of include guard: WL_ECONOMY_REQUEST_H
