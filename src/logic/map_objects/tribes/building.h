/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include <cstring>
#include <string>
#include <vector>

#include <boost/signals2.hpp>

#include "ai/ai_hints.h"
#include "base/macros.h"
#include "logic/map_objects/buildcost.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/bill_of_materials.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/map_objects/tribes/workarea_info.h"
#include "logic/message.h"
#include "logic/widelands.h"
#include "notifications/notifications.h"
#include "scripting/lua_table.h"

struct BuildingHints;
class Image;

namespace Widelands {

struct Flag;
struct Message;
class TribeDescr;
class InputQueue;

class Building;

#define LOW_PRIORITY 2
#define DEFAULT_PRIORITY 4
#define HIGH_PRIORITY 8

/*
 * Common to all buildings!
 */
class BuildingDescr : public MapObjectDescr {
public:
	using FormerBuildings = std::vector<DescriptionIndex>;

	BuildingDescr(const std::string& init_descname,
	              MapObjectType type,
	              const LuaTable& t,
	              const EditorGameBase& egbase);
	~BuildingDescr() override {
	}

	bool is_buildable() const {
		return buildable_;
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
	const Buildcost& returned_wares() const {
		return return_dismantle_;
	}

	/**
	 * The build cost for enhancing a previous building
	 */
	const Buildcost& enhancement_cost() const {
		return enhance_cost_;
	}

	/**
	 * The returned wares for a enhaced building
	 */
	const Buildcost& returned_wares_enhanced() const {
		return return_enhanced_;
	}

	std::string helptext_script() const {
		return helptext_script_;
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
	                 Player&,
	                 Coords,
	                 bool construct,
	                 bool loading = false,
	                 FormerBuildings former_buildings = FormerBuildings()) const;

	virtual uint32_t get_conquers() const;
	virtual uint32_t vision_range() const;

	WorkareaInfo workarea_info_;

	virtual int32_t suitability(const Map&, const FCoords&) const;
	const BuildingHints& hints() const;
	void set_hints_trainingsites_max_percent(int percent);

protected:
	virtual Building& create_object() const = 0;
	Building& create_constructionsite() const;

private:
	const EditorGameBase& egbase_;
	bool buildable_;     // the player can build this himself
	bool destructible_;  // the player can destruct this himself
	Buildcost buildcost_;
	Buildcost return_dismantle_;   // Returned wares on dismantle
	Buildcost enhance_cost_;       // cost for enhancing
	Buildcost return_enhanced_;    // Returned ware for dismantling an enhanced building
	std::string helptext_script_;  // The path and filename to the building's helptext script
	int32_t size_;                 // size of the building
	bool mine_;
	bool port_;
	bool needs_seafaring_;  // This building should only be built on seafaring maps.
	DescriptionIndex enhancement_;
	DescriptionIndex
	   enhanced_from_;        // The building this building was enhanced from, or INVALID_INDEX
	bool enhanced_building_;  // if it is one, it is bulldozable
	BuildingHints hints_;     // hints (knowledge) for computer players

	// for migration, 0 is the default, meaning get_conquers() + 4
	uint32_t vision_range_;
	DISALLOW_COPY_AND_ASSIGN(BuildingDescr);
};

struct NoteBuilding {
	CAN_BE_SENT_AS_NOTE(NoteId::Building)

	Serial serial;

	enum class Action { kChanged, kDeleted, kStartWarp, kFinishWarp, kWorkersChanged };
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

	using FormerBuildings = std::vector<DescriptionIndex>;

public:
	enum class InfoStringFormat { kCensus, kStatistics, kTooltip };

	Building(const BuildingDescr&);

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

	virtual bool fetch_from_flag(Game&);
	virtual bool get_building_work(Game&, Worker&, bool success);

	bool leave_check_and_wait(Game&, Worker&);
	void leave_skip(Game&, Worker&);

	// Get/Set the priority for this waretype for this building. 'type' defines
	// if this is for a worker or a ware, 'index' is the type of worker or ware.
	// If 'adjust' is false, the three possible states HIGH_PRIORITY,
	// DEFAULT_PRIORITY and LOW_PRIORITY are returned, otherwise numerical
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

	void log_general_info(const EditorGameBase&) override;

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

	void send_message(Game& game,
	                  const Message::Type msgtype,
	                  const std::string& title,
	                  const std::string& icon_filename,
	                  const std::string& heading,
	                  const std::string& description,
	                  bool link_to_building_lifetime = true,
	                  uint32_t throttle_time = 0,
	                  uint32_t throttle_radius = 0);

protected:
	// Updates 'statistics_string' with the string that should be displayed for
	// this building right now. Overwritten by child classes.
	virtual void update_statistics_string(std::string*) {
	}

	void start_animation(EditorGameBase&, uint32_t anim);

	void init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void act(Game&, uint32_t data) override;

	void draw(uint32_t gametime,
	          TextToDraw draw_text,
	          const Vector2f& point_on_dst,
	          float scale,
	          RenderTarget* dst) override;
	void
	draw_info(TextToDraw draw_text, const Vector2f& point_on_dst, float scale, RenderTarget* dst);

	void set_seeing(bool see);

	Coords position_;
	Flag* flag_;

	uint32_t anim_;
	int32_t animstart_;

	using LeaveQueue = std::vector<OPtr<Worker>>;
	LeaveQueue leave_queue_;     //  FIFO queue of workers leaving the building
	uint32_t leave_time_;        //  when to wake the next one from leave queue
	ObjectPointer leave_allow_;  //  worker that is allowed to leave now

	//  The player who has defeated this building.
	PlayerNumber defeating_player_;

	int32_t priority_;  // base priority
	std::map<DescriptionIndex, int32_t> ware_priorities_;

	/// Whether we see our vision_range area based on workers in the building
	bool seeing_;

	// The former buildings names, with the current one in last position.
	FormerBuildings old_buildings_;

private:
	std::string statistics_string_;
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_BUILDING_H
