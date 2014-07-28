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

#ifndef WL_LOGIC_CRITTER_BOB_H
#define WL_LOGIC_CRITTER_BOB_H

#include "base/macros.h"
#include "logic/bob.h"
#include "graphic/diranimations.h"

class LuaTable;
class OneWorldLegacyLookupTable;

namespace Widelands {

struct Critter_BobAction;
struct Critter_BobProgram;

//
// Description
//
struct Critter_Bob_Descr : BobDescr {
	Critter_Bob_Descr
		(char const* const _name,
		 char const* const _descname,
		 const std::string& directory,
		 Profile& prof,
		 Section& global_s,
		 Tribe_Descr & _tribe);
	Critter_Bob_Descr(const LuaTable&);
	~Critter_Bob_Descr() override;

	Bob & create_object() const override;

	bool is_swimming() const;
	uint32_t movecaps() const override;
	const DirAnimations & get_walk_anims() const {return m_walk_anims;}

	Critter_BobProgram const * get_program(const std::string &) const;


private:
	DirAnimations m_walk_anims;
	typedef std::map<std::string, Critter_BobProgram *> Programs;
	Programs      m_programs;
	DISALLOW_COPY_AND_ASSIGN(Critter_Bob_Descr);
};

class Critter_Bob : public Bob {
	friend struct Map_Bobdata_Data_Packet;
	friend struct Critter_BobProgram;

	MO_DESCR(Critter_Bob_Descr)

public:
	Critter_Bob(const Critter_Bob_Descr &);

	void init_auto_task(Game &) override;

	void start_task_program(Game &, const std::string & name);

	void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;

	static MapObject::Loader*
	load(Editor_Game_Base&, MapMapObjectLoader&, FileRead&, const OneWorldLegacyLookupTable& lookup_table);

protected:
	struct Loader : Bob::Loader {
		Loader();

		const Task * get_task(const std::string & name) override;
		const BobProgramBase * get_program(const std::string & name) override;
	};

private:
	void roam_update   (Game &, State &);
	void program_update(Game &, State &);

	bool run_remove(Game &, State &, const Critter_BobAction &);

	static Task const taskRoam;
	static Task const taskProgram;

};

}

#endif  // end of include guard: WL_LOGIC_CRITTER_BOB_H
