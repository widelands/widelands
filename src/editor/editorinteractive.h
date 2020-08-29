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

#ifndef WL_EDITOR_EDITORINTERACTIVE_H
#define WL_EDITOR_EDITORINTERACTIVE_H

#include <map>
#include <memory>

#include "editor/editor_category.h"
#include "editor/scripting/function_statements.h"
#include "editor/tools/history.h"
#include "editor/tools/increase_height_tool.h"
#include "editor/tools/increase_resources_tool.h"
#include "editor/tools/info_tool.h"
#include "editor/tools/noise_height_tool.h"
#include "editor/tools/place_critter_tool.h"
#include "editor/tools/place_immovable_tool.h"
#include "editor/tools/resize_tool.h"
#include "editor/tools/scenario_field_owner_tool.h"
#include "editor/tools/scenario_infrastructure_settings_tool.h"
#include "editor/tools/scenario_infrastructure_tool.h"
#include "editor/tools/scenario_road_tool.h"
#include "editor/tools/scenario_vision_tool.h"
#include "editor/tools/scenario_worker_tool.h"
#include "editor/tools/set_origin_tool.h"
#include "editor/tools/set_port_space_tool.h"
#include "editor/tools/set_starting_pos_tool.h"
#include "editor/tools/set_terrain_tool.h"
#include "logic/map.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_base.h"

class FileWrite;
class EditorTool;
namespace Widelands {
class MapScenarioEditorPacket;
}

/**
 * This is the EditorInteractive. It is like the InteractivePlayer class,
 * but for the Editor instead of the game
 */
class EditorInteractive : public InteractiveBase {
	friend class Widelands::MapScenarioEditorPacket;

public:
	struct Tools {
		Tools(const Widelands::Map& map)
		   : current_pointer(&info),
		     use_tool(EditorTool::First),
		     increase_height(decrease_height, set_height),
		     noise_height(set_height),
		     place_immovable(delete_immovable),
		     place_critter(delete_critter),
		     increase_resources(decrease_resources, set_resources),
		     set_port_space(unset_port_space),
		     set_origin(),
		     resize(map.get_width(), map.get_height()),
		     sc_owner(),
		     sc_vision(),
		     sc_infra_del(),
		     sc_infra(sc_infra_del),
		     sc_infra_settings(),
		     sc_road_del(),
		     sc_road(sc_road_del),
		     sc_worker_del(),
		     sc_worker(sc_worker_del) {
		}
		EditorTool& current() const {
			return *current_pointer;
		}
		using ToolVector = std::vector<EditorTool*>;
		EditorTool* current_pointer;
		EditorTool::ToolIndex use_tool;
		EditorInfoTool info;
		EditorSetHeightTool set_height;
		EditorDecreaseHeightTool decrease_height;
		EditorIncreaseHeightTool increase_height;
		EditorNoiseHeightTool noise_height;
		EditorSetTerrainTool set_terrain;
		EditorDeleteImmovableTool delete_immovable;
		EditorPlaceImmovableTool place_immovable;
		EditorSetStartingPosTool set_starting_pos;
		EditorDeleteCritterTool delete_critter;
		EditorPlaceCritterTool place_critter;
		EditorDecreaseResourcesTool decrease_resources;
		EditorSetResourcesTool set_resources;
		EditorIncreaseResourcesTool increase_resources;
		EditorSetPortSpaceTool set_port_space;
		EditorUnsetPortSpaceTool unset_port_space;
		EditorSetOriginTool set_origin;
		EditorResizeTool resize;

		ScenarioFieldOwnerTool sc_owner;
		ScenarioVisionTool sc_vision;
		ScenarioInfrastructureDeleteTool sc_infra_del;
		ScenarioInfrastructureTool sc_infra;
		ScenarioInfrastructureSettingsTool sc_infra_settings;
		ScenarioDeleteRoadTool sc_road_del;
		ScenarioPlaceRoadTool sc_road;
		ScenarioDeleteWorkerTool sc_worker_del;
		ScenarioPlaceWorkerTool sc_worker;
	};
	explicit EditorInteractive(Widelands::EditorGameBase&);
	~EditorInteractive() override {
	}

	// Runs the Editor via the commandline --editor flag. Will load 'filename' as a
	// map and run 'script_to_run' directly after all initialization is done.
	static void run_editor(const std::string& filename, const std::string& script_to_run);

	void load(const std::string& filename);
	void cleanup_for_load() override;

	// leaf functions from base class
	void start() override;
	void think() override;

	void map_clicked(const Widelands::NodeAndTriangle<>& node_and_triangle, bool draw);
	void set_sel_pos(Widelands::NodeAndTriangle<>) override;
	void set_sel_radius_and_update_menu(uint32_t);
	void stop_painting();

	//  Handle UI elements.
	bool handle_key(bool down, SDL_Keysym) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;
	void draw(RenderTarget&) override;

	void select_tool(EditorTool&, EditorTool::ToolIndex);

	Widelands::Player* get_player() const override {
		return nullptr;
	}

	bool omnipotent() const override {
		return true;
	}

	// action functions
	void exit();

	void set_need_save(bool const t) {
		need_save_ = t;
	}

	// Signalizes that the egbase().map has changed. This can happen when a new
	// map is created or loaded, in which case all windows should be closed and
	// all tools should be reset. Otherwise, something else happened that
	// requires the UI to be completely recalculated, for example the origin of
	// the map has changed.
	enum class MapWas {
		kGloballyMutated,
		kReplaced,
	};
	void map_changed(const MapWas& action);

	// Access to the tools.
	Tools* tools();

	// Player vision tool
	Widelands::PlayerNumber get_illustrating_vision_for() const {
		return illustrating_vision_for_;
	}
	void set_illustrating_vision_for(Widelands::PlayerNumber p) {
		assert(finalized_);
		illustrating_vision_for_ = p;
	}

	// Scripting access
	bool finalized() const {
		return finalized_;
	}
	void show_allowed_buildings_window(Widelands::PlayerNumber p) {
		assert(p);
		assert(p <= allowed_buildings_windows_.size());
		allowed_buildings_windows_[p - 1]->create();
	}

	void write_lua(FileWrite&) const;

	ScriptingSaver& scripting_saver() const {
		return *scripting_saver_;
	}

	const std::list<FS_LocalVarDeclOrAssign*>& variables() const {
		return variables_;
	}
	std::list<FS_LocalVarDeclOrAssign*>& variables() {
		return variables_;
	}
	const std::list<FS_LaunchCoroutine*>& functions() const {
		return functions_;
	}
	std::list<FS_LaunchCoroutine*>& functions() {
		return functions_;
	}
	const std::list<std::string>& includes() const {
		return includes_;
	}
	std::list<std::string>& includes() {
		return includes_;
	}

	/// Access to the editor categories
	const std::vector<std::unique_ptr<EditorCategory>>&
	editor_categories(Widelands::MapObjectType type) const;

private:
	// For referencing the items in mainmenu_
	enum class MainMenuEntry {
		kNewMap,
		kNewRandomMap,
		kLoadMap,
		kSaveMap,
		kMapOptions,
		kExitEditor,
	};

	// For referencing the items in toolmenu_
	enum class ToolMenuEntry {
		kChangeHeight,
		kRandomHeight,
		kTerrain,
		kImmovables,
		kAnimals,
		kResources,
		kPortSpace,
		kPlayers,
		kMapOrigin,
		kMapSize,
		kFieldInfo
	};
	// For referencing the items in scenario_toolmenu_
	enum class ScenarioToolMenuEntry {
		kFinalize,
		kFieldOwner,
		kVision,
		kInfrastructure,
		kInfrastructureSettings,
		kWorker,
		kRoad,
		kLua,
	};

	// For referencing the items in showhidemenu_
	enum class ShowHideEntry {
		kBuildingSpaces,
		kGrid,
		kAnimals,
		kImmovables,
		kResources,
		kCensus,
		kOwnership
	};

	// Adds the mainmenu_ to the toolbar
	void add_main_menu();
	// Takes the appropriate action when an item in the mainmenu_ is selected
	void main_menu_selected(MainMenuEntry entry);
	// Adds the toolmenu_ to the toolbar
	void add_tool_menu();
	// Adds the scenario_toolmenu_ to the toolbar
	void add_scenario_tool_menu();
	void rebuild_scenario_tool_menu();
	// Takes the appropriate action when an item in the toolmenu_ is selected
	void tool_menu_selected(ToolMenuEntry entry);
	void scenario_tool_menu_selected(ScenarioToolMenuEntry entry);

	// Adds the showhidemenu_ to the toolbar
	void add_showhide_menu();
	void rebuild_showhide_menu() override;
	// Takes the appropriate action when an item in the showhidemenu_ is selected
	void showhide_menu_selected(ShowHideEntry entry);

	bool player_hears_field(const Widelands::Coords& coords) const override;

	// Show / hide the resources overlays in the mapview
	void toggle_resources();
	// Show / hide the immovables in the mapview
	void toggle_immovables();
	// Show / hide the bobs in the mapview
	void toggle_bobs();
	void toggle_grid();

	/// Ensure all world units have been loaded and fill editor categories
	void load_world_units();

	//  state variables
	bool need_save_;
	uint32_t realtime_;
	bool is_painting_;

	bool finalized_;

	// Returns the reason why finalizing failed, or "" on success
	std::string try_finalize();
	void finalize_clicked();

	// All unique menu windows
	struct EditorMenuWindows {
		UI::UniqueWindow::Registry newmap;
		UI::UniqueWindow::Registry newrandommap;
		UI::UniqueWindow::Registry savemap;
		UI::UniqueWindow::Registry loadmap;
		UI::UniqueWindow::Registry mapoptions;
		UI::UniqueWindow::Registry scenarioctrl;

		UI::UniqueWindow::Registry toolsize;

		UI::UniqueWindow::Registry help;
	} menu_windows_;

	// All unique tool windows for those tools that have them
	struct EditorToolWindows {
		UI::UniqueWindow::Registry height;
		UI::UniqueWindow::Registry noiseheight;
		UI::UniqueWindow::Registry terrain;
		UI::UniqueWindow::Registry immovables;
		UI::UniqueWindow::Registry critters;
		UI::UniqueWindow::Registry resources;
		UI::UniqueWindow::Registry players;
		UI::UniqueWindow::Registry resizemap;
	} tool_windows_;
	struct ScenarioToolWindows {
		UI::UniqueWindow::Registry fieldowner;
		UI::UniqueWindow::Registry vision;
		UI::UniqueWindow::Registry infrastructure;
		UI::UniqueWindow::Registry worker;
		UI::UniqueWindow::Registry road;
		UI::UniqueWindow::Registry lua;
	} scenario_tool_windows_;

	std::vector<std::unique_ptr<UI::UniqueWindow::Registry>> allowed_buildings_windows_;
	void init_allowed_buildings_windows_registries();
	Widelands::PlayerNumber illustrating_vision_for_;

	void unfinalize();

	std::map<Widelands::MapObjectType, std::vector<std::unique_ptr<EditorCategory>>>
	   editor_categories_;

	// Main menu on the toolbar
	UI::Dropdown<MainMenuEntry> mainmenu_;
	// Tools menu on the toolbar
	UI::Dropdown<ToolMenuEntry> toolmenu_;
	UI::Dropdown<ScenarioToolMenuEntry> scenario_toolmenu_;
	// Show / Hide menu on the toolbar
	UI::Dropdown<ShowHideEntry> showhidemenu_;

	UI::Button* undo_;
	UI::Button* redo_;

	std::unique_ptr<Tools> tools_;
	std::unique_ptr<EditorHistory> history_;

	std::list<FS_LocalVarDeclOrAssign*> variables_;
	std::list<FS_LaunchCoroutine*> functions_;
	std::list<std::string> includes_;
	std::unique_ptr<ScriptingSaver> scripting_saver_;

	void new_scripting_saver() {
		scripting_saver_.reset(new ScriptingSaver());
	}

	bool draw_resources_ = true;
	bool draw_immovables_ = true;
	bool draw_bobs_ = true;
	bool draw_grid_ = true;

	Workareas ownership_layer_cache_;

	void update_players();
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteEditorPlayerEdited>> player_notes_;
};

#endif  // end of include guard: WL_EDITOR_EDITORINTERACTIVE_H
