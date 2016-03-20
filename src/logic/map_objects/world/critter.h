/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_WORLD_CRITTER_H
#define WL_LOGIC_MAP_OBJECTS_WORLD_CRITTER_H

#include "base/macros.h"
#include "logic/map_objects/bob.h"
#include "graphic/diranimations.h"

class LuaTable;
class WorldLegacyLookupTable;

namespace Widelands {

struct CritterAction;
struct CritterProgram;

//
// Description
//
struct CritterDescr : BobDescr {
	CritterDescr(const std::string& init_descname, const LuaTable&);
	~CritterDescr() override;

	Bob & create_object() const override;

	bool is_swimming() const;
	uint32_t movecaps() const override;
	const DirAnimations& get_walk_anims() const {return walk_anims_;}

	CritterProgram const * get_program(const std::string &) const;


private:
	DirAnimations walk_anims_;
	using Programs = std::map<std::string, CritterProgram *>;
	Programs      programs_;
	DISALLOW_COPY_AND_ASSIGN(CritterDescr);
};

class Critter : public Bob {
	friend struct MapBobdataPacket;
	friend struct CritterProgram;

	MO_DESCR(CritterDescr)

public:
	Critter(const CritterDescr &);

	void init_auto_task(Game &) override;

	void start_task_program(Game &, const std::string & name);

	void save(EditorGameBase &, MapObjectSaver &, FileWrite &) override;

	static MapObject::Loader*
	load(EditorGameBase&, MapObjectLoader&, FileRead&, const WorldLegacyLookupTable& lookup_table);

protected:
	struct Loader : Bob::Loader {
		Loader();

		const Task * get_task(const std::string & name) override;
		const BobProgramBase * get_program(const std::string & name) override;
	};

private:
	void roam_update   (Game &, State &);
	void program_update(Game &, State &);

	bool run_remove(Game &, State &, const CritterAction &);

	static Task const taskRoam;
	static Task const taskProgram;

};

}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_WORLD_CRITTER_H
