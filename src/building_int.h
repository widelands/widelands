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
// only to be included by building*.cc; if you need anything from this
// file somewhere else, move the declarations to building.h

#ifndef included_building_int_h
#define included_building_int_h


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

	virtual void parse(const char* directoy, Profile* prof, const EncodeData* encdata);
	virtual Building* create_object();
};

class ConstructionSite : public Building {
	MO_DESCR(ConstructionSite_Descr);

public:
	ConstructionSite(ConstructionSite_Descr* descr);
	virtual ~ConstructionSite();

	virtual int get_size();
	virtual uint get_playercaps();

	void set_building(Building_Descr* descr);
	inline Building_Descr* get_building() const { return m_building; }

	virtual void init(Editor_Game_Base* g);
	virtual void cleanup(Editor_Game_Base* g);

	virtual void set_economy(Economy* e);

protected:
	virtual Window *create_options_window(Interactive_Player *plr, Window **registry);

	void request_builder(Game* g);
	static void request_builder_callback(Game* g, Request* rq, int ware, Worker* w, void* data);

private:
	Building_Descr*	m_building; // type of building that is being built

	Request*				m_builder_request;
	Worker*				m_builder;

	std::vector<WaresQueue*>	m_wares;
};


/*
ProductionSite
--------------
Every building that is part of the economics system is a production site.

A production site has a worker.
A production site can have one (or more) output wares types (in theory it should
  be possible to burn wares for some virtual result such as "mana", or maybe
  even just for the fun of it, although that's not planned).
A production site can have one (or more) input wares types. Every input
  wares type has an associated store.
*/
class ProductionSite_Descr : public Building_Descr {
public:
	ProductionSite_Descr(Tribe_Descr *tribe, const char *name);

	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
	virtual Building *create_object();
};

class ProductionSite : public Building {
	MO_DESCR(ProductionSite_Descr);

public:
	ProductionSite(ProductionSite_Descr* descr);
	virtual ~ProductionSite();

	virtual void init(Editor_Game_Base *g);
	virtual void cleanup(Editor_Game_Base *g);

protected:
	virtual Window *create_options_window(Interactive_Player *plr, Window **registry);
};


#endif // included_building_int_h
