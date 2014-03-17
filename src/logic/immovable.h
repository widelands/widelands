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

#ifndef IMMOVABLE_H
#define IMMOVABLE_H

#include "graphic/animation.h"
#include "logic/buildcost.h"
#include "logic/instances.h"
#include "logic/widelands_geometry.h"

struct Profile;

namespace Widelands {

class Economy;
struct Flag;
class Map;
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
		NONE = 0, ///< not robust (i.e. removable by building something over it)
		SMALL,    ///< small building or robust map element, including trees
		MEDIUM,   ///< medium size building
		BIG       ///< big building
	};

	BaseImmovable(const Map_Object_Descr &);

	virtual int32_t  get_size    () const = 0;
	virtual bool get_passable() const = 0;

	typedef std::vector<Coords> PositionList;
	/**
	 * Return all coordinates occupied by this Immovable. We gurantee that the
	 * list always contains one entry and the first one is the main position
	 * if one can be chosen as main.
	 */
	virtual PositionList get_positions
		(const Editor_Game_Base &) const = 0;
	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords&, const Point&)
		= 0;
	virtual const std::string & name() const;

protected:
	void set_position(Editor_Game_Base &, Coords);
	void unset_position(Editor_Game_Base &, Coords);
};


class Immovable;
struct ImmovableProgram;
struct ImmovableAction;
struct ImmovableActionData;

/**
 * Immovable represents a standard immovable such as trees or stones.
 */
struct Immovable_Descr : public Map_Object_Descr {
	typedef std::map<std::string, ImmovableProgram *> Programs;

	Immovable_Descr
		(char const * name, char const * descname,
		 const std::string & directory, Profile &, Section & global_s,
		 const World & world, Tribe_Descr const * const);
	~Immovable_Descr();

	int32_t get_size() const {return m_size;}
	ImmovableProgram const * get_program(const std::string &) const;

	Immovable & create(Editor_Game_Base &, Coords) const;

	Tribe_Descr const * get_owner_tribe() const {return m_owner_tribe;}

	/// How well the terrain around f suits an immovable of this type.
	uint32_t terrain_suitability(FCoords, const Map &) const;

	const Buildcost & buildcost() const {return m_buildcost;}

protected:
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
	friend struct ImmovableProgram;
	friend class Map;

	MO_DESCR(Immovable_Descr);

public:
	Immovable(const Immovable_Descr &);
	~Immovable();

	Player * get_owner() const {return m_owner;}
	void set_owner(Player * player);

	Coords get_position() const {return m_position;}
	virtual PositionList get_positions (const Editor_Game_Base &) const override;

	virtual int32_t  get_type    () const override;
	char const * type_name() const override {return "immovable";}
	virtual int32_t  get_size    () const override;
	virtual bool get_passable() const override;
	const std::string & name() const override;
	void start_animation(const Editor_Game_Base &, uint32_t anim);

	void program_step(Game & game, uint32_t const delay = 1) {
		if (delay)
			m_program_step = schedule_act(game, delay);
		increment_program_pointer();
	}

	void init(Editor_Game_Base &) override;
	void cleanup(Editor_Game_Base &) override;
	void act(Game &, uint32_t data) override;

	virtual void draw(const Editor_Game_Base &, RenderTarget &, const FCoords&, const Point&) override;

	void switch_program(Game & game, const std::string & programname);
	bool construct_ware(Game & game, Ware_Index index);
	bool construct_remaining_buildcost(Game & game, Buildcost * buildcost);

	Tribe_Descr const * get_owner_tribe() const {
		return descr().get_owner_tribe();
	}

	bool is_reserved_by_worker() const;
	void set_reserved_by_worker(bool reserve);

	void set_action_data(ImmovableActionData * data);
	template<typename T>
	T * get_action_data() {
		if (!m_action_data)
			return nullptr;
		if (T * data = dynamic_cast<T *>(m_action_data))
			return data;
		set_action_data(nullptr);
		return nullptr;
	}

protected:
	Player * m_owner;
	Coords                   m_position;

	uint32_t                     m_anim;
	int32_t                      m_animstart;

	const ImmovableProgram * m_program;
	uint32_t m_program_ptr; ///< index of next instruction to execute

/* GCC 4.0 has problems with friend declarations: It doesn't allow
 * substructures of friend classes private access but we rely on this behaviour
 * for ImmovableProgram::ActConstruction. As a dirty workaround, we make the
 * following variables public for this versions but keep the protected for
 * other GCC versions.
 * See the related bug lp:688832.
 */
#if (__GNUC__ == 4) && (__GNUC_MINOR__ == 0)
public:
	uint32_t m_anim_construction_total;
	uint32_t m_anim_construction_done;
	int32_t m_program_step;
protected:
#else
	uint32_t m_anim_construction_total;
	uint32_t m_anim_construction_done;
	int32_t m_program_step; ///< time of next step
#endif
	std::string m_construct_string;

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
		virtual void load_pointers() override;
		virtual void load_finish() override;
	};

public:
	/// \todo Remove as soon as we fully support the new system
	virtual bool has_new_save_support() override {return true;}

	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;
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
	Economy * get_economy() const {return m_economy;}
	Economy & economy() const {return *m_economy;}

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
	const Workers & get_workers() const {return m_workers;}

	virtual void log_general_info(const Editor_Game_Base &) override;

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

	void set_owner(Player *);

protected:
	virtual void init   (Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;

private:
	Player              * m_owner;
	Economy             * m_economy;

	Workers   m_workers;

	// load/save support
protected:
	struct Loader : BaseImmovable::Loader {
		Loader();

		void load(FileRead &);
	};

public:
	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;
};

}

#endif
