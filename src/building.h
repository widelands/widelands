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

#ifndef __S__BUILDING_H
#define __S__BUILDING_H

#include "immovable.h"
#include "ware.h"

class Economy;
class Flag;
class Interactive_Player;
class Tribe_Descr;
class Profile;
struct EncodeData;

class Building;

/*
 * Common to all buildings!
 */
class Building_Descr : public Map_Object_Descr {
public:
	struct CostItem {
		std::string		name;		// name of ware
		int				amount;	// amount

		inline CostItem(const char* iname, int iamount) : name(iname), amount(iamount) { }
	};
	typedef std::vector<CostItem> BuildCost;

public:
	Building_Descr(Tribe_Descr *tribe, const char *name);
	virtual ~Building_Descr(void);

	inline const char *get_name(void) { return m_name; }
	inline const char *get_descname() { return m_descname; }
	inline uint get_idle_anim(void) { return m_idle; }
	inline bool get_buildable(void) { return m_buildable; }
	inline const BuildCost* get_buildcost() const { return &m_buildcost; }
	inline uint get_buildicon() const { return m_buildicon; }
	inline int get_size(void) { return m_size; }
	inline bool get_ismine() { return m_mine; }

	Building *create(Editor_Game_Base *g, Player *owner, Coords pos, bool construct);
	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
	virtual void load_graphics();

protected:
	virtual Building *create_object() = 0;
	Building* create_constructionsite();

private:
	Tribe_Descr		*m_tribe;			// the tribe this building belongs to
	char				m_name[20];			// internal codename
	char				m_descname[30];	// descriptive name for GUI
	bool				m_buildable;		// the player can build this himself
	BuildCost		m_buildcost;
	uint				m_buildicon;		// if buildable: the picture used in the build dialog
	char*				m_buildicon_fname; // filename for this icon
	int				m_size;				// size of the building
	bool				m_mine;
	uint				m_idle;		// idle animation

public:
	static Building_Descr *create_from_dir(Tribe_Descr *tribe, const char *directory,
	                                       const EncodeData *encdata);
};

class Window;

class Building : public PlayerImmovable {
	friend class Building_Descr;

	MO_DESCR(Building_Descr)

public:
	// Player capabilities: which commands can a player issue for this building?
	enum {
		PCap_Bulldoze = 0,	// can bulldoze/remove this buildings
	};

public:
	Building(Building_Descr *descr);
	virtual ~Building();

	virtual int get_type();
	virtual int get_size();
	virtual bool get_passable();

	virtual Flag *get_base_flag();
	virtual uint get_playercaps();

	inline const char *get_name() { return get_descr()->get_name(); }
	inline const char *get_descname() { return get_descr()->get_descname(); }

	virtual bool burn_on_destroy();
	virtual void destroy(Editor_Game_Base*);

	void show_options(Interactive_Player *plr);
	void hide_options();

	virtual bool fetch_from_flag(Game* g);

	bool leave_check_and_wait(Game* g, Worker* w);

protected:
	void start_animation(Editor_Game_Base *g, uint anim);

	virtual void init(Editor_Game_Base *g);
	virtual void cleanup(Editor_Game_Base *g);
	virtual void act(Game *g, uint data);

	virtual void draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords, Point pos);

	virtual Window *create_options_window(Interactive_Player *plr, Window **registry) = 0;

protected:
	Window		*m_optionswindow;
	Coords		m_position;
	Flag			*m_flag;

	uint			m_anim;
	int			m_animstart;

	std::vector<Object_Ptr> m_leave_queue;		// FIFO queue of workers leaving the building
	uint							m_leave_time;		// when to wake the next one from leave queue
	Object_Ptr					m_leave_allow;		// worker that is allowed to leave now
};


/*
Warehouse
*/
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
	inline int get_conquers() const { return m_conquers; }

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

	inline const WareList &get_wares() const { return m_wares; }
	void create_wares(int id, int count);
	void destroy_wares(int id, int count);

	virtual bool fetch_from_flag(Game* g);

	Worker* launch_worker(Game* g, int ware);
	void incorporate_worker(Game *g, Worker *w);

	WareInstance* launch_item(Game* g, int ware);
	void incorporate_item(Game* g, WareInstance* item);

protected:
	virtual Window *create_options_window(Interactive_Player *plr, Window **registry);

private:
	WareList		m_wares;

	uint			m_next_carrier_spawn;		// time of next carrier growth
};


#endif // __S__BUILDING_H
