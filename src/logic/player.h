/*
 * Copyright (C) 2002-2003, 2006-2011 by the Widelands Development Team
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

#ifndef PLAYER_H
#define PLAYER_H

#include "logic/building.h"
#include "logic/editor_game_base.h"
#include "logic/mapregion.h"
#include "logic/message_queue.h"
#include "logic/notification.h"
#include "rgbcolor.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "logic/widelands.h"

class Node;
namespace Widelands {

class Economy;
struct Path;
struct PlayerImmovable;
class Soldier;
class TrainingSite;
struct Flag;
struct Tribe_Descr;
struct Road;
struct AttackController;

/**
 * Manage in-game aspects of players, such as tribe, team, fog-of-war, statistics,
 * messages (notification when a resource has been found etc.) and so on.
 *
 * Also provides functions for directly building player immovables; however,
 * from the UI and AI codes, those should only ever be issued indirectly via
 * \ref GameController and friends, so that replays and network games function
 * properly.
 */
class Player :
	boost::noncopyable,
	public NoteReceiver<NoteImmovable>, public NoteReceiver<NoteFieldPossession>,
	public NoteSender  <NoteImmovable>, public NoteSender  <NoteFieldPossession>
{
public:
	// hard-coded playercolors
	static const RGBColor Colors[MAX_PLAYERS];

	struct Building_Stats {
		bool is_constructionsite;
		Coords pos;
	};
	typedef std::vector<Building_Stats> Building_Stats_vector;
	typedef std::vector<Building_Stats_vector> BuildingStats;

	friend class Editor_Game_Base;
	friend struct Game_Player_Info_Data_Packet;
	friend struct Game_Player_Economies_Data_Packet;
	friend struct Map_Buildingdata_Data_Packet;
	friend struct Map_Players_View_Data_Packet;
	friend struct Map_Exploration_Data_Packet;

	Player
		(Editor_Game_Base &,
		 Player_Number,
		 uint8_t initialization_index,
		 const Tribe_Descr & tribe,
		 const std::string & name);
	~Player();

	void allocate_map();

	const MessageQueue & messages() const {return m_messages;}
	MessageQueue       & messages()       {return m_messages;}

	/// Adds the message to the queue. Takes ownership of the message. Assumes
	/// that it has been allocated in a separate memory block (not as a
	/// component of an array or struct) with operator new, so that it can be
	/// deallocated with operator delete.
	Message_Id add_message(Game &, Message &, bool popup = false);

	/// Like add_message, but if there has been a message from the same sender
	/// in the last timeout milliseconds in a radius r around the coordinates
	/// of m, the message deallocated instead.
	Message_Id add_message_with_timeout
		(Game &, Message &, uint32_t timeout, uint32_t radius);

	/// Indicates that the object linked to the message has been removed
	/// from the game. This implementation expires the message.
	void message_object_removed(Message_Id mid) const;

	void set_message_status(const Message_Id& id, Message::Status const status) {
		messages().set_message_status(id, status);
	}

	const Editor_Game_Base & egbase() const {return m_egbase;}
	Editor_Game_Base       & egbase()       {return m_egbase;}
	Player_Number     player_number() const {return m_plnum;}
	TeamNumber team_number() const {return m_team_number;}
	const RGBColor & get_playercolor() const {return Colors[m_plnum - 1];}
	const Tribe_Descr & tribe() const {return m_tribe;}

	const std::string & get_name() const {return m_name;}
	void set_name(const std::string & name) {m_name = name;}
	void set_frontier_style(uint8_t a) {m_frontier_style_index = a;}
	void set_flag_style(uint8_t a) {m_flag_style_index = a;}
	void set_team_number(TeamNumber team);

	void create_default_infrastructure();

	NodeCaps get_buildcaps(FCoords) const;

	bool is_hostile(const Player &) const;

	// For cheating
	void set_see_all(bool const t) {m_see_all = t; m_view_changed = true;}
	bool see_all() const {return m_see_all;}

	/// Per-player and per-field constructionsite information
	struct Constructionsite_Information {
		Constructionsite_Information() : becomes(nullptr), was(nullptr), totaltime(0), completedtime(0) {}
		const Building_Descr * becomes; // Also works as a marker telling whether there is a construction site.
		const Building_Descr * was; // only valid if "becomes" is an enhanced building.
		uint32_t               totaltime;
		uint32_t               completedtime;
	};

	/// Per-player field information.
	struct Field : boost::noncopyable {
		Field() :
			military_influence(0),
			vision            (0),
			roads(0),
			owner(0),
			time_node_last_unseen(0),
			border(0),
			border_r(0),
			border_br(0),
			border_bl(0)
		{
			//  Must be initialized because the rendering code is accessing it
			//  even for triangles that the player does not see (it is the
			//  darkening that actually hides the ground from the user). This is
			//  important for worlds where the number of terrain types is not
			//  maximal (16), so that an uninitialized terrain index could cause a
			//  not found error in
			//  DescriptionMaintainer<Terrain_Descr>::get(Terrain_Index).
			terrains.d = terrains.r = 0;

			time_triangle_last_surveyed[0] = Never();
			time_triangle_last_surveyed[1] = Never();

			//  Initialized for debug purposes only.
			map_object_descr[0] = map_object_descr[1] = map_object_descr[2] = nullptr;
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
		Military_Influence military_influence;

		/// Indicates whether the player is currently seeing this node or has
		/// has ever seen it.
		///
		/// The value is
		///  0    if the player has never seen the node
		///  1    if the player does not currently see the node, but has seen it
		///       previously
		///  1+n  if the player currently sees the node, where n is the number of
		///       objects that can see the node.
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
		/// people, it stops seeing.
		///
		/// Only the Boolean representation of this value (whether the node has
		/// ever been seen) is saved/loaded. The complete value is then obtained
		/// by the calls to see_node or see_area peformed by all the building and
		/// worker objects that can see the node.
		///
		/// \note Never change this variable directly. Instead, use the functions
		/// \ref see_node and \ref unsee_node or, more conveniently,
		/// \ref see_area and \ref unsee_area .
		Vision vision;


		//  Below follows information about the field, as far as this player
		//  knows.

		/**
		 * The terrain types of the 2 triangles, as far as this player knows.
		 * Each value is only valid when one of the corner nodes of the triangle
		 * has been seen.
		 */
		Widelands::Field::Terrains terrains;

		uint8_t       roads;

		/**
		 * The owner of this node, as far as this player knows.
		 * Only valid when this player has seen this node.
		 */
		Player_Number owner;

		/**
		 * The amount of resource at each of the triangles, as far as this player
		 * knows.
		 * The d component is only valid when time_last_surveyed[0] != Never().
		 * The r component is only valid when time_last_surveyed[1] != Never().
		 * \todo Check this on access, at least in debug builds
		 */
		Widelands::Field::Resource_Amounts resource_amounts;

		/// Whether there is a road between this node and the node to the
		/// east, as far as this player knows.
		/// Only valid when this player has seen this node or the node to the
		/// east.
		uint8_t road_e() const {return roads & Road_Mask;}

		/// Whether there is a road between this node and the node to the
		/// southeast, as far as this player knows.
		/// Only valid when this player has seen this node or the node to the
		/// southeast.
		uint8_t road_se() const {
			return roads >> Road_SouthEast & Road_Mask;
		}

		/// Whether there is a road between this node and the node to the
		/// southwest, as far as this player knows.
		/// Only valid when this player has seen this node or the node to the
		/// southwest.
		uint8_t road_sw() const {
			return roads >> Road_SouthWest & Road_Mask;
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
		 * Is Editor_Game_Base::Never() when never surveyed.
		 */
		Time time_triangle_last_surveyed[2];

		/**
		 * The last time when this player saw this node.
		 * Only valid when \ref vision is 1, i.e. the player has previously seen
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
		 * (i.e. \ref vision <= 1)
		 * and at least one of them has been seen at least once
		 * (i.e. \ref vision == 1).
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
		const Map_Object_Descr             * map_object_descr[3];

		/// Information for constructionsite's animation.
		/// only valid, if there is a constructionsite on this node
		Constructionsite_Information constructionsite;

		/// Save whether the player saw a border the last time (s)he saw the node.
		bool border;
		bool border_r;
		bool border_br;
		bool border_bl;

		//  Summary of intended layout (not yet fully implemented)
		//
		//                                  32bit arch    64bit arch
		//                                 ============  ============
		//  Identifier                     offset  size  offset  size
		//  =======================        ======  ====  ======  ====
		//  military_influence              0x000  0x10   0x000  0x10
		//  vision                          0x010  0x10   0x010  0x10
		//  terrains                        0x020  0x08   0x020  0x08
		//  roads                           0x028  0x06   0x028  0x06
		//  owner_d                         0x02e  0x05   0x02e  0x05
		//  owner_r                         0x033  0x05   0x033  0x05
		//  resource_amounts                0x038  0x08   0x038  0x08
		//  time_triangle_last_surveyed[0]  0x040  0x20   0x040  0x20
		//  time_triangle_last_surveyed[1]  0x060  0x20   0x060  0x20
		//  time_node_last_unseen           0x080  0x20   0x080  0x20
		//  map_object_descr[0]             0x0a0  0x20   0x0a0  0x40
		//  map_object_descr[1]             0x0c0  0x20   0x0e0  0x40
		//  map_object_descr[2]             0x0e0  0x20   0x120  0x40
		//  Constructionsite_Information
		//  border
		//  border_r
		//  border_br
		//  border_bl
		//  <end>                           0x100         0x160
	};

	const Field * fields() const {return m_fields;}

	// See area
	Vision vision(Map_Index const i) const {
		// Node visible if > 1
		return (m_see_all ? 2 : 0) + m_fields[i].vision;
	}
	/**
	 * Called when a node becomes seen or has changed.  Discovers the node and
	 * those of the 6 surrounding edges/triangles that are not seen from another
	 * node.
	 */
	void rediscover_node(const Map &, const Widelands::Field &, FCoords)
	;

	bool has_view_changed() {
		bool t = m_view_changed;
		m_view_changed = false;
		return t;
	}

	/**
	 * Update this player's information about this node and the surrounding
	 * triangles and edges.
	 */
	void see_node
		(const Map &,
		 const Widelands::Field & first_map_field,
		 const FCoords,
		 const Time,
		 const bool forward = false)
	;

	/// Decrement this player's vision for a node.
	void unsee_node
		(const Map_Index,
		 const Time,
		 const bool forward = false)
	;

	/// Call see_node for each node in the area.
	void see_area(const Area<FCoords>& area) {
		const Time gametime = egbase().get_gametime();
		const Map & map = egbase().map();
		const Widelands::Field & first_map_field = map[0];
		MapRegion<Area<FCoords> > mr(map, area);
		do {
			see_node(map, first_map_field, mr.location(), gametime);
		} while (mr.advance(map));
		m_view_changed = true;
	}

	/// Decrement this player's vision for each node in an area.
	void unsee_area(const Area<FCoords>& area) {
		const Time gametime = egbase().get_gametime();
		const Map &                  map      = egbase().map         ();
		const Widelands::Field & first_map_field = map[0];
		MapRegion<Area<FCoords> > mr(map, area);
		do unsee_node(mr.location().field - &first_map_field, gametime);
		while (mr.advance(map));
		m_view_changed = true;
	}

	Military_Influence military_influence(Map_Index const i) const {
		return m_fields[i].military_influence;
	}

	Military_Influence & military_influence(Map_Index const i) {
		return m_fields[i].military_influence;
	}

	bool is_worker_type_allowed(const Ware_Index& i) const {
		return m_allowed_worker_types.at(i);
	}
	void allow_worker_type(Ware_Index, bool allow);

	// Allowed buildings
	bool is_building_type_allowed(const Building_Index& i) const {
		return m_allowed_building_types[i];
	}
	void allow_building_type(Building_Index, bool allow);

	// Battle options
	void set_retreat_percentage(uint8_t value);
	uint8_t get_retreat_percentage() const {
		return m_retreat_percentage;
	}
	void allow_retreat_change(bool allow);
	bool is_retreat_change_allowed() const {
		return m_allow_retreat_change;
	}

	// Player commands
	// Only to be called indirectly via CmdQueue
	Flag & force_flag(FCoords);      /// Do what it takes to create the flag.
	Flag *   build_flag(Coords);      /// Build a flag if it is allowed.
	Road & force_road(const Path &);
	Road * build_road(const Path &); /// Build a road if it is allowed.
	Building & force_building
		(const Coords,
		 const Building::FormerBuildings &);
	Building & force_csite
		(const Coords,
		 Building_Index,
		 const Building::FormerBuildings & = Building::FormerBuildings());
	Building * build(Coords, Building_Index, bool, Building::FormerBuildings &);
	void bulldoze(PlayerImmovable &, bool recurse = false);
	void flagaction(Flag &);
	void start_stop_building(PlayerImmovable &);
	void military_site_set_soldier_preference(PlayerImmovable &, uint8_t m_soldier_preference);
	void start_or_cancel_expedition(Warehouse &);
	void enhance_building
		(Building *, Building_Index index_of_new_building);
	void dismantle_building (Building *);

	// Economy stuff
	void    add_economy(Economy &);
	void remove_economy(Economy &);
	bool    has_economy(Economy &) const;
	typedef std::vector<Economy *> Economies;
	Economies::size_type get_economy_number(Economy const *) const;
	Economy * get_economy_by_number(Economies::size_type const i) const {
		return m_economies[i];
	}
	uint32_t get_nr_economies() const {return m_economies.size();}

	// Military stuff
	void drop_soldier(PlayerImmovable &, Soldier &);
	void change_training_options(TrainingSite &, int32_t atr, int32_t val);

	uint32_t findAttackSoldiers
		(Flag                   &,
		 std::vector<Soldier *> * soldiers = nullptr,
		 uint32_t                 max = std::numeric_limits<uint32_t>::max());
	void enemyflagaction
		(Flag &, Player_Number attacker, uint32_t count, uint8_t retreat);

	uint32_t casualties() const {return m_casualties;}
	uint32_t kills     () const {return m_kills;}
	uint32_t msites_lost        () const {return m_msites_lost;}
	uint32_t msites_defeated    () const {return m_msites_defeated;}
	uint32_t civil_blds_lost    () const {return m_civil_blds_lost;}
	uint32_t civil_blds_defeated() const {return m_civil_blds_defeated;}
	void count_casualty          () {++m_casualties;}
	void count_kill              () {++m_kills;}
	void count_msite_lost        () {++m_msites_lost;}
	void count_msite_defeated    () {++m_msites_defeated;}
	void count_civil_bld_lost    () {++m_civil_blds_lost;}
	void count_civil_bld_defeated() {++m_civil_blds_defeated;}

	uint32_t frontier_anim() const {
		return tribe().frontier_animation(m_frontier_style_index);
	}
	uint32_t flag_anim    () const {
		return tribe().flag_animation    (m_flag_style_index);
	}

	// Statistics
	const Building_Stats_vector & get_building_statistics
		(const Building_Index& i) const
	{
		return m_building_stats[i];
	}

	std::vector<uint32_t> const * get_ware_production_statistics
		(Ware_Index const) const;

	std::vector<uint32_t> const * get_ware_consumption_statistics
		(Ware_Index const) const;

	std::vector<uint32_t> const * get_ware_stock_statistics
		(Ware_Index const) const;

	void ReadStatistics(FileRead &, uint32_t version);
	void WriteStatistics(FileWrite &) const;
	void sample_statistics();
	void ware_produced(Ware_Index);

	void ware_consumed(Ware_Index, uint8_t);
	void next_ware_production_period();

	void receive(const NoteImmovable &) override;
	void receive(const NoteFieldPossession     &) override;
	void receive(const NoteFieldTransformed     &);

	void setAI(const std::string &);
	const std::string & getAI() const;

	// used in shared kingdom mode
	void add_further_starting_position(uint8_t plr, uint8_t init) {
		m_further_shared_in_player.push_back(plr);
		m_further_initializations .push_back(init);
	}

private:
	void update_building_statistics(Building &, losegain_t);
	void update_team_players();
	void play_message_sound(const std::string & sender);
	void _enhance_or_dismantle
		(Building *, Building_Index const index_of_new_building = Building_Index::Null());

private:
	MessageQueue           m_messages;

	Editor_Game_Base     & m_egbase;
	uint8_t                m_initialization_index;
	std::vector<uint8_t>   m_further_initializations;    // used in shared kingdom mode
	std::vector<uint8_t>   m_further_shared_in_player;   //  ''  ''   ''     ''     ''
	uint8_t                m_frontier_style_index;
	uint8_t                m_flag_style_index;
	TeamNumber             m_team_number;
	std::vector<Player *>  m_team_player;
	bool                   m_team_player_uptodate;
	bool                   m_see_all;
	bool                   m_view_changed;
	const Player_Number    m_plnum;
	const Tribe_Descr    & m_tribe; // buildings, wares, workers, sciences
	uint32_t               m_casualties, m_kills;
	uint32_t               m_msites_lost,     m_msites_defeated;
	uint32_t               m_civil_blds_lost, m_civil_blds_defeated;

	/**
	 * Is player allowed to modify m_retreat_percentage?
	 * Below percentage value, soldiers will retreat when current battle finish
	 */

	bool                  m_allow_retreat_change;
	uint8_t               m_retreat_percentage;

	Field *               m_fields;
	std::vector<bool>     m_allowed_worker_types;
	std::vector<bool>     m_allowed_building_types;
	Economies             m_economies;
	std::string           m_name; // Player name
	std::string           m_ai; /**< Name of preferred AI implementation */

	/**
	 * Wares produced (by ware id) since the last call to @ref sample_statistics
	 */
	std::vector<uint32_t> m_current_produced_statistics;

	/**
	 * Wares consumed (by ware id) since the last call to @ref sample_statistics
	 */
	std::vector<uint32_t> m_current_consumed_statistics;

	/**
	 * Statistics of wares produced over the life of the game, indexed as
	 * m_ware_productions[ware id][time index]
	 */
	std::vector< std::vector<uint32_t> > m_ware_productions;

	/**
	 * Statistics of wares consumed over the life of the game, indexed as
	 * m_ware_consumptions[ware_id][time_index]
	 */
	std::vector< std::vector<uint32_t> > m_ware_consumptions;

	/**
	 * Statistics of wares stored inside of warehouses over the
	 * life of the game, indexed as
	 * m_ware_stocks[ware_id][time_index]
	 */
	std::vector< std::vector<uint32_t> > m_ware_stocks;

	BuildingStats m_building_stats;
};

void find_former_buildings
	(const Tribe_Descr & tribe_descr, const Building_Index bi,
	 Building::FormerBuildings* former_buildings);

}

#endif
