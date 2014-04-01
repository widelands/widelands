/*
 * Copyright (C) 2006-2011 by the Widelands Development Team
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

#ifndef PARTIALLY_FINISHED_BUILDING_H
#define PARTIALLY_FINISHED_BUILDING_H

#include "logic/building.h"

namespace Widelands {

class Request;
struct WaresQueue;

/*
Partially_Finished_Building
---------------------------

This class is the base for constructionsites and
dismantlesites.
*/
class Partially_Finished_Building : public Building {
	friend struct Map_Buildingdata_Data_Packet;
	friend struct Map_Building_Data_Packet;

public:
	Partially_Finished_Building(const Building_Descr & building_descr);

	virtual void set_building         (const Building_Descr &);

	virtual void init   (Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;

	virtual int32_t get_size() const override;
	virtual uint32_t get_playercaps() const override;
	virtual uint32_t get_ui_anim() const override;

	virtual void set_economy(Economy *) override;

	uint32_t get_nrwaresqueues() {return m_wares.size();}
	WaresQueue * get_waresqueue(uint32_t const idx) {return m_wares[idx];}

	uint32_t get_built_per64k() const;
	Request * get_builder_request() {return m_builder_request;}
	static void request_builder_callback(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

private:
	void request_builder(Game &);


	virtual uint32_t build_step_time() const = 0;

protected:
	const Building_Descr * m_building; // type of building that was or will become

	Request * m_builder_request;
	OPtr<Worker> m_builder;

	typedef std::vector<WaresQueue *> Wares;
	Wares m_wares;

	bool     m_working;        // true if the builder is currently working
	uint32_t m_work_steptime;  // time when next step is completed
	uint32_t m_work_completed; // how many steps have we done so far?
	uint32_t m_work_steps;     // how many steps (= wares) until we're done?
	//Player::Constructionsite_Information * m_info; // asked for by player point of view for the gameview
};

}

#endif /* end of include guard: PARTIALLY_FINISHED_BUILDING_H */
