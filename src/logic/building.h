/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#ifndef WL_LOGIC_BUILDING_H
#define WL_LOGIC_BUILDING_H

#include <cstring>
#include <string>
#include <vector>

#include <boost/signals2.hpp>

#include "ai/ai_hints.h"
#include "base/macros.h"
#include "logic/bill_of_materials.h"
#include "logic/buildcost.h"
#include "logic/immovable.h"
#include "logic/message.h"
#include "logic/soldier_counts.h"
#include "logic/wareworker.h"
#include "logic/widelands.h"
#include "logic/workarea_info.h"
#include "scripting/lua_table.h"

namespace UI {class Window;}
struct BuildingHints;
class InteractiveGameBase;
class Profile;
class Image;

namespace Widelands {

struct Flag;
struct Message;
class TribeDescr;
class WaresQueue;

class Building;

#define LOW_PRIORITY             2
#define DEFAULT_PRIORITY         4
#define HIGH_PRIORITY            8

/*
 * Common to all buildings!
 */
class BuildingDescr : public MapObjectDescr {
public:
	using FormerBuildings = std::vector<BuildingIndex>;

	BuildingDescr(const std::string& init_descname, MapObjectType type,
					  const LuaTable& t, const EditorGameBase& egbase);
	~BuildingDescr() override {}

	bool is_buildable   () const {return m_buildable;}
	bool is_destructible() const {return m_destructible;}
	bool is_enhanced    () const {return m_enhanced_building;}

	/**
	 * The build cost for direct construction
	 */
	const Buildcost & buildcost() const {return m_buildcost;}

	/**
	 * Returned wares for dismantling
	 */
	const Buildcost & returned_wares() const {return m_return_dismantle;}

	/**
	 * The build cost for enhancing a previous building
	 */
	const Buildcost & enhancement_cost() const {return m_enhance_cost;}

	/**
	 * The returned wares for a enhaced building
	 */
	const Buildcost & returned_wares_enhanced() const {return m_return_enhanced;}

	std::string helptext_script() const {return helptext_script_;}
	int32_t get_size() const {return m_size;}
	bool get_ismine() const {return m_mine;}
	bool get_isport() const {return m_port;}

	// Returns the enhancement this building can become or
	// INVALID_INDEX if it cannot be enhanced.
	const BuildingIndex & enhancement() const {return m_enhancement;}
	// Returns the building from which this building can be enhanced or
	// INVALID_INDEX if it cannot be built as an enhanced building.
	const BuildingIndex& enhanced_from() const {return m_enhanced_from;}
	void set_enhanced_from(const BuildingIndex& index) {m_enhanced_from = index;}

	/// Create a building of this type in the game. Calls init, which does
	/// different things for different types of buildings (such as conquering
	/// land and requesting things). Therefore this must not be used to allocate
	/// a building during savegame loading. (It would cause many bugs.)
	///
	/// Does not perform any sanity checks.
	/// If former_buildings is not empty this is an enhancing.
	Building & create
		(EditorGameBase &,
		 Player &,
		 Coords,
		 bool                   construct,
		 bool                   loading = false,
		 FormerBuildings former_buildings = FormerBuildings())
		const;

	virtual uint32_t get_conquers() const;
	virtual uint32_t vision_range() const;

	WorkareaInfo m_workarea_info;

	virtual int32_t suitability(const Map &, FCoords) const;
	const BuildingHints & hints() const {return m_hints;}

protected:
	virtual Building & create_object() const = 0;
	Building & create_constructionsite() const;

private:
	const EditorGameBase& egbase_;
	bool          m_buildable;       // the player can build this himself
	bool          m_destructible;    // the player can destruct this himself
	Buildcost     m_buildcost;
	Buildcost     m_return_dismantle; // Returned wares on dismantle
	Buildcost     m_enhance_cost;     // cost for enhancing
	Buildcost     m_return_enhanced;   // Returned ware for dismantling an enhanced building
	std::string   helptext_script_;   // The path and filename to the building's helptext script
	int32_t       m_size;            // size of the building
	bool          m_mine;
	bool          m_port;
	BuildingIndex  m_enhancement;
	BuildingIndex  m_enhanced_from; // The building this building was enhanced from, or INVALID_INDEX
	bool          m_enhanced_building; // if it is one, it is bulldozable
	BuildingHints m_hints;             // hints (knowledge) for computer players

	// for migration, 0 is the default, meaning get_conquers() + 4
	uint32_t m_vision_range;
	DISALLOW_COPY_AND_ASSIGN(BuildingDescr);
};


class Building : public PlayerImmovable {
	friend class BuildingDescr;
	friend class MapBuildingdataPacket;

	MO_DESCR(BuildingDescr)

public:
	// Player capabilities: which commands can a player issue for this building?
	enum {
		PCap_Bulldoze = 1, // can bulldoze/remove this buildings
		PCap_Dismantle = 1 << 1, // can dismantle this buildings
		PCap_Enhancable = 1 << 2, // can be enhanced to something
	};

	using FormerBuildings = std::vector<BuildingIndex>;

public:
	Building(const BuildingDescr&);
	virtual ~Building();

	void load_finish(EditorGameBase &) override;

	int32_t  get_size    () const override;
	bool get_passable() const override;

	Flag & base_flag() override;
	virtual uint32_t get_playercaps() const;

	virtual Coords get_position() const {return m_position;}
	PositionList get_positions (const EditorGameBase &) const override;

	std::string info_string(const std::string & format);

	// Return the overlay string that is displayed on the map view when enabled
	// by the player.
	const std::string& update_and_get_statistics_string() {
		update_statistics_string(&m_statistics_string);
		return m_statistics_string;
	}

	/// \returns the queue for a ware type or \throws WException.
	virtual WaresQueue & waresqueue(WareIndex);

	virtual bool burn_on_destroy();
	void destroy(EditorGameBase &) override;

	void show_options(InteractiveGameBase &, bool avoid_fastclick = false, Point pos = Point(- 1, - 1));
	void hide_options();
	void refresh_options(InteractiveGameBase &);

	virtual bool fetch_from_flag(Game &);
	virtual bool get_building_work(Game &, Worker &, bool success);

	bool leave_check_and_wait(Game &, Worker &);
	void leave_skip(Game &, Worker &);

	// Get/Set the priority for this waretype for this building. 'type' defines
	// if this is for a worker or a ware, 'index' is the type of worker or ware.
	// If 'adjust' is false, the three possible states HIGH_PRIORITY,
	// DEFAULT_PRIORITY and LOW_PRIORITY are returned, otherwise numerical
	// values adjusted to the preciousness of the ware in general are returned.
	virtual int32_t get_priority
		(WareWorker type, WareIndex, bool adjust = true) const;
	void set_priority(int32_t type, WareIndex ware_index, int32_t new_priority);

	void collect_priorities
		(std::map<int32_t, std::map<WareIndex, int32_t> > & p) const;

	/**
	 * The former buildings vector keeps track of all former buildings
	 * that have been enhanced up to the current one. The current building
	 * index will be in the last position. For construction sites, it is
	 * empty except enhancements. For a dismantle site, the last item will
	 * be the one being dismantled.
	 */
	const FormerBuildings get_former_buildings() {
		return m_old_buildings;
	}

	void log_general_info(const EditorGameBase &) override;

	//  Use on training sites only.
	virtual void change_train_priority(uint32_t, int32_t) {}
	virtual void switch_train_mode () {}

	///  Stores the PlayerNumber of the player who has defeated this building.
	void set_defeating_player(PlayerNumber const player_number) {
		m_defeating_player = player_number;
	}

	void    add_worker(Worker &) override;
	void remove_worker(Worker &) override;
	mutable boost::signals2::signal<void ()> workers_changed;

	void send_message
		(Game & game,
		 const Message::Type msgtype,
		 const std::string & title,
		 const std::string & description,
		 bool link_to_building_lifetime = true,
		 uint32_t throttle_time = 0,
		 uint32_t throttle_radius = 0);

protected:
	// Updates 'statistics_string' with the string that should be displayed for
	// this building right now. Overwritten by child classes.
	virtual void update_statistics_string(std::string*) {
	}

	void start_animation(EditorGameBase &, uint32_t anim);

	void init(EditorGameBase &) override;
	void cleanup(EditorGameBase &) override;
	void act(Game &, uint32_t data) override;

	void draw(const EditorGameBase &, RenderTarget &, const FCoords&, const Point&) override;
	void draw_help(const EditorGameBase &, RenderTarget &, const FCoords&, const Point&);

	virtual void create_options_window
		(InteractiveGameBase &, UI::Window * & registry)
		= 0;

	void set_seeing(bool see);

	UI::Window * m_optionswindow;
	Coords       m_position;
	Flag       * m_flag;

	uint32_t m_anim;
	int32_t  m_animstart;

	using LeaveQueue = std::vector<OPtr<Worker>>;
	LeaveQueue m_leave_queue; //  FIFO queue of workers leaving the building
	uint32_t    m_leave_time;  //  when to wake the next one from leave queue
	ObjectPointer  m_leave_allow; //  worker that is allowed to leave now

	//  The player who has defeated this building.
	PlayerNumber           m_defeating_player;

	int32_t m_priority; // base priority
	std::map<WareIndex, int32_t> m_ware_priorities;

	/// Whether we see our vision_range area based on workers in the building
	bool m_seeing;

	// Signals connected for the option window
	std::vector<boost::signals2::connection> options_window_connections;

	// The former buildings names, with the current one in last position.
	FormerBuildings m_old_buildings;

private:
	std::string m_statistics_string;
};

}

#endif  // end of include guard: WL_LOGIC_BUILDING_H
