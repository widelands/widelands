/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_BUILDING_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_BUILDING_H

#include "ai/ai_hints.h"
#include "base/macros.h"
#include "logic/map_objects/buildcost.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/attack_target.h"
#include "logic/map_objects/tribes/building_settings.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/map_objects/tribes/workarea_info.h"
#include "logic/message.h"
#include "scripting/lua_table.h"

namespace Widelands {

class InputQueue;

constexpr int32_t kPriorityLow = 2;
constexpr int32_t kPriorityNormal = 4;
constexpr int32_t kPriorityHigh = 8;

constexpr float kBuildingSilhouetteOpacity = 0.3f;

/* The value "" means that the DescriptionIndex is a normal building, as happens e.g. when enhancing
 * a building. The value "tribe"/"world" means that the DescriptionIndex refers to an immovable of
 * OwnerType kTribe/kWorld, as happens e.g. with amazon treetop sentry. This immovable
 * should therefore always be painted below the building image.
 */
using FormerBuildings = std::vector<std::pair<DescriptionIndex, std::string>>;

/*
 * Common to all buildings!
 */
class BuildingDescr : public MapObjectDescr {
public:
	BuildingDescr(const std::string& init_descname,
	              MapObjectType type,
	              const LuaTable& t,
	              Tribes& tribes);
	~BuildingDescr() override {
	}

	bool is_buildable() const {
		return buildable_;
	}
	bool can_be_dismantled() const {
		return can_be_dismantled_;
	}
	bool is_destructible() const {
		return destructible_;
	}
	bool is_enhanced() const {
		return enhanced_building_;
	}

	/**
	 * The build cost for direct construction
	 */
	const Buildcost& buildcost() const {
		return buildcost_;
	}

	/**
	 * Returned wares for dismantling
	 */
	const Buildcost& returns_on_dismantle() const {
		return returns_on_dismantle_;
	}

	/**
	 * The build cost for enhancing a previous building
	 */
	const Buildcost& enhancement_cost() const {
		return enhancement_cost_;
	}

	/**
	 * The returned wares for a enhaced building
	 */
	const Buildcost& enhancement_returns_on_dismantle() const {
		return enhancement_returns_on_dismantle_;
	}

	int32_t get_size() const {
		return size_;
	}
	bool get_ismine() const {
		return mine_;
	}
	bool get_isport() const {
		return port_;
	}
	bool needs_seafaring() const {
		return needs_seafaring_;
	}
	bool needs_waterways() const {
		return needs_waterways_;
	}

	bool is_useful_on_map(bool seafaring_allowed, bool waterways_allowed) const;

	// Returns the enhancement this building can become or
	// INVALID_INDEX if it cannot be enhanced.
	const DescriptionIndex& enhancement() const {
		return enhancement_;
	}
	// Returns the building from which this building can be enhanced or
	// INVALID_INDEX if it cannot be built as an enhanced building.
	const DescriptionIndex& enhanced_from() const {
		return enhanced_from_;
	}
	void set_enhanced_from(const DescriptionIndex& index) {
		enhanced_from_ = index;
	}

	/// Create a building of this type in the game. Calls init, which does
	/// different things for different types of buildings (such as conquering
	/// land and requesting things). Therefore this must not be used to allocate
	/// a building during savegame loading. (It would cause many bugs.)
	///
	/// Does not perform any sanity checks.
	/// If former_buildings is not empty this is an enhancing.
	Building& create(EditorGameBase&,
	                 Player*,
	                 Coords,
	                 bool construct,
	                 bool loading = false,
	                 FormerBuildings former_buildings = FormerBuildings()) const;

	virtual uint32_t get_conquers() const;
	virtual uint32_t vision_range() const;

	const WorkareaInfo& workarea_info() const {
		return workarea_info_;
	}

	// TODO(sirver): This should not be public. It is mutated by other classes
	// in many places.
	WorkareaInfo workarea_info_;

	bool suitability(const Map&, const FCoords&) const;
	const AI::BuildingHints& hints() const;
	void set_hints_trainingsites_max_percent(int percent);

	uint32_t get_unoccupied_animation() const;

	DescriptionIndex get_built_over_immovable() const {
		return built_over_immovable_;
	}

protected:
	virtual Building& create_object() const = 0;
	Building& create_constructionsite() const;

private:
	void set_enhancement_cost(const Buildcost& enhance_cost, const Buildcost& return_enhanced);

	const Tribes& tribes_;
	const bool buildable_;     // the player can build this himself
	bool can_be_dismantled_;   // the player can dismantle this building
	const bool destructible_;  // the player can destruct this himself
	Buildcost buildcost_;
	Buildcost returns_on_dismantle_;  // Returned wares on dismantle
	Buildcost enhancement_cost_;      // cost for enhancing
	Buildcost
	   enhancement_returns_on_dismantle_;  // Returned ware for dismantling an enhanced building
	int32_t size_;                         // size of the building
	bool mine_;
	bool port_;
	bool needs_seafaring_;  // This building should only be built on seafaring maps.
	bool needs_waterways_;  // This building should only be built on maps with waterways/ferries
	                        // enabled
	DescriptionIndex enhancement_;
	DescriptionIndex
	   enhanced_from_;         // The building this building was enhanced from, or INVALID_INDEX
	bool enhanced_building_;   // if it is one, it is bulldozable
	AI::BuildingHints hints_;  // hints (knowledge) for computer players
	DescriptionIndex built_over_immovable_;  // can be built only on nodes where an immovable with
	                                         // this attribute stands

	// for migration, 0 is the default, meaning get_conquers() + 4
	uint32_t vision_range_;
	DISALLOW_COPY_AND_ASSIGN(BuildingDescr);
};

struct NoteBuilding {
	CAN_BE_SENT_AS_NOTE(NoteId::Building)

	Serial serial;

	enum class Action { kChanged, kStartWarp, kFinishWarp, kWorkersChanged };
	const Action action;

	NoteBuilding(Serial init_serial, const Action& init_action)
	   : serial(init_serial), action(init_action) {
	}
};

class Building : public PlayerImmovable {
	friend class BuildingDescr;
	friend class MapBuildingdataPacket;

	MO_DESCR(BuildingDescr)

public:
	// Player capabilities: which commands can a player issue for this building?
	enum {
		PCap_Bulldoze = 1,         // can bulldoze/remove this buildings
		PCap_Dismantle = 1 << 1,   // can dismantle this buildings
		PCap_Enhancable = 1 << 2,  // can be enhanced to something
	};

public:
	enum class InfoStringFormat { kCensus, kStatistics, kTooltip };

	explicit Building(const BuildingDescr&);

	void load_finish(EditorGameBase&) override;

	int32_t get_size() const override;
	bool get_passable() const override;

	Flag& base_flag() override;
	virtual uint32_t get_playercaps() const;

	virtual Coords get_position() const {
		return position_;
	}
	PositionList get_positions(const EditorGameBase&) const override;

	std::string info_string(const InfoStringFormat& format);

	// Return the overlay string that is displayed on the map view when enabled
	// by the player.
	const std::string& update_and_get_statistics_string() {
		update_statistics_string(&statistics_string_);
		return statistics_string_;
	}

	/// \returns the queue for the matching ware or worker type or \throws WException.
	virtual InputQueue& inputqueue(DescriptionIndex, WareWorker);

	virtual bool burn_on_destroy();
	void destroy(EditorGameBase&) override;

	bool is_destruction_blocked() const {
		return is_destruction_blocked_;
	}
	void set_destruction_blocked(bool b) {
		is_destruction_blocked_ = b;
	}

	virtual bool fetch_from_flag(Game&);
	virtual bool get_building_work(Game&, Worker&, bool success);

	bool leave_check_and_wait(Game&, Worker&);
	void leave_skip(Game&, Worker&);

	// Get/Set the priority for this waretype for this building. 'type' defines
	// if this is for a worker or a ware, 'index' is the type of worker or ware.
	// If 'adjust' is false, the three possible states kPriorityHigh,
	// kPriorityNormal and kPriorityLow are returned, otherwise numerical
	// values adjusted to the preciousness of the ware in general are returned.
	virtual int32_t get_priority(WareWorker type, DescriptionIndex, bool adjust = true) const;
	void set_priority(int32_t type, DescriptionIndex ware_index, int32_t new_priority);

	void collect_priorities(std::map<int32_t, std::map<DescriptionIndex, int32_t>>& p) const;

	/**
	 * The former buildings vector keeps track of all former buildings
	 * that have been enhanced up to the current one. The current building
	 * index will be in the last position. For construction sites, it is
	 * empty except enhancements. For a dismantle site, the last item will
	 * be the one being dismantled.
	 */
	const FormerBuildings get_former_buildings() {
		return old_buildings_;
	}

	void log_general_info(const EditorGameBase&) const override;

	//  Use on training sites only.
	virtual void change_train_priority(uint32_t, int32_t) {
	}
	virtual void switch_train_mode() {
	}

	///  Stores the PlayerNumber of the player who has defeated this building.
	void set_defeating_player(PlayerNumber const player_number) {
		defeating_player_ = player_number;
	}

	void add_worker(Worker&) override;
	void remove_worker(Worker&) override;

	virtual const BuildingSettings* create_building_settings() const {
		return nullptr;
	}

	// AttackTarget object associated with this building. If the building can
	// never be attacked (for example productionsites) this will be nullptr.
	const AttackTarget* attack_target() const {
		return attack_target_;
	}

	// SoldierControl object associated with this building. If the building can
	// not house soldiers (for example productionsites) this will be nullptr.
	const SoldierControl* soldier_control() const {
		return soldier_control_;
	}
	SoldierControl* mutable_soldier_control() {
		return soldier_control_;
	}

	void send_message(Game& game,
	                  const Message::Type msgtype,
	                  const std::string& title,
	                  const std::string& icon_filename,
	                  const std::string& heading,
	                  const std::string& description,
	                  bool link_to_building_lifetime = true,
	                  const Duration& throttle_time = Duration(0),
	                  uint32_t throttle_radius = 0);

	bool mute_messages() const {
		return mute_messages_;
	}
	void set_mute_messages(bool m) {
		mute_messages_ = m;
	}

	void start_animation(EditorGameBase&, uint32_t anim);

	bool is_seeing() const {
		return seeing_;
	}

protected:
	// Updates 'statistics_string' with the string that should be displayed for
	// this building right now. Overwritten by child classes.
	virtual void update_statistics_string(std::string*) {
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
	void
	draw_info(InfoToDraw info_to_draw, const Vector2f& point_on_dst, float scale, RenderTarget* dst);

	void set_seeing(bool see);
	void set_attack_target(AttackTarget* new_attack_target);
	void set_soldier_control(SoldierControl* new_soldier_control);

	Coords position_;
	Flag* flag_;

	uint32_t anim_;
	Time animstart_;

	using LeaveQueue = std::vector<OPtr<Worker>>;
	LeaveQueue leave_queue_;     //  FIFO queue of workers leaving the building
	Time leave_time_;        //  when to wake the next one from leave queue
	ObjectPointer leave_allow_;  //  worker that is allowed to leave now

	//  The player who has defeated this building.
	PlayerNumber defeating_player_;

	std::map<DescriptionIndex, int32_t> ware_priorities_;

	/// Whether we see our vision_range area based on workers in the building
	bool seeing_;

	// The former buildings names, with the current one in last position.
	FormerBuildings old_buildings_;
	const MapObjectDescr* was_immovable_;

private:
	std::string statistics_string_;
	AttackTarget* attack_target_;      // owned by the base classes, set by 'set_attack_target'.
	SoldierControl* soldier_control_;  // owned by the base classes, set by 'set_soldier_control'.

	bool mute_messages_;
	bool is_destruction_blocked_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_BUILDING_H
