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


class MilitarySite_Descr : public ProductionSite_Descr {
public:
	MilitarySite_Descr(Tribe_Descr* tribe, const char* name);
	virtual ~MilitarySite_Descr();

	virtual void parse(const char* directory, Profile* prof,
		const EncodeData* encdata);
	virtual Building* create_object();

	virtual bool is_only_production_site(void) { return false; }

	virtual int get_conquers(void) const { return m_conquer_radius; }
	inline  int get_max_number_of_soldiers(void) { return m_num_soldiers; }
	inline  int get_max_number_of_medics(void) { return m_num_medics; }
	inline  int get_heal_per_second(void) { return m_heal_per_second; }
	inline  int get_heal_increase_per_medic(void) {
		return m_heal_incr_per_medic;
	}

private:
	int m_conquer_radius;
	int m_num_soldiers;
	int m_num_medics;
	int m_heal_per_second;
	int m_heal_incr_per_medic;
};

class MilitarySite : public ProductionSite {
	MO_DESCR(MilitarySite_Descr);

public:
	MilitarySite(MilitarySite_Descr* descr);
	virtual ~MilitarySite();

	virtual std::string get_statistics_string();

	virtual void init(Editor_Game_Base* g);
	virtual void cleanup(Editor_Game_Base* g);
	virtual void act(Game* g, uint data);

	virtual void set_economy(Economy* e);

	/*
	get_soldiersqueue()
	release_soldier()
	request_soldier()
	*/

protected:
	virtual UIWindow* create_options_window(Interactive_Player* plr,
		UIWindow** registry);

private:
	void request_soldier(Game* g);
	static void request_soldier_callback(Game* g, Request* rq, int ware,
		Worker* w, void* data);

private:
	Request* m_soldier_request;
	Worker*  m_soldier;
	bool		m_didconquer;
	//Soldier*
	//Soldier_Queue
};

#endif
