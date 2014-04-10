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

#ifndef WORKER_DESCR_H
#define WORKER_DESCR_H

#include "logic/bob.h"
#include "graphic/diranimations.h"
#include "logic/immovable.h"
#include "io/filewrite.h"

class Image;

namespace Widelands {

/// \todo (Antonio Trueba#1#): Get rid of forward class declaration
/// (chicked-and-egg problem)
class Worker;
struct WorkerProgram;


class Worker_Descr : public BobDescr
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
		 const std::string & directory, Profile &,  Section & global_s,
		 const Tribe_Descr &);
	virtual ~Worker_Descr();

	virtual Bob & create_object() const override;

	virtual void load_graphics();

	bool is_buildable() const {return m_buildable;}
	const Buildcost & buildcost() const {
		assert(is_buildable());
		return m_buildcost;
	}

	const Tribe_Descr * get_tribe() const {return m_owner_tribe;}
	const Tribe_Descr & tribe() const {return *m_owner_tribe;}
	std::string helptext() const {return m_helptext;}
	Point get_ware_hotspot() const {return m_ware_hotspot;}

	/// How much of the worker type that an economy should store in warehouses.
	/// The special value std::numeric_limits<uint32_t>::max() means that the
	/// the target quantity of this ware type will never be checked and should
	/// not be configurable.
	uint32_t default_target_quantity() const {return m_default_target_quantity;}

	bool has_demand_check() const {
		return default_target_quantity() != std::numeric_limits<uint32_t>::max();
	}

	/// Called when a demand check for this ware type is encountered during
	/// parsing. If there was no default target quantity set in the ware type's
	/// configuration, set the default value 1.
	void set_has_demand_check() {
		if (m_default_target_quantity == std::numeric_limits<uint32_t>::max())
			m_default_target_quantity = 1;
	}

	const Image* icon() const {return m_icon;}
	const DirAnimations & get_walk_anims() const {return m_walk_anims;}
	const DirAnimations & get_right_walk_anims(bool const carries_ware) const {
		return carries_ware ? m_walkload_anims : m_walk_anims;
	}
	WorkerProgram const * get_program(const std::string &) const;

	virtual Worker_Type get_worker_type() const {return NORMAL;}

	// For leveling
	int32_t get_level_experience() const {return m_level_experience;}
	Ware_Index becomes() const {return m_becomes;}
	Ware_Index worker_index() const;
	bool can_act_as(Ware_Index) const;

	Worker & create
		(Editor_Game_Base &, Player &, PlayerImmovable *, Coords) const;

	typedef std::map<Worker_Descr const *, std::string> becomes_map_t;
	virtual uint32_t movecaps() const override;

	typedef std::map<std::string, WorkerProgram *> Programs;
	const Programs & programs() const {return m_programs;}

protected:

	std::string       m_helptext;   ///< Short (tooltip-like) help text
	Point             m_ware_hotspot;
	uint32_t          m_default_target_quantity;
	std::string const m_icon_fname; ///< Filename of worker's icon
	const Image     * m_icon;       ///< Pointer to icon into picture stack
	DirAnimations     m_walk_anims;
	DirAnimations     m_walkload_anims;
	bool              m_buildable;
	Buildcost         m_buildcost;

	/**
	 * Number of experience points required for leveling up,
	 * or -1 if the worker cannot level up.
	 */
	int32_t m_level_experience;

	/**
	 * Type that this worker can become, i.e. level up to (or Null).
	 */
	Ware_Index  m_becomes;
	Programs    m_programs;
};

}

#endif
