/*
 * Copyright (C) 2002-2004, 2007-2009 by the Widelands Development Team
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

#ifndef MILITARYSITE_H
#define MILITARYSITE_H

#include "logic/garrison.h"
#include "logic/garrisonhandler.h"
#include "logic/productionsite.h"
#include "logic/requirements.h"

namespace Widelands {

class Soldier;

struct MilitarySite_Descr : public ProductionSite_Descr {
	MilitarySite_Descr
		(char const * name, char const * descname,
		 const std::string & directory, Profile &,  Section & global_s,
		 const Tribe_Descr & tribe);

	virtual Building & create_object() const;

	virtual uint32_t get_conquers() const {return m_conquer_radius;}
	uint32_t get_max_number_of_soldiers () const throw () {
		return m_num_soldiers;
	}
	uint32_t get_heal_per_second        () const throw () {
		return m_heal_per_second;
	}

	bool     m_prefers_heroes_at_start;
private:
	uint32_t m_conquer_radius;
	uint32_t m_num_soldiers;
	uint32_t m_heal_per_second;
};

class MilitarySite :
	public ProductionSite, public GarrisonOwner
{
	friend struct Map_Buildingdata_Data_Packet;
	MO_DESCR(MilitarySite_Descr);

public:
	MilitarySite(const MilitarySite_Descr &);
	virtual ~MilitarySite();

	void load_finish(Editor_Game_Base &);
	virtual void init(Editor_Game_Base &);
	virtual void cleanup(Editor_Game_Base &);
	virtual void act(Game &, uint32_t data);

	char const * type_name() const throw () {return "militarysite";}
	virtual std::string get_statistics_string();

	virtual void set_economy(Economy *);
	virtual bool get_building_work(Game &, Worker &, bool success);

	// GarrisonOwner implementation
	virtual Garrison* get_garrison() const;
	virtual Building* get_building();
	virtual void garrison_occupied();
	virtual void garrison_lost(Game & game, Player_Number defeating, bool captured);
	virtual void reinit_after_conqueral(Game& game);

protected:
	virtual void create_options_window
		(Interactive_GameBase &, UI::Window * & registry);

private:
	std::unique_ptr<GarrisonHandler> m_garrison;
};

}

#endif
