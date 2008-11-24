/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef CRITTER_BOB_H
#define CRITTER_BOB_H

#include "bob.h"

namespace Widelands {

class Critter_BobAction;
class Critter_BobProgram;

//
// Description
//
struct Critter_Bob_Descr : public Bob::Descr {
	Critter_Bob_Descr
		(char const * name, char const * descname,
		 std::string const & directory, Profile &, Section & global_s,
		 Tribe_Descr const *, EncodeData const * = 0);
	virtual ~Critter_Bob_Descr();

	Bob * create_object() const;

	bool is_swimming() const throw () {return m_swimming;}
	uint32_t movecaps() const throw ();
	const DirAnimations & get_walk_anims() const throw () {return m_walk_anims;}
	__attribute__ ((deprecated)) const char * get_descname() const throw () {return descname().c_str();}

	Critter_BobProgram const * get_program
		(std::string const & programname) const;

private:
	DirAnimations m_walk_anims;
	bool          m_swimming;
	typedef std::map<std::string, Critter_BobProgram *> Programs;
	Programs    m_programs;
};

class Critter_Bob : public Bob {
	friend struct Map_Bobdata_Data_Packet;
	friend class Critter_BobProgram;

	MO_DESCR(Critter_Bob_Descr);

public:
	Critter_Bob(const Critter_Bob_Descr &);
	virtual ~Critter_Bob();

	char const * type_name() const throw () {return "critter";}
	virtual Bob::Type get_bob_type() const throw () {return Bob::CRITTER;}

	virtual void init_auto_task(Game* g);

	void start_task_program(Game* g, const std::string & name);
	const std::string & descname() const throw () {return descr().descname();}
	__attribute__ ((deprecated)) const char * get_descname() const throw ()  {return descname().c_str();}

private:
	void roam_update(Game* g, State* state);
	void program_update(Game* g, State* state);

private:
	bool run_remove(Game *, State *, Critter_BobAction const *);

private:
	static Task taskRoam;
	static Task taskProgram;
};

};

#endif
