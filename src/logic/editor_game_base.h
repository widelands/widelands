/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2011 by the Widelands Development Team
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

#ifndef EDITOR_GAME_BASE_H
#define EDITOR_GAME_BASE_H

#include <cstring>
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>

#include "logic/bob.h"
#include "logic/building.h"
#include "constants.h"
#include "logic/map.h"
#include "logic/notification.h"
#include "logic/player_area.h"

namespace UI {struct ProgressWindow;}
struct Fullscreen_Menu_LaunchGame;
struct Interactive_Base;
class LuaInterface;

namespace Widelands {

class Players_Manager;

class Battle;
class Bob;
struct Building_Descr;
class Immovable;
class Map;
struct Object_Manager;
class Player;
struct PlayerImmovable;
struct Tribe_Descr;
struct Flag;
struct AttackController;

class Editor_Game_Base :
	boost::noncopyable,
	NoteReceiver<NoteImmovable>,
	NoteReceiver<NoteFieldPossession>,
	NoteReceiver<NoteFieldTransformed>
{
public:
	friend struct ::Fullscreen_Menu_LaunchGame;
	friend struct ::Interactive_Base;
	friend struct Game_Game_Class_Data_Packet;

	Editor_Game_Base(LuaInterface * lua);
	virtual ~Editor_Game_Base();

	void set_map(Map *);
	Map & map() const {return *m_map;}
	Map * get_map() {return m_map;}
	Map & get_map() const {return *m_map;}
	const Object_Manager & objects() const {return m_objects;}
	Object_Manager       & objects()       {return m_objects;}

	// logic handler func
	virtual void think();

	// Player commands
	void remove_player(Player_Number);
	Player * add_player
		(Player_Number,
		 uint8_t             initialization_index,
		 const std::string & tribe,
		 const std::string & name,
		 TeamNumber team = 0);
	Player * get_player(int32_t n) const;
	Player & player(int32_t n) const;
	virtual Player * get_safe_player(Player_Number);

	// loading stuff
	void allocate_player_maps();
	virtual void postload();
	void load_graphics(UI::ProgressWindow & loader_ui);
	virtual void cleanup_for_load();

	void set_road(FCoords, uint8_t direction, uint8_t roadtype);

	// warping stuff. instantly creating map_objects
	Building & warp_building
		(Coords, Player_Number, Building_Index,
		Building::FormerBuildings former_buildings = Building::FormerBuildings());
	Building & warp_constructionsite
		(Coords, Player_Number, Building_Index, bool loading = false,
		 Building::FormerBuildings former_buildings = Building::FormerBuildings());
	Building & warp_dismantlesite
		(Coords, Player_Number, bool loading = false,
		Building::FormerBuildings former_buildings = Building::FormerBuildings());
	Bob & create_bob(Coords, const BobDescr &, Player * owner = nullptr);
	Bob & create_bob
		(Coords, BobDescr::Index, Tribe_Descr const * const = nullptr, Player * owner = nullptr);
	Bob & create_bob
		(Coords, const std::string & name, Tribe_Descr const * const = nullptr, Player * owner = nullptr);
	Immovable & create_immovable(Coords, uint32_t idx, Tribe_Descr const *);
	Immovable & create_immovable
		(Coords, const std::string & name, Tribe_Descr const *);

	int32_t get_gametime() const {return m_gametime;}
	Interactive_Base * get_ibase() const {return m_ibase;}

	// safe system for storing pointers to non-Map_Object C++ objects
	// unlike objects in the Object_Manager, these pointers need not be
	// synchronized across the network, and they are not saved in savegames
	uint32_t add_trackpointer(void *);
	void * get_trackpointer(uint32_t serial);
	void remove_trackpointer(uint32_t serial);

	// Manually load a tribe into memory. Used by the editor
	const Tribe_Descr & manually_load_tribe(const std::string & tribe);
	const Tribe_Descr & manually_load_tribe(Player_Number const p) {
		return manually_load_tribe(map().get_scenario_player_tribe(p));
	}
	// Get a tribe from the loaded list, when available
	Tribe_Descr const * get_tribe(const char * tribe) const;
	Tribe_Descr const * get_tribe(const std::string & name) const {
		return get_tribe(name.c_str());
	}

	void inform_players_about_ownership(Map_Index, Player_Number);
	void inform_players_about_immovable(Map_Index, Map_Object_Descr const *);
	void inform_players_about_road     (FCoords,   Map_Object_Descr const *);

	void unconquer_area
		(Player_Area<Area<FCoords> >, Player_Number destroying_player = 0);
	void   conquer_area            (Player_Area<Area<FCoords> >);
	void   conquer_area_no_building(Player_Area<Area<FCoords> > const);

	void receive(const NoteImmovable &) override;
	void receive(const NoteFieldPossession     &) override;
	void receive(const NoteFieldTransformed    &) override;

	void cleanup_objects() {
		objects().cleanup(*this);
	}

	// next function is used to update the current gametime,
	// for queue runs e.g.
	int32_t & get_game_time_pointer() {return m_gametime;}
	void set_ibase(Interactive_Base * const b) {m_ibase = b;}

	/// Lua frontend, used to run Lua scripts
	LuaInterface & lua() {return *m_lua;}

	Players_Manager* player_manager() {return m_player_manager.get();}

	Interactive_GameBase * get_igbase();

private:
	// FIXME -- SDL returns time as uint32. Why do I have int32 ? Please comment or change this to uint32.
	int32_t m_gametime;
	Object_Manager             m_objects;

	LuaInterface             * m_lua;
	std::unique_ptr<Players_Manager> m_player_manager;

protected:
	typedef std::vector<Tribe_Descr *> Tribe_Vector;
	Tribe_Vector           m_tribes;
private:
	Interactive_Base         * m_ibase;
	Map                      * m_map;

	uint32_t                       m_lasttrackserial;
	std::map<uint32_t, void *>     m_trackpointers;

private:
	/// \param preferred_player
	///  When conquer is false, this can be used to prefer a player over other
	///  players, when lost land is reassigned. This can for example be used to
	///  reward the player who actually destroyed a MilitarySite by giving an
	///  unconquered location that he has influence over to him, instead of some
	///  other player who has higher influence over that location. If 0, land is
	///  simply assigned by influence.
	///
	/// \param neutral_when_no_influence
	///  If true and the player completely loses influence over a location, it
	///  becomes neutral unless some other player claims it by having positive
	///  influence.
	///
	/// \param neutral_when_competing_influence
	///  If true and the player completely loses influence over a location and
	///  several players have positive and equal influence, the location becomes
	///  becomes neutral unless some other player claims it by having higher
	///  influence.
	///
	/// \param conquer_guarded_location_by_superior_influence
	///  If true, the conquering player will (automatically, without actually
	///  attacking) conquer a location even if another player already owns and
	///  covers the location with a militarysite, if the conquering player's
	///  influence becomes greater than the owner's influence.
	virtual void do_conquer_area
		(Player_Area<Area<FCoords> > player_area,
		 bool          conquer,
		 Player_Number preferred_player                               = 0,
		 bool          neutral_when_no_influence                      = false,
		 bool          neutral_when_competing_influence               = false,
		 bool          conquer_guarded_location_by_superior_influence = false);
	void cleanup_playerimmovables_area(Player_Area<Area<FCoords> >);
};

#define iterate_players_existing(p, nr_players, egbase, player)               \
   iterate_player_numbers(p, nr_players)                                      \
      if (Widelands::Player * const player = (egbase).get_player(p))          \

#define iterate_players_existing_novar(p, nr_players, egbase)                 \
   iterate_player_numbers(p, nr_players)                                      \
      if ((egbase).get_player(p))                                             \

#define iterate_players_existing_const(p, nr_players, egbase, player)         \
   iterate_player_numbers(p, nr_players)                                      \
      if (Widelands::Player const * const player = (egbase).get_player(p))    \

}

#endif
