/*
 * Copyright (C) 2002-2004, 2007-2008 by the Widelands Development Team
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
#include "requirements.h"

namespace Widelands {

class Soldier;

struct MilitarySite_Descr : public ProductionSite_Descr {
	MilitarySite_Descr
		(const Tribe_Descr &, const std::string & militarysite_name);
	virtual ~MilitarySite_Descr();

	virtual void parse(char const * directory, Profile *, EncodeData const *);
	virtual Building * create_object() const;

	virtual bool is_only_production_site() const throw () {return false;}

	virtual uint32_t get_conquers() const {return m_conquer_radius;}
	int32_t get_max_number_of_soldiers () const throw () {return m_num_soldiers;}
	int32_t get_max_number_of_medics   () const throw () {return m_num_medics;}
	int32_t get_heal_per_second        () const throw () {return m_heal_per_second;}
	int32_t get_heal_increase_per_medic() const throw ()
	{return m_heal_incr_per_medic;}

private:
	int32_t m_conquer_radius;
	int32_t m_num_soldiers;
	int32_t m_num_medics;
	int32_t m_heal_per_second;
	int32_t m_heal_incr_per_medic;
};

class MilitarySite : public ProductionSite {
	friend struct Map_Buildingdata_Data_Packet;
	MO_DESCR(MilitarySite_Descr);

public:
	MilitarySite(MilitarySite_Descr const &);
	virtual ~MilitarySite();

	virtual int32_t get_building_type() const throw ()
	{return Building::MILITARYSITE;}
	char const * type_name() const throw () {return "militarysite";}
	virtual std::string get_statistics_string();

	void fill(Game &);

	virtual void init(Editor_Game_Base* g);
	virtual void cleanup(Editor_Game_Base* g);
	virtual void act(Game* g, uint32_t data);

	virtual void set_economy(Economy* e);
	virtual const std::vector<Soldier *> & get_soldiers() const throw ()
	{return m_soldiers;}

	uint32_t get_capacity () const throw () {return m_capacity;}
	//  overload of building functions
	virtual void drop_soldier (uint32_t serial);
	virtual void soldier_capacity_up   () {change_soldier_capacity  (1);}
	virtual void soldier_capacity_down () {change_soldier_capacity (-1);}

	/// This methods are helper for use at configure this site.
	void set_requirements (const Requirements&);
	void  clear_requirements ();
	const Requirements& get_requirements () const {return m_soldier_requirements;}

	/*
      So, to set a new requirement to the request you should do something like:

         Requirements new_req;
         new_req.set (atribute, min_value, max_value);
         new_req.set (atribute, min_value, max_value);
         ms->set_requirements (new_req);

	*/

	/// Testing stuff
	uint32_t nr_attack_soldiers();
	void set_in_battle(bool const in_battle) {m_in_battle = in_battle;};

	virtual bool has_soldiers() {return m_soldiers.size() > 0;}

protected:
	void conquer_area(Game &);

	virtual UI::Window * create_options_window
		(Interactive_Player * plr, UI::Window * * registry);

private:
	void request_soldier();
	static void request_soldier_callback
		(Game *, Request *, Ware_Index, Worker *, void * data);

	void drop_soldier (Game *g, int32_t i);
	void call_soldiers();
	void change_soldier_capacity (int32_t);
private:
	Requirements m_soldier_requirements;
	std::vector<Request*> m_soldier_requests;
	std::vector<Soldier*> m_soldiers;
	bool                   m_didconquer;
	uint32_t                   m_capacity;
	bool     m_in_battle;
};

};

#endif
