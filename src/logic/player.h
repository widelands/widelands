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

#ifndef WL_LOGIC_PLAYER_H
#define WL_LOGIC_PLAYER_H

#include <memory>

#include "base/macros.h"
#include "economy/economy.h"
#include "graphic/color.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/message_queue.h"
#include "logic/see_unsee_node.h"
#include "logic/widelands.h"
#include "sound/constants.h"

class Node;
namespace Widelands {

struct Path;
struct PlayerImmovable;
class TrainingSite;
struct Road;
struct Waterway;

/**
 * Manage in-game aspects of players, such as tribe, team, fog-of-war, statistics,
 * messages (notification when a resource has been found etc.) and so on.
 *
 * Also provides functions for directly building player immovables; however,
 * from the UI and AI codes, those should only ever be issued indirectly via
 * \ref GameController and friends, so that replays and network games function
 * properly.
 */
class Player {
public:
	struct BuildingStats {
		bool is_constructionsite;
		Coords pos;
	};
	using BuildingStatsVector = std::vector<BuildingStats>;
	using PlayerBuildingStats = std::vector<BuildingStatsVector>;

	friend class EditorGameBase;
	friend struct GamePlayerInfoPacket;
	friend struct GamePlayerEconomiesPacket;
	friend struct GamePlayerAiPersistentPacket;
	friend class MapBuildingdataPacket;
	friend class MapPlayersViewPacket;

	Player(EditorGameBase&,
	       PlayerNumber,
	       uint8_t initialization_index,
	       const TribeDescr& tribe,
	       const std::string& name);
	~Player();

	void allocate_map();

	const MessageQueue& messages() const {
		return messages_;
	}
	MessageQueue* get_messages() {
		return &messages_;
	}

	/// Adds the message to the queue.
	MessageId add_message(Game&, std::unique_ptr<Message> message, bool popup = false);

	/// Like add_message, but if there has been a message from the same sender
	/// in the last timeout milliseconds in a radius r around the coordinates
	/// of m, the message deallocated instead.
	MessageId add_message_with_timeout(Game&,
	                                   std::unique_ptr<Message> message,
	                                   uint32_t timeout,
	                                   uint32_t radius);

	/// Indicates that the object linked to the message has been removed
	/// from the game. This implementation deletes the message.
	void message_object_removed(MessageId mid) const;

	void set_message_status(const MessageId& id, Message::Status const status) {
		get_messages()->set_message_status(id, status);
	}

	const std::set<Serial>& ships() const;
	void add_ship(Serial ship);
	void remove_ship(Serial ship);

	const EditorGameBase& egbase() const {
		return egbase_;
	}
	EditorGameBase& egbase() {
		return egbase_;
	}
	PlayerNumber player_number() const {
		return player_number_;
	}
	TeamNumber team_number() const {
		return team_number_;
	}
	const RGBColor& get_playercolor() const {
		return kPlayerColors[player_number_ - 1];
	}
	const TribeDescr& tribe() const {
		return tribe_;
	}

	const std::string& get_name() const {
		return name_;
	}
	void set_name(const std::string& name) {
		name_ = name;
	}
	void set_team_number(TeamNumber team);

	void create_default_infrastructure();

	NodeCaps get_buildcaps(const FCoords&) const;

	bool is_hostile(const Player&) const;

	/**
	 * Returns whether the player lost the last warehouse.
	 */
	bool is_defeated() const;

	// For cheating
	void set_see_all(bool const t) {
		see_all_ = t;
	}
	bool see_all() const {
		return see_all_ || is_picking_custom_starting_position_;
	}

	/// Data that are used and managed by AI. They are here to have it saved as a part of player's
	/// data
	struct AiPersistentState {
		// TODO(tiborb): this should be replaced by command line switch
		static constexpr size_t kMagicNumbersSize = 200;
		static constexpr size_t kNeuronPoolSize = 80;
		static constexpr size_t kFNeuronPoolSize = 60;

		// Seafaring constants for controlling expeditions
		static constexpr uint32_t kColonyScanStartArea = 35;
		static constexpr uint32_t kColonyScanMinArea = 12;
		static constexpr uint32_t kNoExpedition = 0;

		AiPersistentState()
		   : initialized(false),
		     colony_scan_area(0),
		     trees_around_cutters(0),
		     expedition_start_time(0),
		     ships_utilization(0),
		     no_more_expeditions(false),
		     last_attacked_player(0),
		     least_military_score(0),
		     target_military_score(0),
		     ai_productionsites_ratio(0),
		     ai_personality_mil_upper_limit(0),
		     magic_numbers(kMagicNumbersSize, 0),
		     neuron_weights(kNeuronPoolSize, 0),
		     neuron_functs(kNeuronPoolSize, 0),
		     f_neurons(kFNeuronPoolSize, 0) {
		}

		void initialize();

		// Was initialized
		bool initialized;
		uint32_t colony_scan_area;
		uint32_t trees_around_cutters;
		uint32_t expedition_start_time;
		int16_t
		   ships_utilization;  // 0-10000 to avoid floats, used for decision for building new ships
		bool no_more_expeditions;
		int16_t last_attacked_player;
		int32_t least_military_score;
		int32_t target_military_score;
		uint32_t ai_productionsites_ratio;
		int32_t ai_personality_mil_upper_limit;
		std::vector<int16_t> magic_numbers;
		std::vector<int8_t> neuron_weights;
		std::vector<int8_t> neuron_functs;
		std::vector<uint32_t> f_neurons;
		std::unordered_map<Widelands::DescriptionIndex, uint32_t> remaining_basic_buildings;
	} ai_data;

	AiPersistentState* get_mutable_ai_persistent_state() {
		return &ai_data;
	}

	/// Per-player field information.
	struct Field {
		Field()
		   : military_influence(0),
		     seeing(SeeUnseeNode::kUnexplored),
		     r_e(RoadSegment::kNone),
		     r_se(RoadSegment::kNone),
		     r_sw(RoadSegment::kNone),
		     owner(0),
		     time_node_last_unseen(0),
		     map_object_descr(nullptr),
		     border(0),
		     border_r(0),
		     border_br(0),
		     border_bl(0) {
			//  Must be initialized because the rendering code is accessing it
			//  even for triangles that the player does not see (it is the
			//  darkening that actually hides the ground from the user).
			terrains.d = terrains.r = 0;

			time_triangle_last_surveyed[0] = never();
			time_triangle_last_surveyed[1] = never();
		}

		/// Military influence is exerted by buildings with the help of soldiers.
		/// When the first soldier enters a building, it starts to exert military
		/// influence on the nodes within its conquer radius. When a building
		/// starts to exert military influence, it adds influence values to the
		/// nodes. When the last soldier leaves the building, it stops to exert
		/// military influence. Then the same values are subtracted from the
		/// nodes. Adding and subtracting military influence values affects land
		/// ownership according to certain rules.
		///
		/// This is not saved/loaded. It is recalculated during the loading
		/// process by adding influence values to the nodes surrounding a
		/// building when the first soldier located in it is loaded.
		MilitaryInfluence military_influence;

		/// Indicates whether the player is currently seeing this node or has
		/// has ever seen it.
		///
		/// The value is
		///  `kUnexplored`      if the player has never seen the node
		///  `kPreviouslySeen`  if the player does not currently see
		///                     the node, but has seen it previously
		///  `kVisible`         if the player currently sees the node
		///
		/// Note a fundamental difference between seeing a node, and having
		/// knownledge about resources. A node is considered continuously seen by
		/// a player as long as it is within vision range of any person of that
		/// player. If something on the node changes, the game engine will inform
		/// that player about it. But resource knowledge is not continuous. It is
		/// instant (given at the time when the geological survey completes) and
		/// immediately starts aging. Mining implies geological surveying, so a
		/// player will be informed about resource changes that he causes himself
		/// by mining.
		///
		/// Buildings do not see on their own. Only people can see. But as soon
		/// as a person enters a building, the person stops seeing. If it is the
		/// only person in the building, the building itself starts to see (some
		/// buildings, such as fortresses usually see much further than persons
		/// standing on the ground). As soon as a person leaves a building, the
		/// person begins to see on its own. If the building becomes empty of
		/// people, it stops seeing. Exception: Warehouses always see.
		///
		/// Only the Boolean representation of this value (whether the node has
		/// ever been seen) is saved/loaded. The complete value is then obtained
		/// by the calls to see_node or see_area peformed by all the building and
		/// worker objects that can see the node.
		///
		/// \note Never change this directly. Use update_vision() to recalculate.
		SeeUnseeNode seeing;

		//  Below follows information about the field, as far as this player
		//  knows.

		/**
		 * The terrain types of the 2 triangles, as far as this player knows.
		 * Each value is only valid when one of the corner nodes of the triangle
		 * has been seen.
		 */
		Widelands::Field::Terrains terrains;

		/**
		 * The road types of the 3 edges, as far as this player knows.
		 * Each value is only valid when this player has seen this node
		 * or the node to the the edge leads up to.
		 */
		RoadSegment r_e;
		RoadSegment r_se;
		RoadSegment r_sw;

		/**
		 * The owner of this node, as far as this player knows.
		 * Only valid when this player has seen this node.
		 */
		PlayerNumber owner;

		/**
		 * The amount of resource at each of the triangles, as far as this player
		 * knows.
		 * The d component is only valid when time_last_surveyed[0] != Never().
		 * The r component is only valid when time_last_surveyed[1] != Never().
		 */
		// TODO(unknown): Check this on access, at least in debug builds
		Widelands::Field::ResourceAmounts resource_amounts;

		/// Whether there is a road between this node and the node to the
		/// east, as far as this player knows.
		/// Only valid when this player has seen this node or the node to the
		/// east.
		RoadSegment road_e() const {
			return r_e;
		}

		/// Whether there is a road between this node and the node to the
		/// southeast, as far as this player knows.
		/// Only valid when this player has seen this node or the node to the
		/// southeast.
		RoadSegment road_se() const {
			return r_se;
		}

		/// Whether there is a road between this node and the node to the
		/// southwest, as far as this player knows.
		/// Only valid when this player has seen this node or the node to the
		/// southwest.
		RoadSegment road_sw() const {
			return r_sw;
		}

		/**
		 * The last time when this player surveyed the respective triangle
		 * geologically. Indexed by TCoords::TriangleIndex. A geologic survey is a
		 * thorough investigation. Therefore it is considered impossible to have
		 * knowledge about the resources of a triangle without having knowledge
		 * about each of the surrounding nodes:
		 *
		 *     geologic information about a triangle =>
		 *         each neighbouring node has been seen
		 *
		 * and the contrapositive:
		 *
		 *     some neighbouring node has never been seen =>
		 *         no geologic information about the triangle
		 *
		 * Is EditorGameBase::Never() when never surveyed.
		 */
		Time time_triangle_last_surveyed[2];

		/**
		 * The last time when this player saw this node.
		 * Only valid when \ref seeing is kPreviouslySeen, i.e. the player has previously seen
		 * this node but can't see it right now.
		 *
		 * This value is only for the node.
		 *
		 * The corresponding value for a triangle between the nodes A, B and C is
		 *   max
		 *     (time_node_last_unseen for A,
		 *      time_node_last_unseen for B,
		 *      time_node_last_unseen for C)
		 * and is only valid if all of {A, B, C} are currently not seen
		 * (i.e. \ref seeing != kVisible)
		 * and at least one of them has been seen at least once
		 * (i.e. \ref seeing == kPreviouslySeen).
		 *
		 * The corresponding value for an edge between the nodes A and B is
		 *   max(time_node_last_unseen for A, time_node_last_unseen for B)
		 * and is only valid if all of {A, B} are currently not seen and at
		 * least one of them has been seen at least once.
		 *
		 */
		Time time_node_last_unseen;

		/**
		 * The type of immovable on this node, as far as this player knows.
		 * Only valid when the player has seen this node (or maybe a nearby node
		 * if the immovable is big?). (Roads are not stored here.)
		 */
		const MapObjectDescr* map_object_descr;

		/* Information for constructionSite and DismantleSite animation.
		 * `constructionsite` is only valid if there is a constructionsite
		 * on this node. `dismantlesite.progress` equals the value of
		 * `get_built_per64k()` at the time the dismantlesite was last seen.
		 */
		union PartiallyFinishedBuildingDetails {
			ConstructionsiteInformation constructionsite;
			struct {
				uint32_t progress;
				const BuildingDescr* building;
			} dismantlesite;
			PartiallyFinishedBuildingDetails();
			~PartiallyFinishedBuildingDetails() {
			}
		} partially_finished_building;

		/// Save whether the player saw a border the last time (s)he saw the node.
		bool border;
		bool border_r;
		bool border_br;
		bool border_bl;

	private:
		DISALLOW_COPY_AND_ASSIGN(Field);
	};

	const Field* fields() const {
		return fields_.get();
	}

	SeeUnseeNode get_vision(MapIndex) const;
	bool is_seeing(MapIndex i) const {
		return get_vision(i) == SeeUnseeNode::kVisible;
	}

	// Cause this player and all his team mates to recalculate the visibility
	// state of the given area of fields. If `force_visible` is true, we
	// will assume without checking that we can see all fields of this area.
	void update_vision(const Area<FCoords>&, bool force_visible);

	/// Explicitly hide or reveal the given field. The modes are as follows:
	/// - kPreviouslySeen: Decrement the field's vision
	/// - kUnexplored:     Make the field completely black
	/// - kVisible:        Give the player full vision of this field.
	// Note that kPreviouslySeen and kVisible will work as expected only when
	// no building or worker is seeing the field. But they will always undo
	// the effects of revealing the field with kVisible.
	void hide_or_reveal_field(const Coords&, SeeUnseeNode);

	MilitaryInfluence military_influence(MapIndex const i) const {
		return fields_[i].military_influence;
	}

	MilitaryInfluence& military_influence(MapIndex const i) {
		return fields_[i].military_influence;
	}

	bool is_worker_type_allowed(const DescriptionIndex& i) const;
	void allow_worker_type(DescriptionIndex, bool allow);

	// Allowed buildings. A building is also allowed if it's a militarysite that the player's tribe
	// doesn't have.
	bool is_building_type_allowed(const DescriptionIndex& i) const;
	void allow_building_type(DescriptionIndex, bool allow);

	// Player commands
	// Only to be called indirectly via CmdQueue
	Flag& force_flag(const FCoords&);  /// Do what it takes to create the flag.
	Flag* build_flag(const Coords&);   /// Build a flag if it is allowed.
	Road& force_road(const Path&);
	Road* build_road(const Path&);  /// Build a road if it is allowed.
	Waterway& force_waterway(const Path&);
	Waterway* build_waterway(const Path&);  /// Build a waterway if it is allowed.
	Building& force_building(Coords, const FormerBuildings&);
	Building& force_csite(Coords, DescriptionIndex, const FormerBuildings& = FormerBuildings());
	Building* build(Coords, DescriptionIndex, bool, FormerBuildings&);
	void bulldoze(PlayerImmovable&, bool recurse = false);
	void flagaction(Flag&);
	void start_stop_building(PlayerImmovable&);
	void military_site_set_soldier_preference(PlayerImmovable&,
	                                          SoldierPreference soldier_preference);
	void start_or_cancel_expedition(Warehouse&);
	void enhance_building(Building*, DescriptionIndex index_of_new_building, bool keep_wares);
	void dismantle_building(Building*, bool keep_wares);

	Economy* create_economy(WareWorker);
	Economy* create_economy(Serial serial, WareWorker);  // For saveloading only
	void remove_economy(Serial serial);
	const std::map<Serial, std::unique_ptr<Economy>>& economies() const;
	Economy* get_economy(Widelands::Serial serial) const;
	bool has_economy(Widelands::Serial serial) const;

	uint32_t get_current_produced_statistics(uint8_t);

	// Military stuff
	void drop_soldier(PlayerImmovable&, Soldier&);
	void change_training_options(TrainingSite&, TrainingAttribute attr, int32_t val);

	uint32_t find_attack_soldiers(Flag&,
	                              std::vector<Soldier*>* soldiers = nullptr,
	                              uint32_t max = std::numeric_limits<uint32_t>::max());
	void enemyflagaction(Flag&, PlayerNumber attacker, const std::vector<Widelands::Soldier*>&);

	uint32_t casualties() const {
		return casualties_;
	}
	uint32_t kills() const {
		return kills_;
	}
	uint32_t msites_lost() const {
		return msites_lost_;
	}
	uint32_t msites_defeated() const {
		return msites_defeated_;
	}
	uint32_t civil_blds_lost() const {
		return civil_blds_lost_;
	}
	uint32_t civil_blds_defeated() const {
		return civil_blds_defeated_;
	}
	void count_casualty() {
		++casualties_;
	}
	void count_kill() {
		++kills_;
	}
	void count_msite_lost() {
		++msites_lost_;
	}
	void count_msite_defeated() {
		++msites_defeated_;
	}
	void count_civil_bld_lost() {
		++civil_blds_lost_;
	}
	void count_civil_bld_defeated() {
		++civil_blds_defeated_;
	}

	// Statistics
	const BuildingStatsVector& get_building_statistics(const DescriptionIndex& i) const;

	std::vector<uint32_t> const* get_ware_production_statistics(DescriptionIndex const) const;

	std::vector<uint32_t> const* get_ware_consumption_statistics(DescriptionIndex const) const;

	std::vector<uint32_t> const* get_ware_stock_statistics(DescriptionIndex const) const;

	void init_statistics();
	void
	read_statistics(FileRead&, uint16_t packet_version, const TribesLegacyLookupTable& lookup_table);
	void write_statistics(FileWrite&) const;
	void read_remaining_shipnames(FileRead&);
	void write_remaining_shipnames(FileWrite&) const;
	void sample_statistics();
	void ware_produced(DescriptionIndex);

	void ware_consumed(DescriptionIndex, uint8_t);
	void next_ware_production_period();

	void set_ai(const std::string&);
	const std::string& get_ai() const;

	// used in shared kingdom mode
	void add_further_starting_position(uint8_t plr, uint8_t init) {
		further_shared_in_player_.push_back(plr);
		further_initializations_.push_back(init);
	}

	void set_attack_forbidden(PlayerNumber who, bool forbid);
	bool is_attack_forbidden(PlayerNumber who) const;

	const std::string pick_shipname();

	void add_seer(const MapObject&, const Area<FCoords>&);
	void add_seer(const MapObject&);
	void remove_seer(const MapObject&, const Area<FCoords>&);

	void add_soldier(unsigned h, unsigned a, unsigned d, unsigned e);
	void remove_soldier(unsigned h, unsigned a, unsigned d, unsigned e);
	uint32_t count_soldiers(unsigned h, unsigned a, unsigned d, unsigned e) const;
	uint32_t count_soldiers_h(unsigned) const;
	uint32_t count_soldiers_a(unsigned) const;
	uint32_t count_soldiers_d(unsigned) const;
	uint32_t count_soldiers_e(unsigned) const;

	bool is_muted(DescriptionIndex di) const {
		return muted_building_types_.count(di);
	}
	void set_muted(DescriptionIndex, bool mute);

	void start_picking_custom_starting_position() {
		assert(!is_picking_custom_starting_position_);
		is_picking_custom_starting_position_ = true;
	}
	bool pick_custom_starting_position(const Coords&);
	void do_pick_custom_starting_position(const Coords&);
	bool is_picking_custom_starting_position() const {
		return is_picking_custom_starting_position_;
	}
	bool get_starting_position_suitability(const Coords&) const;

private:
	BuildingStatsVector* get_mutable_building_statistics(const DescriptionIndex& i);
	void update_building_statistics(Building&, NoteImmovable::Ownership ownership);
	void update_team_players();
	void play_message_sound(const Message* message);
	void enhance_or_dismantle(Building*, DescriptionIndex index_of_new_building, bool keep_wares);

	// Called when a node becomes seen or has changed.  Discovers the node and
	// those of the 6 surrounding edges/triangles that are not seen from another
	// node.
	void rediscover_node(const Map&, const FCoords&);

	std::unique_ptr<Notifications::Subscriber<NoteImmovable>> immovable_subscriber_;
	std::unique_ptr<Notifications::Subscriber<NoteFieldTerrainChanged>>
	   field_terrain_changed_subscriber_;

	MessageQueue messages_;

	EditorGameBase& egbase_;
	uint8_t initialization_index_;
	std::vector<uint8_t> further_initializations_;   // used in shared kingdom mode
	std::vector<uint8_t> further_shared_in_player_;  //  ''  ''   ''     ''     ''
	TeamNumber team_number_;
	std::set<PlayerNumber> team_player_;
	bool see_all_;
	const PlayerNumber player_number_;
	const TribeDescr& tribe_;  // buildings, wares, workers, sciences
	uint32_t casualties_, kills_;
	uint32_t msites_lost_, msites_defeated_;
	uint32_t civil_blds_lost_, civil_blds_defeated_;

	std::list<std::string> remaining_shipnames_;
	// If we run out of ship names, we'll want to continue with unique numbers
	uint32_t ship_name_counter_;

	std::unique_ptr<Field[]> fields_;
	std::set<DescriptionIndex> allowed_worker_types_;
	std::set<DescriptionIndex> allowed_building_types_;
	std::map<Serial, std::unique_ptr<Economy>> economies_;
	std::set<Serial> ships_;
	std::string name_;  // Player name
	std::string ai_;    /**< Name of preferred AI implementation */

	bool should_see(const FCoords&) const;
	// Own bobs and buildings that are seeing fields in their vicinity
	std::list<const MapObject*> seers_;

	void update_vision(const FCoords&, bool force_visible);
	void update_vision_whole_map();
	std::set<MapIndex> revealed_fields_;

	/**
	 * Wares produced (by ware id) since the last call to @ref sample_statistics
	 */
	std::map<DescriptionIndex, Quantity> current_produced_statistics_;

	/**
	 * Wares consumed (by ware id) since the last call to @ref sample_statistics
	 */
	std::map<DescriptionIndex, Quantity> current_consumed_statistics_;

	using StatisticsMap = std::map<DescriptionIndex, std::vector<Quantity>>;

	/**
	 * Statistics of wares produced over the life of the game, indexed as
	 * ware_productions_[ware id][time index]
	 */
	StatisticsMap ware_productions_;

	/**
	 * Statistics of wares consumed over the life of the game, indexed as
	 * ware_consumptions_[ware_id][time_index]
	 */
	StatisticsMap ware_consumptions_;

	/**
	 * Statistics of wares stored inside of warehouses over the
	 * life of the game, indexed as
	 * ware_stocks_[ware_id][time_index]
	 */
	StatisticsMap ware_stocks_;

	std::set<DescriptionIndex> muted_building_types_;

	std::set<PlayerNumber> forbid_attack_;

	PlayerBuildingStats building_stats_;

	struct SoldierStatistics {
		const unsigned health, attack, defense, evade;
		Quantity total;
		SoldierStatistics(unsigned h, unsigned a, unsigned d, unsigned e)
		   : health(h), attack(a), defense(d), evade(e), total(0) {
		}
		bool operator==(const SoldierStatistics& s) const {
			return s.health == health && s.attack == attack && s.defense == defense &&
			       s.evade == evade;
		}
	};
	std::vector<SoldierStatistics> soldier_stats_;

	bool is_picking_custom_starting_position_;

	FxId message_fx_;
	FxId attack_fx_;
	FxId occupied_fx_;

	DISALLOW_COPY_AND_ASSIGN(Player);
};

void find_former_buildings(const Tribes& tribes,
                           const DescriptionIndex bi,
                           FormerBuildings* former_buildings);
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_PLAYER_H
