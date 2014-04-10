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

#ifndef CRITTER_BOB_H
#define CRITTER_BOB_H

#include "logic/bob.h"
#include "graphic/diranimations.h"

namespace Widelands {

struct Critter_BobAction;
struct Critter_BobProgram;

//
// Description
//
struct Critter_Bob_Descr : public BobDescr {
	Critter_Bob_Descr
		(char const * name, char const * descname,
		 const std::string & directory, Profile &, Section & global_s,
		 Tribe_Descr const *);
	virtual ~Critter_Bob_Descr();

	Bob & create_object() const override;

	bool is_swimming() const {return m_swimming;}
	uint32_t movecaps() const override;
	const DirAnimations & get_walk_anims() const {return m_walk_anims;}

	Critter_BobProgram const * get_program(const std::string &) const;

private:
	DirAnimations m_walk_anims;
	bool          m_swimming;
	typedef std::map<std::string, Critter_BobProgram *> Programs;
	Programs      m_programs;
};

class Critter_Bob : public Bob {
	friend struct Map_Bobdata_Data_Packet;
	friend struct Critter_BobProgram;

	MO_DESCR(Critter_Bob_Descr);

public:
	Critter_Bob(const Critter_Bob_Descr &);

	char const * type_name() const override {return "critter";}
	virtual Bob::Type get_bob_type() const override {return Bob::CRITTER;}

	virtual void init_auto_task(Game &) override;

	void start_task_program(Game &, const std::string & name);
	const std::string & descname() const {return descr().descname();}

private:
	void roam_update   (Game &, State &);
	void program_update(Game &, State &);

	bool run_remove(Game &, State &, const Critter_BobAction &);

	static Task const taskRoam;
	static Task const taskProgram;

	// saving and loading
protected:
	struct Loader : Bob::Loader {
		Loader();

		virtual const Task * get_task(const std::string & name) override;
		virtual const BobProgramBase * get_program(const std::string & name) override;
	};

public:
	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;

	static Map_Object::Loader * load
		(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);
};

}

#endif
