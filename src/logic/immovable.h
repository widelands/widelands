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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef IMMOVABLE_H
#define IMMOVABLE_H

#include "buildcost.h"
#include "graphic/animation.h"
#include "instances.h"
#include "widelands_geometry.h"

struct Profile;

namespace Widelands {

struct Economy;
struct Flag;
struct Map;
struct Tribe_Descr;
class WareInstance;
class Worker;

/**
 * BaseImmovable is the base for all non-moving objects (immovables such as
 * trees, buildings, flags, roads).
 *
 * The Immovable's size influences building capabilities around it.
 * If size is NONE, the immovable can simply be removed by placing something on
 * it (this is usually true for decorations).
 *
 * For more information, see the Map::recalc_* functions.
 */
struct BaseImmovable : public Map_Object {
	enum {
		NONE,   ///< not robust (i.e. removable by building something over it)
		SMALL,  ///< small building or robust map element, including trees
		MEDIUM, ///< medium size building
		BIG     ///< big building
	};

	BaseImmovable(const Map_Object_Descr &);

	virtual int32_t  get_size    () const throw () = 0;
	virtual bool get_passable() const throw () = 0;

	typedef std::vector<Coords> PositionList;
	/**
	 * Return all coordinates occupied by this Immovable. We gurantee that the
	 * list always contains one entry and the first one is the main position
	 * if one can be chosen as main.
	 */
	virtual PositionList get_positions
		(const Editor_Game_Base &) const throw () = 0;
	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point)
		= 0;
	virtual std::string const & name() const throw ();

protected:
	void set_position(Editor_Game_Base &, Coords);
	void unset_position(Editor_Game_Base &, Coords);
};


class Immovable;
class ImmovableProgram;
struct ImmovableAction;
struct ImmovableActionData;

/**
 * Immovable represents a standard immovable such as trees or stones.
 */
struct Immovable_Descr : public Map_Object_Descr {
	friend struct Map_Immovabledata_Data_Packet; // For writing (get_program)

	typedef std::map<std::string, ImmovableProgram *> Programs;

	Immovable_Descr
		(char const * name, char const * descname,
		 std::string const & directory, Profile &, Section & global_s,
		 World const & world, Tribe_Descr const * const);
	~Immovable_Descr();

	int32_t get_size() const throw () {return m_size;}
	char const * get_picture() const {return m_picture.c_str();}
	ImmovableProgram const * get_program(std::string const &) const;

	Immovable & create(Editor_Game_Base &, Coords) const;

	Tribe_Descr const * get_owner_tribe() const throw () {return m_owner_tribe;}

	/// How well the terrain around f suits an immovable of this type.
	uint32_t terrain_suitability(FCoords, Map const &) const;

	const Buildcost & buildcost() const {return m_buildcost;}

protected:
	std::string m_picture;
	int32_t     m_size;

	Programs    m_programs;

	/// The tribe to which this Immovable_Descr belongs or 0 if it is a
	/// world immovable
	const Tribe_Descr * const m_owner_tribe;

	/// Buildcost for externally constructible immovables (for ship construction)
	/// \see ActConstruction
	Buildcost m_buildcost;

private:
	uint8_t m_terrain_affinity[16];
};

class Immovable : public BaseImmovable {
	friend struct Immovable_Descr;
	friend class ImmovableProgram;
	friend struct Map;

	// for writing (obsolete since build-11)
	friend struct Map_Immovabledata_Data_Packet;

	MO_DESCR(Immovable_Descr);

public:
	Immovable(const Immovable_Descr &);
	~Immovable();

	Coords get_position() const {return m_position;}
	virtual PositionList get_positions (const Editor_Game_Base &) const throw ();

	virtual int32_t  get_type    () const throw ();
	char const * type_name() const throw () {return "immovable";}
	virtual int32_t  get_size    () const throw ();
	virtual bool get_passable() const throw ();
	std::string const & name() const throw ();
	void start_animation(Editor_Game_Base const &, uint32_t anim);

	void program_step(Game & game, uint32_t const delay = 1) {
		if (delay)
			m_program_step = schedule_act(game, delay);
		increment_program_pointer();
	}

	void init(Editor_Game_Base &);
	void cleanup(Editor_Game_Base &);
	void act(Game &, uint32_t data);

	virtual void draw(Editor_Game_Base const &, RenderTarget &, FCoords, Point);

	void switch_program(Game & game, std::string const & programname);
	bool construct_ware_item(Game & game, Ware_Index index);
	bool construct_remaining_buildcost(Game & game, Buildcost * buildcost);

	Tribe_Descr const * get_owner_tribe() const {
		return descr().get_owner_tribe();
	}

	bool is_reserved_by_worker() const;
	void set_reserved_by_worker(bool reserve);

	void set_action_data(ImmovableActionData * data);
	template<typename T>
	T* get_action_data() {
		if (!m_action_data)
			return 0;
		if (T* data = dynamic_cast<T*>(m_action_data))
			return data;
		set_action_data(0);
		return 0;
	}

protected:
	Coords                   m_position;

	uint32_t                     m_anim;
	int32_t                      m_animstart;
	uint32_t m_anim_construction_total;
	uint32_t m_anim_construction_done;

	const ImmovableProgram * m_program;
	uint32_t m_program_ptr; ///< index of next instruction to execute
	int32_t                      m_program_step; ///< time of next step

	/**
	 * Private persistent data for the currently active program action.
	 *
	 * \warning Use get_action_data to access this.
	 */
	ImmovableActionData * m_action_data;

	/**
	 * Immovables like trees are reserved by a worker that is walking
	 * towards them, so that e.g. two lumberjacks don't attempt to
	 * work on the same tree simultaneously.
	 */
	bool m_reserved_by_worker;

	// Load/save support
protected:
	struct Loader : public BaseImmovable::Loader {
		void load(FileRead &, uint8_t version);
		virtual void load_pointers();
		virtual void load_finish();
	};

public:
	/// \todo Remove as soon as we fully support the new system
	virtual bool has_new_save_support() {return true;}

	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &);
	static Map_Object::Loader * load
		(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);

private:
	void increment_program_pointer();

	void draw_construction
		(const Editor_Game_Base &, RenderTarget &, const Point);
};


/**
 * PlayerImmovable is an immovable owned by a player that belongs to an economy:
 * building, flag or road
 *
 * A PlayerImmovable can also house a number of workers, which are automatically
 * turned into fugitives when the immovable is destroyed, and their economy is
 * also adjusted automatically.
 */
struct PlayerImmovable : public BaseImmovable {
	PlayerImmovable(const Map_Object_Descr &);
	virtual ~PlayerImmovable();

	Player * get_owner() const {return m_owner;}
	Player & owner() const {return *m_owner;}
	Economy * get_economy() const throw () {return m_economy;}
	Economy & economy() const throw () {return *m_economy;}

	virtual Flag & base_flag() = 0;

	virtual void set_economy(Economy *);

	virtual void    add_worker(Worker &);
	virtual void remove_worker(Worker &);

	typedef std::vector<Worker *> Workers;

	/**
	 * \return a list of workers that are currently located at this
	 * immovable. This is not the same as the list of production
	 * workers returned by \ref ProductionSite::get_production_workers
	 */
	Workers const & get_workers() const {return m_workers;}

	virtual void log_general_info(Editor_Game_Base const &);

	/**
	 * These functions are called when a ware or worker arrives at
	 * this immovable as the destination of a transfer that does not
	 * have an associated request.
	 *
	 * At the time of this writing, this happens only for warehouses.
	 *
	 * \note This is independent of the \ref add_worker / \ref remove_worker
	 * functionality, which has to do with setting up locations.
	 */
	/*@{*/
	virtual void receive_ware(Game &, Ware_Index ware);
	virtual void receive_worker(Game &, Worker & worker);
	/*@}*/

protected:
	void set_owner(Player *);

	virtual void init   (Editor_Game_Base &);
	virtual void cleanup(Editor_Game_Base &);

private:
	Player              * m_owner;
	Economy             * m_economy;

	Workers   m_workers;
};

}

#endif
