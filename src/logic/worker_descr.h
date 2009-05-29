/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef WORKER_DESCR_H
#define WORKER_DESCR_H

#include "bob.h"

#include "immovable.h"
#include "io/filewrite.h"

namespace Widelands {

/// \todo (Antonio Trueba#1#): Get rid of forward class declaration
/// (chicked-and-egg problem)
class Worker;
struct WorkerProgram;


class Worker_Descr : public Bob::Descr
{
	friend struct Tribe_Descr;
	friend class Warehouse;
	friend struct WorkerProgram;

public:
	typedef std::map<std::string, uint8_t> Buildcost;

	typedef Ware_Index::value_t Index;
	enum Worker_Type {
		NORMAL = 0,
		CARRIER,
		SOLDIER,
	};

	Worker_Descr
		(char const * const name, char const * const descname,
		 std::string const & directory, Profile &,  Section & global_s,
		 Tribe_Descr const &, EncodeData const *);
	virtual ~Worker_Descr();

	virtual Bob & create_object() const;

	virtual void load_graphics();

	bool              buildable() const throw () {return buildcost().size();}
	Buildcost const & buildcost() const throw () {return m_buildcost;}

	const Tribe_Descr * get_tribe() const throw () {return m_owner_tribe;}
	const Tribe_Descr & tribe() const throw () {return *m_owner_tribe;}
	std::string helptext() const {return m_helptext;}

	uint32_t icon() const throw () {return m_icon;}
	const DirAnimations & get_walk_anims() const throw () {return m_walk_anims;}
	const DirAnimations & get_right_walk_anims(const bool carries_ware)
		const throw ()
	{return carries_ware ? m_walkload_anims : m_walk_anims;}
	WorkerProgram const * get_program(std::string const &) const;

	virtual Worker_Type get_worker_type() const {return NORMAL;}

	// For leveling
	int32_t get_max_exp() const throw () {return m_max_experience;}
	int32_t get_min_exp() const throw () {return m_min_experience;}
	Ware_Index becomes() const throw () {return m_becomes;}
	Ware_Index worker_index() const throw ();
	bool can_act_as(Ware_Index) const;

	Worker & create
		(Editor_Game_Base &, Player &, PlayerImmovable *, Coords) const;

	typedef std::map<Worker_Descr *, std::string> becomes_map_t;
	virtual uint32_t movecaps() const throw ();

	typedef std::map<std::string, WorkerProgram *> Programs;
	Programs const & programs() const throw () {return m_programs;}

protected:
#ifdef WRITE_GAME_DATA_AS_HTML
	void writeHTML(::FileWrite &) const;
#endif

	std::string   m_helptext;       ///< Short (tooltip-like) help text
	std::string const m_icon_fname; ///< Filename of worker's icon
	uint32_t          m_icon;       ///< Pointer to icon into picture stack
	DirAnimations m_walk_anims;
	DirAnimations m_walkload_anims;
	Buildcost     m_buildcost;
	int32_t           m_max_experience;
	int32_t           m_min_experience;
	Ware_Index    m_becomes;       /// Type that this type can become (or Null).
	Programs    m_programs;
};

};

#endif
