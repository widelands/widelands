/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_FERRY_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_FERRY_H

#include <memory>

#include "base/macros.h"
#include "logic/map_objects/tribes/carrier.h"

namespace Widelands {

struct FerryFleet;
struct Waterway;
struct Coords;

class FerryDescr : public CarrierDescr {
public:
	FerryDescr(const std::string& init_descname, const LuaTable& table, Descriptions& descriptions);
	~FerryDescr() override {
	}

	uint32_t movecaps() const override;

protected:
	Bob& create_object() const override;

private:
	DISALLOW_COPY_AND_ASSIGN(FerryDescr);
};

/**
 * A ferry is a very special worker that rows along waterways.
 * It works exactly like a carrier, except that it swims.
 */
struct Ferry : public Carrier {
	friend struct MapBobdataPacket;

	MO_DESCR(FerryDescr)

	explicit Ferry(const FerryDescr& ferry_descr);
	~Ferry() override {
	}

	bool init(EditorGameBase&) override;
	void set_economy(Game&, Economy*, WareWorker);

	FerryFleet* get_fleet() const;

	Waterway* get_destination(const Game& game) const;
	void set_destination(Game& game, Waterway* ww);

	void init_auto_task(Game& game) override;
	void start_task_unemployed(Game&);
	void start_task_row(Game&, const Waterway&);

	bool unemployed();

private:
	friend struct FerryFleet;
	FerryFleet* fleet_;

	std::unique_ptr<Coords> destination_;

	bool init_fleet();
	void set_fleet(FerryFleet* fleet);

	static const Task taskUnemployed;
	static const Task taskRow;
	void unemployed_update(Game&, State&);
	void row_update(Game&, State&);

	Time unemployed_since_;

protected:
	void cleanup(EditorGameBase&) override;

	struct Loader : public Carrier::Loader {
	public:
		Loader() {
		}
		void load(FileRead&) override;

	protected:
		const Task* get_task(const std::string& name) override;
	};

	Loader* create_loader() override;

public:
	void do_save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_FERRY_H
