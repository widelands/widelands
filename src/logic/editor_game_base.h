/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef EDITOR_GAME_BASE_H
#define EDITOR_GAME_BASE_H

#include "bob.h"
#include "building.h"
#include "constants.h"
#include "map.h"
#include "notification.h"
#include "player_area.h"

#include <string>
#include <cstring>
#include <vector>

namespace UI {struct ProgressWindow;}
struct Fullscreen_Menu_LaunchGame;
struct Interactive_Base;

namespace Widelands {

struct AreaWatcher;
struct Battle;
class Bob;
struct Building_Descr;
class Immovable;
struct Map;
struct Object_Manager;
struct Player;
struct PlayerImmovable;
struct Tribe_Descr;
class Flag;
struct AttackController;

struct Editor_Game_Base : NoteReceiver<NoteImmovable>, NoteReceiver<NoteField>
{
	friend struct ::Fullscreen_Menu_LaunchGame;
	friend struct ::Interactive_Base;
	friend class Game_Game_Class_Data_Packet;

	Editor_Game_Base();
	virtual ~Editor_Game_Base();

	void set_map(Map *, bool = true);
	Map & map() const throw () {return *m_map;}
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
		 const std::string & name);
	Player * get_player(const int32_t n) const {
		assert(1 <= n);
		assert     (n <= MAX_PLAYERS);
		return m_players[n - 1];
	}
	Player & player(const int32_t n) const {
		assert(1 <= n);
		assert     (n <= MAX_PLAYERS);
		return *m_players[n - 1];
	}
	virtual Player * get_safe_player(Player_Number);

	// loading stuff
	void allocate_player_maps();
	virtual void postload();
	void load_graphics(UI::ProgressWindow & loader_ui);
	virtual void cleanup_for_load
		(bool flush_graphics = true, bool flush_animations = true);

	void set_road(FCoords, uint8_t direction, uint8_t roadtype);

	// warping stuff. instantly creating map_objects
	Building & warp_building(Coords, Player_Number, Building_Index);
	Building & warp_constructionsite
		(Coords, Player_Number, Building_Index,
		 Building_Index oldid = Building_Index::Null());
	Bob & create_bob(Coords, Bob::Descr::Index, Tribe_Descr const * const = 0);
	Immovable & create_immovable(Coords, int32_t idx, Tribe_Descr const *);
	Immovable & create_immovable
		(Coords, std::string const & name, Tribe_Descr const *);

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
	Tribe_Descr const & manually_load_tribe(Player_Number const p) {
		return manually_load_tribe(map().get_scenario_player_tribe(p));
	}
	// Get a tribe from the loaded list, when available
	Tribe_Descr const * get_tribe(const char * tribe) const;

	void inform_players_about_ownership(Map_Index, Player_Number);
	void inform_players_about_immovable(Map_Index, Map_Object_Descr const *);
	void inform_players_about_road     (FCoords,   Map_Object_Descr const *);

	void receive(NoteImmovable const &);
	void receive(NoteField     const &);

	void cleanup_objects() throw () {
		objects().cleanup(*this);
	}

	// next function is used to update the current gametime,
	// for queue runs e.g.
	int32_t & get_game_time_pointer() {return m_gametime;}
	void set_ibase(Interactive_Base * const b) {m_ibase = b;}

private:
	int32_t m_gametime;
	Player                   * m_players[MAX_PLAYERS];
	Object_Manager             m_objects;
protected:
	typedef std::vector<Tribe_Descr *> Tribe_Vector;
	Tribe_Vector           m_tribes;
private:
	Interactive_Base         * m_ibase;
	Map                      * m_map;

	uint32_t                       m_lasttrackserial;
	std::map<uint32_t, void *>     m_trackpointers;

private:
	Editor_Game_Base & operator= (Editor_Game_Base const &);
	explicit Editor_Game_Base    (Editor_Game_Base const &);
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

extern const uint8_t g_playercolors[MAX_PLAYERS][12];

}

#endif
