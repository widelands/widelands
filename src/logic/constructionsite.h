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

#ifndef WL_LOGIC_CONSTRUCTIONSITE_H
#define WL_LOGIC_CONSTRUCTIONSITE_H

#include <vector>

#include "base/macros.h"
#include "logic/partially_finished_building.h"
#include "logic/player.h"

namespace Widelands {

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

Note that the ConstructionSiteDescr class is mostly a dummy class.
The ConstructionSite is derived from Building so that it fits in more cleanly
with the transport and Flag code.

Every tribe has exactly one ConstructionSiteDescr.
The ConstructionSite's idling animation is the basic construction site marker.
*/
struct ConstructionSiteDescr : public BuildingDescr {
	ConstructionSiteDescr
		(char const * name, char const * descname,
		 const std::string & directory, Profile &, Section & global_s,
		 const TribeDescr & tribe);
	~ConstructionSiteDescr() override {}

	Building & create_object() const override;

private:
	DISALLOW_COPY_AND_ASSIGN(ConstructionSiteDescr);
};

class ConstructionSite : public PartiallyFinishedBuilding {
	friend class MapBuildingdataPacket;

	static const uint32_t CONSTRUCTIONSITE_STEP_TIME = 30000;

	MO_DESCR(ConstructionSiteDescr)

public:
	ConstructionSite(const ConstructionSiteDescr & descr);

	const Player::ConstructionsiteInformation & get_info() {return m_info;}

	WaresQueue & waresqueue(WareIndex) override;

	void set_building(const BuildingDescr &) override;
	const BuildingDescr & building() const {return *m_building;}

	void init   (EditorGameBase &) override;
	void cleanup(EditorGameBase &) override;

	bool burn_on_destroy() override;

	bool fetch_from_flag(Game &) override;
	bool get_building_work(Game &, Worker &, bool success) override;

protected:
	void update_statistics_string(std::string* statistics_string) override;

	uint32_t build_step_time() const override {return CONSTRUCTIONSITE_STEP_TIME;}
	virtual void create_options_window
		(InteractiveGameBase &, UI::Window * & registry) override;

	static void wares_queue_callback
		(Game &, WaresQueue *, WareIndex, void * data);

	void draw(const EditorGameBase &, RenderTarget &, const FCoords&, const Point&) override;

private:
	int32_t     m_fetchfromflag;  // # of wares to fetch from flag

	bool        m_builder_idle;   // used to determine whether the builder is idle
	Player::ConstructionsiteInformation m_info; // asked for by player point of view for the gameview
};

}

#endif  // end of include guard: WL_LOGIC_CONSTRUCTIONSITE_H
