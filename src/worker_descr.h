/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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


/// \todo (Antonio Trueba#1#): Get rid of forward class declaration (chicked-and-egg problem)
class Worker;
class WorkerProgram;


class Worker_Descr : public Bob::Descr
{
	friend class Tribe_Descr;
	friend class Warehouse;
	friend class WorkerProgram;

	typedef std::map<std::string, WorkerProgram*> ProgramMap;

	struct CostItem {
		std::string name;   // name of ware
		int         amount; // amount

		inline CostItem(const char* iname, int iamount): name   (iname),
		                                                 amount (iamount)
		                                                 {}
	};

	typedef std::vector<CostItem> BuildCost;

public:
		enum Worker_Type {
			NORMAL = 0,
			CARRIER,
			SOLDIER,
		};

		Worker_Descr(const Tribe_Descr &, const std::string & name);
		virtual ~Worker_Descr();

		virtual Bob * create_object() const;

		virtual void load_graphics();

		inline bool get_buildable() {return m_buildable;}
		inline const BuildCost* get_buildcost() {return &m_buildcost;}

		const Tribe_Descr * get_tribe() const throw () {return m_owner_tribe;}
		const Tribe_Descr & tribe() const throw () {return *m_owner_tribe;}
		const std::string & descname() const throw () {return m_descname;}
		inline std::string get_helptext() const {return m_helptext;}

		uint get_menu_pic() const throw () {return m_menu_pic;}
		const DirAnimations & get_walk_anims() const throw ()
				{return m_walk_anims;}
		const DirAnimations & get_right_walk_anims(const bool carries_ware)
				const throw ()
				{return carries_ware ? m_walkload_anims : m_walk_anims;}
		const WorkerProgram* get_program(std::string programname) const;

		virtual Worker_Type get_worker_type() const {return NORMAL;}

		// For leveling
		int get_max_exp() const throw () {return m_max_experience;}
		int get_min_exp() const throw () {return m_min_experience;}
		const char * get_becomes() const throw () {return m_becomes.c_str();}
		int get_becomes_index() const throw ();
		bool can_act_as(int ware) const;

		Worker *create(Editor_Game_Base *g, Player *owner,
		               PlayerImmovable *location, Coords coords);


protected:
		virtual void parse(const char *directory, Profile *prof,
		                   const EncodeData *encdata);
		static Worker_Descr * create_from_dir(const Tribe_Descr &,
		                                      const char * const directory,
		                                      const EncodeData * const);

		std::string   m_descname;       ///< Descriptive name
		std::string   m_helptext;       ///< Short (tooltip-like) help text
		char        * m_menu_pic_fname; ///< Filename of worker's icon
		uint          m_menu_pic;       ///< Pointer to icon into picture stack
		DirAnimations m_walk_anims;
		DirAnimations m_walkload_anims;
		bool          m_buildable;
		BuildCost     m_buildcost;      ///< What and how much we need to build this worker
		int           m_max_experience;
		int           m_min_experience;
		std::string   m_becomes;        ///< Workername this worker evolves to, if any
		mutable int   m_becomes_index;  ///< index in tribe array if any (cached)
		ProgramMap    m_programs;
};


#endif // WORKER_DESCR_H
