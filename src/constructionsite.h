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

#ifndef CONSTRUCTIONSITE_H
#define CONSTRUCTIONSITE_H

#include <vector>
#include "building.h"
#include "types.h"

class Building;
class Request;
class WaresQueue;

/*
ConstructionSite
----------------
A "building" that acts as a placeholder during construction work.
Once the construction phase is finished, the ConstructionSite is removed
and replaced by the actual building.

A construction site can have a worker.
A construction site has one (or more) input wares types, each with an
  associated store.

Note that the ConstructionSite_Descr class is mostly a dummy class.
The ConstructionSite is derived from Building so that it fits in more cleanly
with the transport and Flag code.

Every tribe has exactly one ConstructionSite_Descr.
The ConstructionSite's idling animation is the basic construction site marker.
*/
class ConstructionSite_Descr : public Building_Descr {
public:
	ConstructionSite_Descr(Tribe_Descr* tribe, const char* name);

	virtual void parse(const char* directoy, Profile* prof,
		const EncodeData* encdata);
	virtual Building* create_object();
};

class ConstructionSite : public Building {
	MO_DESCR(ConstructionSite_Descr);

public:
	ConstructionSite(ConstructionSite_Descr* descr);
	virtual ~ConstructionSite();

	virtual int get_size();
	virtual uint get_playercaps();
	virtual uint get_ui_anim();
	virtual std::string get_census_string();
	virtual std::string get_statistics_string();
	uint get_built_per64k();

	void set_building(Building_Descr* descr);
   void set_previous_building(Building_Descr* descr);
   inline Building_Descr* get_building() const { return m_building; }

	virtual void init(Editor_Game_Base* g);
	virtual void cleanup(Editor_Game_Base* g);

	virtual bool burn_on_destroy();

	virtual void set_economy(Economy* e);

	uint get_nrwaresqueues() { return m_wares.size(); }
	WaresQueue* get_waresqueue(uint idx) { return m_wares[idx]; }

	virtual bool fetch_from_flag(Game* g);
	virtual bool get_building_work(Game* g, Worker* w, bool success);

protected:
	virtual UIWindow* create_options_window(Interactive_Player* plr,
		UIWindow** registry);

	void request_builder(Game* g);
	static void request_builder_callback(Game* g, Request* rq, int ware,
		Worker* w, void* data);
	static void wares_queue_callback(Game* g, WaresQueue* wq, int ware,
		void* data);

	virtual void draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords,
		Point pos);

private:
	Building_Descr* m_building; // type of building that is being built
   Building_Descr* m_prev_building; // Building that was standing here before

	Request* m_builder_request;
	Worker*  m_builder;

	std::vector<WaresQueue*> m_wares;

	int m_fetchfromflag; // # of items to fetch from flag

	bool m_working;        // true if the builder is currently working
	uint m_work_steptime;  // time when next step is completed
	uint m_work_completed; // how many steps have we done so far?
	uint m_work_steps;     // how many steps (= items) until we're done?
};

#endif
