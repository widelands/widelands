/*
 * Copyright (C) 2002-2004, 2006-2009, 2011 by the Widelands Development Team
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

#ifndef CONSTRUCTIONSITE_H
#define CONSTRUCTIONSITE_H

#include <vector>

#include "logic/partially_finished_building.h"
#include "logic/player.h"

namespace Widelands {

class Building;
class Request;
struct WaresQueue;

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
struct ConstructionSite_Descr : public Building_Descr {
	ConstructionSite_Descr
		(char const * name, char const * descname,
		 const std::string & directory, Profile &, Section & global_s,
		 const Tribe_Descr & tribe);

	virtual Building & create_object() const override;
};

class ConstructionSite : public Partially_Finished_Building {
	friend struct Map_Buildingdata_Data_Packet;

	static const uint32_t CONSTRUCTIONSITE_STEP_TIME = 30000;

	MO_DESCR(ConstructionSite_Descr);

public:
	ConstructionSite(const ConstructionSite_Descr & descr);

	char const * type_name() const override {return "constructionsite";}
	virtual std::string get_statistics_string() override;

	const Player::Constructionsite_Information & get_info() {return m_info;}

	virtual WaresQueue & waresqueue(Ware_Index) override;

	virtual void set_building(const Building_Descr &) override;
	const Building_Descr & building() const {return *m_building;}

	virtual void init   (Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;

	virtual bool burn_on_destroy() override;

	virtual bool fetch_from_flag(Game &) override;
	virtual bool get_building_work(Game &, Worker &, bool success) override;

protected:
	virtual uint32_t build_step_time() const override {return CONSTRUCTIONSITE_STEP_TIME;}
	virtual void create_options_window
		(Interactive_GameBase &, UI::Window * & registry) override;

	static void wares_queue_callback
		(Game &, WaresQueue *, Ware_Index, void * data);

	virtual void draw(const Editor_Game_Base &, RenderTarget &, const FCoords&, const Point&) override;

private:
	int32_t  m_fetchfromflag;  // # of wares to fetch from flag

	bool     m_builder_idle;   // used to determine whether the builder is idle
	Player::Constructionsite_Information m_info; // asked for by player point of view for the gameview
};

}

#endif
