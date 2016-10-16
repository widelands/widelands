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

#ifndef WL_LOGIC_MAP_OBJECTS_IMMOVABLE_H
#define WL_LOGIC_MAP_OBJECTS_IMMOVABLE_H

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "graphic/animation.h"
#include "logic/map_objects/buildcost.h"
#include "logic/map_objects/map_object.h"
#include "logic/widelands_geometry.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"

class LuaTable;
class TribesLegacyLookupTable;
class WorldLegacyLookupTable;

namespace Widelands {

class Economy;
class Map;
class TerrainAffinity;
class Tribes;
class WareInstance;
class Worker;
class World;
struct Flag;
struct PlayerImmovable;
class TribeDescr;

struct NoteImmovable {
	CAN_BE_SENT_AS_NOTE(NoteId::Immovable)

	PlayerImmovable* pi;

	enum class Ownership { LOST, GAINED };
	Ownership ownership;

	NoteImmovable(PlayerImmovable* const init_pi, Ownership const init_ownership)
	   : pi(init_pi), ownership(init_ownership) {
	}
};

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
struct BaseImmovable : public MapObject {
	enum Size {
		NONE = 0,  ///< not robust (i.e. removable by building something over it)
		SMALL,     ///< small building or robust map element, including trees
		MEDIUM,    ///< medium size building
		BIG        ///< big building
	};

	BaseImmovable(const MapObjectDescr&);

	virtual int32_t get_size() const = 0;
	virtual bool get_passable() const = 0;

	using PositionList = std::vector<Coords>;
	/**
	 * Return all coordinates occupied by this Immovable. We gurantee that the
	 * list always contains one entry and the first one is the main position
	 * if one can be chosen as main.
	 */
	virtual PositionList get_positions(const EditorGameBase&) const = 0;

	// Draw this immovable onto 'dst' choosing the frame appropriate for
	// 'gametime'. 'show_text' decides if census and statistics are written too.
	// The 'coords_to_draw' are passed one to give objects that occupy multiple
	// fields a way to only draw themselves once. The 'point_on_dst' determines
	// the point for the hotspot of the animation and 'zoom' determines how big
	// the immovable will be plotted.
	enum ShowText {
		kNone = 0,
		kCensus = 1,
		kStatistics = 2,
	};
	virtual void draw(uint32_t gametime,
	          ShowText show_text,
	          const Coords& coords_to_draw,
	          const Vector2f& point_on_dst,
				 float zoom,
	          RenderTarget* dst) = 0;

	static int32_t string_to_size(const std::string& size);
	static std::string size_to_string(int32_t size);

protected:
	void set_position(EditorGameBase&, const Coords&);
	void unset_position(EditorGameBase&, const Coords&);
};

class Immovable;
struct ImmovableProgram;
struct ImmovableAction;
struct ImmovableActionData;

/**
 * Immovable represents a standard immovable such as trees or rocks.
 */
class ImmovableDescr : public MapObjectDescr {
public:
	using Programs = std::map<std::string, ImmovableProgram*>;

	/// World immovable
	ImmovableDescr(const std::string& init_descname, const LuaTable&, const World& world);
	/// Tribes immovable
	ImmovableDescr(const std::string& init_descname, const LuaTable&, const Tribes& tribes);
	~ImmovableDescr() override;

	int32_t get_size() const {
		return size_;
	}
	ImmovableProgram const* get_program(const std::string&) const;

	Immovable& create(EditorGameBase&, const Coords&) const;

	MapObjectDescr::OwnerType owner_type() const {
		return owner_type_;
	}

	const Buildcost& buildcost() const {
		return buildcost_;
	}

	// Returns the editor category, or nullptr if the immovable has no editor category
	// (e.g. Tribe immovables never have one).
	const EditorCategory* editor_category() const;

	// A basic localized name for the immovable, used by trees
	const std::string& species() const {
		return species_;
	}

	// Every immovable that can 'grow' needs to have terrain affinity defined,
	// all others do not. Returns true if this one has it defined.
	bool has_terrain_affinity() const;

	// Returns the terrain affinity. If !has_terrain_affinity() this will return
	// an undefined value.
	const TerrainAffinity& terrain_affinity() const;

protected:
	int32_t size_;
	Programs programs_;

	/// Whether this ImmovableDescr belongs to a tribe or the world
	const MapObjectDescr::OwnerType owner_type_;

	/// Buildcost for externally constructible immovables (for ship construction)
	/// \see ActConstruction
	Buildcost buildcost_;

	std::string species_;

private:
	// Common constructor functions for tribes and world.
	ImmovableDescr(const std::string& init_descname,
	               const LuaTable&,
	               MapObjectDescr::OwnerType type);

	// Adds a default program if none was defined.
	void make_sure_default_program_is_there();

	EditorCategory* editor_category_;  // not owned.
	std::unique_ptr<TerrainAffinity> terrain_affinity_;
	DISALLOW_COPY_AND_ASSIGN(ImmovableDescr);
};

class Immovable : public BaseImmovable {
	friend class ImmovableDescr;
	friend struct ImmovableProgram;
	friend class Map;

	MO_DESCR(ImmovableDescr)

public:
	Immovable(const ImmovableDescr&);
	~Immovable();

	Player* get_owner() const {
		return owner_;
	}
	void set_owner(Player* player);

	Coords get_position() const {
		return position_;
	}
	PositionList get_positions(const EditorGameBase&) const override;

	int32_t get_size() const override;
	bool get_passable() const override;
	void start_animation(const EditorGameBase&, uint32_t anim);

	void program_step(Game& game, uint32_t const delay = 1) {
		if (delay)
			program_step_ = schedule_act(game, delay);
		increment_program_pointer();
	}

	void init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void act(Game&, uint32_t data) override;
	void draw(uint32_t gametime,
	          ShowText show_text,
	          const Coords& coords_to_draw,
	          const Vector2f& point_on_dst,
	          float zoom,
	          RenderTarget* dst) override;

	void switch_program(Game& game, const std::string& programname);
	bool construct_ware(Game& game, DescriptionIndex index);
	bool construct_remaining_buildcost(Game& game, Buildcost* buildcost);

	void set_action_data(ImmovableActionData* data);
	template <typename T> T* get_action_data() {
		if (!action_data_)
			return nullptr;
		if (T* data = dynamic_cast<T*>(action_data_.get()))
			return data;
		set_action_data(nullptr);
		return nullptr;
	}

protected:
	Player* owner_;
	Coords position_;

	uint32_t anim_;
	int32_t animstart_;

	const ImmovableProgram* program_;
	uint32_t program_ptr_;  ///< index of next instruction to execute

/* GCC 4.0 has problems with friend declarations: It doesn't allow
 * substructures of friend classes private access but we rely on this behaviour
 * for ImmovableProgram::ActConstruction. As a dirty workaround, we make the
 * following variables public for this versions but keep the protected for
 * other GCC versions.
 * See the related bug lp:688832.
 */
#if (__GNUC__ == 4) && (__GNUC_MINOR__ == 0)
public:
	uint32_t anim_construction_total_;
	uint32_t anim_construction_done_;
	uint32_t program_step_;

protected:
#else
	uint32_t anim_construction_total_;
	uint32_t anim_construction_done_;
	uint32_t program_step_;  ///< time of next step
#endif

	/**
	 * Private persistent data for the currently active program action.
	 *
	 * \warning Use get_action_data to access this.
	 */
	std::unique_ptr<ImmovableActionData> action_data_;

	// Load/save support
protected:
	struct Loader : public BaseImmovable::Loader {
		void load(FileRead&, uint8_t packet_version);
		void load_pointers() override;
		void load_finish() override;
	};

public:
	// TODO(unknown): Remove as soon as we fully support the new system
	bool has_new_save_support() override {
		return true;
	}

	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
	static MapObject::Loader* load(EditorGameBase&,
	                               MapObjectLoader&,
	                               FileRead&,
	                               const WorldLegacyLookupTable& world_lookup_table,
	                               const TribesLegacyLookupTable& tribes_lookup_table);

private:
	void increment_program_pointer();
	void draw_construction(uint32_t gametime,
	                       ShowText show_text,
	                       const Vector2f& point_on_dst,
	                       float zoom,
	                       RenderTarget* dst);
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
	PlayerImmovable(const MapObjectDescr&);
	virtual ~PlayerImmovable();

	Player* get_owner() const {
		return owner_;
	}
	Player& owner() const {
		return *owner_;
	}
	Economy* get_economy() const {
		return economy_;
	}
	Economy& economy() const {
		return *economy_;
	}

	virtual Flag& base_flag() = 0;

	virtual void set_economy(Economy*);

	virtual void add_worker(Worker&);
	virtual void remove_worker(Worker&);

	using Workers = std::vector<Worker*>;

	/**
	 * \return a list of workers that are currently located at this
	 * immovable. This is not the same as the list of production
	 * workers returned by \ref ProductionSite::get_production_workers
	 */
	const Workers& get_workers() const {
		return workers_;
	}

	void log_general_info(const EditorGameBase&) override;

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
	virtual void receive_ware(Game&, DescriptionIndex ware);
	virtual void receive_worker(Game&, Worker& worker);
	/*@}*/

	void set_owner(Player*);

protected:
	void init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

private:
	Player* owner_;
	Economy* economy_;

	Workers workers_;

	// load/save support
protected:
	struct Loader : BaseImmovable::Loader {
		Loader();

		void load(FileRead&);
	};

public:
	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_IMMOVABLE_H
