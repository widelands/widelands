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

#ifndef MILITARYSITE_H
#define MILITARYSITE_H

#include "productionsite.h"
#include "types.h"
#include "transport.h"

class Soldier;

class MilitarySite_Descr : public ProductionSite_Descr {
public:
	MilitarySite_Descr(Tribe_Descr* tribe, const char* name);
	virtual ~MilitarySite_Descr();

	virtual void parse(const char* directory, Profile* prof,
		const EncodeData* encdata);
	virtual Building* create_object();

	virtual bool is_only_production_site(void) { return false; }

	virtual int get_conquers(void) const { return m_conquer_radius; }
	int get_max_number_of_soldiers () const throw () {return m_num_soldiers;}
	int get_max_number_of_medics   () const throw () {return m_num_medics;}
	int get_heal_per_second        () const throw () {return m_heal_per_second;}
	int get_heal_increase_per_medic() const throw ()
	{return m_heal_incr_per_medic;}

private:
	int m_conquer_radius;
	int m_num_soldiers;
	int m_num_medics;
	int m_heal_per_second;
	int m_heal_incr_per_medic;
};

class MilitarySite : public ProductionSite {
   friend class Widelands_Map_Buildingdata_Data_Packet;
	MO_DESCR(MilitarySite_Descr);

public:
	MilitarySite(MilitarySite_Descr* descr);
	virtual ~MilitarySite();

	virtual int get_building_type() const throw ()
	{return Building::MILITARYSITE;}
	virtual std::string get_statistics_string();

	virtual void init(Editor_Game_Base* g);
	virtual void cleanup(Editor_Game_Base* g);
	virtual void act(Game* g, uint data);

	virtual void set_economy(Economy* e);

    virtual std::vector<Soldier*>* get_soldiers(void) { return &m_soldiers; }

	int	get_capacity() { return m_capacity; }
	// Overload of building functions
   virtual void drop_soldier (uint serial);
	virtual void soldier_capacity_up ()		{ change_soldier_capacity(1); }
	virtual void soldier_capacity_down ()	{ change_soldier_capacity(-1); }

   /**
      This methods are helper for use at configure this site.
   */
   void set_requeriments (Requeriments*);
   void  clear_requeriments ();
   Requeriments get_requeriments () { return m_soldier_requeriments; }

   /*
      So, to set a new requeriment to the request you should do something like:

         Requeriments new_req = ms->get_requeriments();
         new_req.set (atribute, min_value, max_value);
         new_req.set (atribute, min_value, max_value);
         ms->set_requeriments (&new_req);

   */

   /// Testing stuff
   int launch_attack(PlayerImmovable*, int);
   int can_launch_soldiers () { return m_soldiers.size() > 1; }

   virtual bool has_soldiers() { return m_soldiers.size() > 0; }
   virtual void defend (Game*, Soldier*);
   virtual void conquered_by (Player*);

protected:
	virtual UIWindow* create_options_window(Interactive_Player* plr,
		UIWindow** registry);

private:
	void request_soldier(Game* g);
	static void request_soldier_callback(Game* g, Request* rq, int ware,
		Worker* w, void* data);

	void drop_soldier (Game *g, int i);
	void call_soldiers(Game *g);
	void change_soldier_capacity (int);
private:
	Requeriments m_soldier_requeriments;
	std::vector<Request*> m_soldier_requests;
	std::vector<Soldier*> m_soldiers;
	bool		m_didconquer;
	uint		m_capacity;
};

#endif
