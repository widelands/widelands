/*
 * Copyright (C) 2002-2003, 2006-2009 by the Widelands Development Team
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

#ifndef PLAYER_H
#define PLAYER_H

#include "areawatcher.h"
#include "building.h"
#include "editor_game_base.h"
#include "mapregion.h"
#include "notification.h"
#include "rgbcolor.h"

#include "widelands.h"

namespace Widelands {

struct Economy;
struct Path;
struct PlayerImmovable;
class Soldier;
struct TrainingSite;
class Flag;
struct Tribe_Descr;
struct AttackController;

/** class Player
 *
 * What we really need is a Player class that stores e.g. score
 * and diplomacy of a player.
 *
 * These Player classes should be controlled via the cmd queue.
 * Commands are inserted by:
 *  - local player
 *  - network packets
 *  - AI code
 * Player commands should be controlled by the \ref GameController
 * in the long run.
 *                      -- Nicolai
 */
struct Player :
	public NoteReceiver<NoteImmovable>, public NoteReceiver<NoteField>,
	public NoteSender  <NoteImmovable>, public NoteSender  <NoteField>
{
	struct Building_Stats {
		bool is_constructionsite;
		Coords pos;
	};
	typedef std::vector<Building_Stats> Building_Stats_vector;
	typedef std::vector<Building_Stats_vector> BuildingStats;

	friend struct Editor_Game_Base;
	friend class Game_Player_Info_Data_Packet;
	friend class Game_Player_Economies_Data_Packet;
	friend struct Map_Buildingdata_Data_Packet;
	friend struct Map_Players_View_Data_Packet;
	friend struct Map_Seen_Fields_Data_Packet;

	Player
		(Editor_Game_Base &,
		 Player_Number,
		 uint8_t initialization_index,
		 const Tribe_Descr & tribe,
		 const std::string & name,
		 const uint8_t * playercolor);
	~Player();

	void allocate_map();

	const Editor_Game_Base & egbase() const throw () {return m_egbase;}
	Editor_Game_Base       & egbase()       throw () {return m_egbase;}
	Player_Number     player_number() const throw () {return m_plnum;}
	RGBColor const * get_playercolor() const {return m_playercolor;}
	const Tribe_Descr & tribe() const throw () {return m_tribe;}

	const std::string & get_name() const throw () {return m_name;}
	void set_name(const std::string & name) {m_name = name;}

	void create_default_infrastructure();

	FieldCaps get_buildcaps(FCoords) const;

	// For cheating
	void set_see_all(bool const t) {m_see_all = t; m_view_changed = true;}
	bool see_all() const throw () {return m_see_all;}

	/// Per-player field information.
	struct Field {
		Field() :
			military_influence(0),
			vision            (0)
		{
			//  Must be initialized because the rendering code is accessing it
			//  even for triangles that the player does not see (it is the
			//  darkening that actually hides the ground from the user). This is
			//  important for worlds where the number of terrain types is not
			//  maximal (16), so that an uninitialized terrain index could cause a
			//  not found error in
			//  Descr_Maintainer<Terrain_Descr>::get(Terrain_Index).
			terrains.d = terrains.r = 0;

			time_triangle_last_surveyed[0] = Never();
			time_triangle_last_surveyed[1] = Never();

			//  Initialized for debug purposes only.
			map_object_descr[0] = map_object_descr[1] = map_object_descr[2] = 0;
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
		uint8_t road_e() const throw () {return roads & Road_Mask;}

		/// Whether there is a road between this node and the node to the
		/// southeast, as far as this player knows.
		/// Only valid when this player has seen this node or the node to the
		/// southeast.
		uint8_t road_se() const throw () {
			return roads >> Road_SouthEast & Road_Mask;
		}

		/// Whether there is a road between this node and the node to the
		/// southwest, as far as this player knows.
		/// Only valid when this player has seen this node or the node to the
		/// southwest.
		uint8_t road_sw() const throw () {
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
		const Map_Object_Descr * map_object_descr[3];

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
		//  <end>                           0x100         0x160

	private:
		Field & operator= (Field const &);
		explicit Field    (Field const &);
	};

	const Field * fields() const throw () {return m_fields;}

	// See area
	Vision vision(Map_Index const i) const throw () {return m_fields[i].vision;}

	bool has_view_changed() {
		bool t = m_view_changed;
		m_view_changed = false;
		return t;
	}

	/**
	 * Update this player's information about this node and the surrounding
	 * triangles and edges. If lasting is true, the vision is incremented so that
	 * the node remains seen at least until a corresponding call to unsee_node is
	 * made.
	 */
	void see_node
		(const Map                  &,
		 const Widelands::Field & first_map_field,
		 const FCoords,
		 const Time,
		 const bool                   lasting = true)
		throw ();

	/// Decrement this player's vision for a node.
	void unsee_node(Map_Index const i, Time const gametime) throw () {
		Field & field = m_fields[i];
		assert(1 < field.vision);
		--field.vision;
		if (field.vision == 1)
			field.time_node_last_unseen = gametime;
		assert(1 <= field.vision);
	}

	/// Call see_node for each node in the area.
	void see_area(const Area<FCoords> area, const bool lasting = true)
		throw ()
	{
		const Time gametime = egbase().get_gametime();
		const Map & map = egbase().map();
		const Widelands::Field & first_map_field = map[0];
		MapRegion<Area<FCoords> > mr(map, area);
		do see_node(map, first_map_field, mr.location(), gametime, lasting);
		while (mr.advance(map));
		m_view_changed = true;
	}

	/// Decrement this player's vision for each node in an area.
	void unsee_area(const Area<FCoords> area) throw () {
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

	// Allowed buildings
	bool is_building_allowed(Building_Index const i) const throw () {
		return m_allowed_buildings[i];
	}
	void allow_building(Building_Index, bool allow);

	// Player commands
	// Only to be called indirectly via CmdQueue
	Flag & force_flag(FCoords);      /// Do what it takes to create the flag.
	void   build_flag(Coords);       /// Build a flag if it is allowed.
	void   force_road(Path const &, bool create_carrier = false);
	void   build_road(Path const &); /// Build a road if it is allowed.
	void force_building
		(Coords,
		 Building_Index,
		 uint32_t      const * ware_counts,
		 uint32_t      const * worker_counts,
		 Soldier_Counts const & soldier_counts);
	void build(Coords, Building_Index);
	void bulldoze(PlayerImmovable &, bool recurse = false);
	void flagaction(Flag &);
	void start_stop_building(PlayerImmovable &);
	void enhance_building
		(Building *, Building_Index index_of_new_building);

	// Economy stuff
	void    add_economy(Economy &);
	void remove_economy(Economy &);
	bool    has_economy(Economy &) const throw ();
	typedef std::vector<Economy *> Economies;
	Economies::size_type get_economy_number(Economy const *) const throw ();
	Economy * get_economy_by_number(Economies::size_type const i) const {
		return m_economies[i];
	}
	uint32_t get_nr_economies() const {return m_economies.size();}

	// Military stuff
	void drop_soldier(PlayerImmovable &, Soldier &);
	void change_training_options(TrainingSite &, int32_t atr, int32_t val);

	uint32_t findAttackSoldiers
		(Flag                   &,
		 std::vector<Soldier *> * soldiers = 0,
		 uint32_t                 max = std::numeric_limits<uint32_t>::max());
	void enemyflagaction(Flag &, Player_Number attacker, uint32_t count);

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

	AreaWatcher & add_areawatcher(const Player_Area<> player_area) {
		assert(player_area.player_number == player_number());
		see_area
			(Area<FCoords>
			 	(egbase().map().get_fcoords(player_area), player_area.radius));
		AreaWatcher & result = AreaWatcher::create(egbase(), player_area);
		m_areawatchers.insert(&result);
		return result;
	}

	void remove_areawatcher(AreaWatcher & areawatcher) {
		unsee_area
			(Area<FCoords>
			 	(egbase().map().get_fcoords(areawatcher), areawatcher.radius));
		m_areawatchers.erase(&areawatcher);
	}

	typedef std::set<Object_Ptr> AreaWatchers;
	const AreaWatchers & areawatchers() const throw () {return m_areawatchers;}

	// Statistics
	Building_Stats_vector const & get_building_statistics
		(Building_Index const i) const
	{
		return m_building_stats[i];
	}
	std::vector<uint32_t> const * get_ware_production_statistics
		(Ware_Index const) const;

	void ReadStatistics(FileRead &, uint32_t version);
	void WriteStatistics(FileWrite &) const;
	void sample_statistics();
	void ware_produced(Ware_Index);
	void next_ware_production_period();

	void receive(NoteImmovable const &);
	void receive(NoteField     const &);

	void setAI(std::string const &);
	const std::string & getAI() const;

private:
	void update_building_statistics(Building &, losegain_t);

	/**
	 * Called when a node becomes seen (not only the first time) before the
	 * vision counter is incremented. Discovers the node and those of the 6
	 * surrounding edges/triangles that are not seen from another node.
	 */
	void discover_node(Map const &, Widelands::Field const &, FCoords, Field &)
		throw ();

private:
	AreaWatchers           m_areawatchers;

	Editor_Game_Base     & m_egbase;
	uint8_t                m_initialization_index;
	bool m_see_all;
	bool                   m_view_changed;
	const Player_Number    m_plnum;
	Tribe_Descr const    & m_tribe; // buildings, wares, workers, sciences
	uint32_t               m_casualties, m_kills;
	uint32_t               m_msites_lost,     m_msites_defeated;
	uint32_t               m_civil_blds_lost, m_civil_blds_defeated;
	RGBColor               m_playercolor[4];

	Field *               m_fields;
	std::vector<bool>     m_allowed_buildings;
	Economies             m_economies;
	std::string           m_name; // Player name
	std::string           m_ai; /**< Name of preferred AI implementation */

	std::vector<uint32_t> m_current_statistics;
	std::vector< std::vector<uint32_t> > m_ware_productions;
	BuildingStats m_building_stats;
};

};

#endif
