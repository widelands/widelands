/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include <atomic>
#include <memory>

#include "base/macros.h"
#include "logic/map_objects/buildcost.h"
#include "logic/map_objects/info_to_draw.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/widelands_geometry.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"

namespace Widelands {

class Building;
class BuildingDescr;
class Economy;
class Immovable;
class Map;
class TerrainAffinity;
class Worker;
struct Flag;
struct ImmovableAction;
struct ImmovableActionData;
struct ImmovableProgram;
struct PlayerImmovable;

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

	explicit BaseImmovable(const MapObjectDescr&);

	virtual int32_t get_size() const = 0;
	virtual bool get_passable() const = 0;

	virtual void set_owner(Player* player);

	using PositionList = std::vector<Coords>;
	/**
	 * Return all coordinates occupied by this Immovable. We gurantee that the
	 * list always contains one entry and the first one is the main position
	 * if one can be chosen as main.
	 */
	virtual PositionList get_positions(const EditorGameBase&) const = 0;

	// Draw this immovable onto 'dst' choosing the frame appropriate for
	// 'gametime'. 'info_to_draw' decides if census and statistics are written too.
	// The 'coords_to_draw' are passed one to give objects that occupy multiple
	// fields a way to only draw themselves once. The 'point_on_dst' determines
	// the point for the hotspot of the animation and 'scale' determines how big
	// the immovable will be plotted.
	virtual void draw(const Time& gametime,
	                  InfoToDraw info_to_draw,
	                  const Vector2f& point_on_dst,
	                  const Coords& coords,
	                  float scale,
	                  RenderTarget* dst) = 0;

	static int32_t string_to_size(const std::string& size);
	static std::string size_to_string(int32_t size);

protected:
	void set_position(EditorGameBase&, const Coords&);
	void unset_position(EditorGameBase&, const Coords&);
};

/**
 * Immovable represents a standard immovable such as trees or rocks.
 */
class ImmovableDescr : public MapObjectDescr {
	friend struct ImmovableProgram;

public:
	using Programs = std::map<std::string, ImmovableProgram*>;

	/// Common constructor for tribes and world.
	ImmovableDescr(const std::string& init_descname,
	               const LuaTable&,
	               const std::vector<std::string>& attribs,
	               Descriptions& descriptions);
	~ImmovableDescr() override;

	int32_t get_size() const {
		return size_;
	}
	ImmovableProgram const* get_program(const std::string&) const;

	Immovable& create(EditorGameBase&,
	                  const Coords&,
	                  const Widelands::BuildingDescr* former_building_descr) const;

	const Buildcost& buildcost() const {
		return buildcost_;
	}

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

	// Map object names that the immovable can transform/grow into
	const std::set<std::pair<MapObjectType, std::string>>& becomes() const {
		return becomes_;
	}

	// A set of all productionsites that gather this immovable or any of its future types
	const std::set<std::string> collected_by() const {
		return collected_by_;
	}
	void add_collected_by(const Descriptions& descriptions, const std::string& prodsite);

	void register_immovable_relation(const std::string&, const std::string&);
	void add_became_from(const std::string& s) {
		became_from_.insert(s);
	}

protected:
	Descriptions& descriptions_;
	int32_t size_;
	Programs programs_;

	/// Buildcost for externally constructible immovables (for ship construction)
	/// \see ActConstruct
	Buildcost buildcost_;

	std::string species_;
	std::set<std::pair<MapObjectType, std::string>> becomes_;
	std::set<std::string> became_from_;  // immovables that turn into this one
	std::set<std::string> collected_by_;

private:
	// Adds a default program if none was defined.
	void make_sure_default_program_is_there();

	std::unique_ptr<TerrainAffinity> terrain_affinity_;
	DISALLOW_COPY_AND_ASSIGN(ImmovableDescr);
};

class Immovable : public BaseImmovable {
	friend class ImmovableDescr;
	friend struct ImmovableProgram;
	friend class Map;

	MO_DESCR(ImmovableDescr)

public:
	/// If this immovable was created by a building, 'former_building_descr' can be set in order to
	/// display information about it.
	explicit Immovable(const ImmovableDescr&,
	                   const Widelands::BuildingDescr* former_building_descr = nullptr);
	~Immovable() override = default;

	Coords get_position() const {
		return position_;
	}
	PositionList get_positions(const EditorGameBase&) const override;

	int32_t get_size() const override;
	bool get_passable() const override;
	void start_animation(const EditorGameBase&, uint32_t anim);

	void program_step(Game& game, const Duration& delay = Duration(1)) {
		assert(delay.is_valid());
		if (delay.get() > 0) {
			program_step_ = schedule_act(game, delay);
		}
		increment_program_pointer();
	}

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void act(Game&, uint32_t data) override;
	void draw(const Time& gametime,
	          InfoToDraw info_to_draw,
	          const Vector2f& point_on_dst,
	          const Coords& coords,
	          float scale,
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

	void delay_growth(Duration ms) {
		growth_delay_ += ms;
	}
	bool apply_growth_delay(Game&);

	bool is_marked_for_removal(PlayerNumber) const;
	void set_marked_for_removal(PlayerNumber, bool mark);
	const std::set<PlayerNumber>& get_marked_for_removal() const {
		return marked_for_removal_;
	}

protected:
	// The building type that created this immovable, if any.
	const BuildingDescr* former_building_descr_;

	Coords position_;

	std::atomic<uint32_t> anim_;
	Time animstart_;

	const ImmovableProgram* program_;
	uint32_t program_ptr_;  ///< index of next instruction to execute

	// Whether a worker was told to remove this object ASAP.
	// A set of all players who want this object gone.
	std::set<PlayerNumber> marked_for_removal_;

/* GCC 4.0 has problems with friend declarations: It doesn't allow
 * substructures of friend classes private access but we rely on this behaviour
 * for ImmovableProgram::ActConstruct. As a dirty workaround, we make the
 * following variables public for this versions but keep the protected for
 * other GCC versions.
 * See the related bug lp:688832.
 */
#if (__GNUC__ == 4) && (__GNUC_MINOR__ == 0)
public:
	uint32_t anim_construction_total_;
	uint32_t anim_construction_done_;
	Time program_step_;

protected:
#else
	std::atomic<uint32_t> anim_construction_total_;
	std::atomic<uint32_t> anim_construction_done_;
	Time program_step_;  ///< time of next step
#endif

	/**
	 * Private persistent data for the currently active program action.
	 *
	 * \warning Use get_action_data to access this.
	 */
	std::unique_ptr<ImmovableActionData> action_data_;

private:
	Duration growth_delay_;

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
	static MapObject::Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);

private:
	/// If this immovable was created by a building, this can be set in order to display information
	/// about it. If this is a player immovable, you will need to set the owner first.
	void set_former_building(const BuildingDescr& building);

	void increment_program_pointer();
	void draw_construction(const Time& gametime,
	                       InfoToDraw info_to_draw,
	                       const Vector2f& point_on_dst,
	                       const Widelands::Coords& coords,
	                       float scale,
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
	explicit PlayerImmovable(const MapObjectDescr&);
	~PlayerImmovable() override;

	Economy* get_economy(WareWorker type) const {
		return type == wwWARE ? ware_economy_ : worker_economy_;
	}
	Economy& economy(WareWorker type) const {
		return *(type == wwWARE ? ware_economy_ : worker_economy_);
	}

	virtual Flag& base_flag() = 0;

	virtual void set_economy(Economy*, WareWorker);

	virtual void add_worker(Worker&);
	virtual void remove_worker(Worker&);

	using Workers = std::vector<Worker*>;

	/**
	 * \return a list of workers that are currently located at this
	 * immovable. This is not the same as the list of production
	 * workers returned by \ref ProductionSite::working_positions
	 */
	const Workers& get_workers() const {
		return workers_;
	}

	void log_general_info(const EditorGameBase&) const override;

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

	void set_owner(Player*) override;

protected:
	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

private:
	Economy* ware_economy_;
	Economy* worker_economy_;

	Workers workers_;

	// load/save support
protected:
	struct Loader : BaseImmovable::Loader {
		Loader() = default;

		void load(FileRead&);
	};

public:
	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_IMMOVABLE_H
