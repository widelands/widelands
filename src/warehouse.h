/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include <map>
#include "building.h"

class Economy;
class Editor_Game_Base;
class Interactive_Player;
class Tribe_Descr;
class Profile;
class WareInstance;
class WareList;
struct EncodeData;


/*
Warehouse
*/
class WarehouseSupply;

class Warehouse_Descr : public Building_Descr {
public:
	enum {
		Subtype_Normal,
		Subtype_HQ,
		Subtype_Port
	};

	Warehouse_Descr(Tribe_Descr *tribe, const char *name);

	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
	virtual Building *create_object();

	inline int get_subtype() const { return m_subtype; }
	virtual int get_conquers(void) const { return m_conquers; }

private:
	int	m_subtype;
	int	m_conquers;		// HQs conquer
};


class Warehouse : public Building {
	MO_DESCR(Warehouse_Descr);

public:
	Warehouse(Warehouse_Descr *descr);
	virtual ~Warehouse();

	virtual void init(Editor_Game_Base *g);
	virtual void cleanup(Editor_Game_Base *g);

	virtual void act(Game *g, uint data);

	virtual void set_economy(Economy *e);

	const WareList &get_wares() const;
	void create_wares(int id, int count);
	void destroy_wares(int id, int count);

	virtual bool fetch_from_flag(Game* g);

	Worker* launch_worker(Game* g, int ware);
	void incorporate_worker(Game *g, Worker *w);

	WareInstance* launch_item(Game* g, int ware);
	void do_launch_item(Game* g, WareInstance* item);
	void incorporate_item(Game* g, WareInstance* item);

protected:
	virtual UIWindow *create_options_window(Interactive_Player *plr, UIWindow **registry);

private:
	static void idle_request_cb(Game* g, Request* rq, int ware, Worker* w, void* data);

private:
	WarehouseSupply*			m_supply;
	std::vector<Request*>	m_requests; // one idle request per ware type
   std::multimap<std::string,Worker*> m_incroporated_workers; // Workers who live here at the moment
	int m_next_carrier_spawn;		// time of next carrier growth
};


#endif
