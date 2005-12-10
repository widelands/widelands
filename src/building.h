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

#ifndef BUILDING_H
#define BUILDING_H

#include <string>
#include <vector>
#include "immovable.h"
#include "instances.h"
#include "ware.h"
#include "workarea_info.h"

class Flag;
class Interactive_Player;
class Tribe_Descr;
class Profile;
class UIWindow;
struct EncodeData;

class Building;
class BuildingHints;

/*
 * Common to all buildings!
 */
class Building_Descr : public Map_Object_Descr {
   friend class Widelands_Map_Buildingdata_Data_Packet;

public:
	struct CostItem {
		std::string name;   // name of ware
		int         amount; // amount

		inline CostItem(const char* iname, int iamount)
			: name(iname), amount(iamount) {}
	};
	typedef std::vector<CostItem> BuildCost;

public:
	Building_Descr(Tribe_Descr* tribe, const char* name);
	virtual ~Building_Descr(void);

	inline const char* get_name(void) const { return m_name; }
	inline const char* get_descname() const { return m_descname; }
	inline bool get_buildable(void) const { return m_buildable; }
   inline bool get_enhanced_building(void) const { return m_enhanced_building; }
	inline const BuildCost* get_buildcost() const { return &m_buildcost; }
	inline uint get_buildicon() const { return m_buildicon; }
	inline int get_size(void) const { return m_size; }
	inline bool get_ismine() const { return m_mine; }
   virtual uint get_ui_anim() { return get_animation("idle"); }

	inline bool get_stopable() const { return m_stopable;}
	inline std::string get_stop_icon() const { return m_stop_icon;}
	inline std::string get_continue_icon() const { return m_continue_icon;}
   inline const std::vector<char*>* get_enhances_to() const { return &m_enhances_to; }

	Building* create(Editor_Game_Base* g, Player* owner, Coords pos,
		bool construct, Building_Descr* = 0);
	virtual void parse(const char* directory, Profile* prof,
		const EncodeData* encdata);
	virtual void load_graphics();

	virtual int get_conquers(void) const { return 0; }
	
   inline Tribe_Descr* get_tribe(void) const { return m_tribe; }
	Workarea_Info m_workarea_info, m_recursive_workarea_info;
	
	const BuildingHints* get_hints() const { return m_hints; }

protected:
	virtual Building* create_object() = 0;
	Building* create_constructionsite(Building_Descr* );
	bool         m_stopable;
	std::string  m_stop_icon;
	std::string  m_continue_icon;

private:
	Tribe_Descr* m_tribe;           // the tribe this building belongs to
	char         m_name[20];        // internal codename
	char         m_descname[30];    // descriptive name for GUI
	bool         m_buildable;       // the player can build this himself
	BuildCost    m_buildcost;
	uint         m_buildicon;       // if buildable: picture in the build dialog
	char*        m_buildicon_fname; // filename for this icon
	int          m_size;            // size of the building
	bool         m_mine;
	std::vector<char*> m_enhances_to;     // building to enhance to or 0
	bool         m_enhanced_building; // if it is one, it is bulldozable
	BuildingHints*	m_hints;	// hints (knowledge) for computer players

public:
	static Building_Descr* create_from_dir(Tribe_Descr* tribe,
		const char* directory, const EncodeData* encdata);
};


class Building : public PlayerImmovable {
	friend class Building_Descr;
   friend class Widelands_Map_Buildingdata_Data_Packet;

	MO_DESCR(Building_Descr)

public:
	// Player capabilities: which commands can a player issue for this building?
	enum {
		PCap_Bulldoze = 0, // can bulldoze/remove this buildings
		PCap_Stopable = 1, // is stopable
		PCap_Enhancable = 3, // can be enhanced to something
	};

public:
   enum Type {
      PRODUCTIONSITE=0,
      CONSTRUCTIONSITE,
      MILITARYSITE,
      WAREHOUSE,
		TRAININGSITE
   };
   
	Building(Building_Descr* descr);
	virtual ~Building();

   virtual int get_building_type()=0;

	virtual int get_type();
	virtual int get_size();
	virtual bool get_passable();
   virtual uint get_ui_anim();

	virtual Flag* get_base_flag();
	virtual uint get_playercaps();
	virtual Coords get_position() const { return m_position; }

	inline const char* get_name() { return get_descr()->get_name(); }
	inline const char* get_descname() { return get_descr()->get_descname(); }

	virtual std::string get_census_string();
	virtual std::string get_statistics_string();

	virtual bool burn_on_destroy();
	virtual void destroy(Editor_Game_Base*);

	void show_options(Interactive_Player* plr);
	void hide_options();

	virtual bool fetch_from_flag(Game* g);
	virtual bool get_building_work(Game* g, Worker* w, bool success);

	bool leave_check_and_wait(Game* g, Worker* w);
	inline int get_conquers(void) const { return get_descr()->get_conquers(); }

	inline std::string get_stop_icon() const { return get_descr()->get_stop_icon(); }
	inline std::string get_continue_icon() const { return get_descr()->get_continue_icon(); }
	inline bool get_stop() const { return m_stop; }
	virtual void set_stop(bool stop);

   inline const std::vector<char*>* get_enhances_to() const { return get_descr()->get_enhances_to(); }
   
   void log_general_info(Editor_Game_Base* egbase);

    // Use on military and training sites
   virtual void drop_soldier(uint) { };
	virtual	void soldier_capacity_up ()		{ }
	virtual	void soldier_capacity_down ()	{ }

    // Use on training sites only
   virtual void change_train_priority(uint, int) { };
	virtual void switch_train_mode () { };

   /// Testing stuff
   virtual bool has_soldiers() { return false; };
   virtual void conquered_by (Player*);
protected:
	void start_animation(Editor_Game_Base* g, uint anim);

	virtual void init(Editor_Game_Base* g);
	virtual void cleanup(Editor_Game_Base* g);
	virtual void act(Game* g, uint data);

	virtual void draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords,
		Point pos);
	void draw_help(Editor_Game_Base* game, RenderTarget* dst, FCoords coords,
		Point pos);

	virtual UIWindow* create_options_window(Interactive_Player* plr,
		UIWindow** registry) = 0;

protected:
	UIWindow* m_optionswindow;
	Coords    m_position;
	Flag*     m_flag;

	uint m_anim;
	int  m_animstart;

	std::vector<Object_Ptr> m_leave_queue; // FIFO queue of workers leaving the building
	uint                    m_leave_time;  // when to wake the next one from leave queue
	Object_Ptr              m_leave_allow; // worker that is allowed to leave now
	bool m_stop;
};

#endif
