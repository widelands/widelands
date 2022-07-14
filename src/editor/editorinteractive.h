/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_EDITOR_EDITORINTERACTIVE_H
#define WL_EDITOR_EDITORINTERACTIVE_H

#include <map>
#include <memory>

#include "editor/editor_category.h"
#include "editor/tools/history.h"
#include "editor/tools/increase_height_tool.h"
#include "editor/tools/increase_resources_tool.h"
#include "editor/tools/info_tool.h"
#include "editor/tools/noise_height_tool.h"
#include "editor/tools/place_critter_tool.h"
#include "editor/tools/place_immovable_tool.h"
#include "editor/tools/resize_tool.h"
#include "editor/tools/set_origin_tool.h"
#include "editor/tools/set_port_space_tool.h"
#include "editor/tools/set_starting_pos_tool.h"
#include "editor/tools/set_terrain_tool.h"
#include "editor/tools/toolhistory_tool.h"
#include "logic/map.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_base.h"

class EditorTool;

std::string editor_splash_image();

/**
 * This is the EditorInteractive. It is like the InteractivePlayer class,
 * but for the Editor instead of the game
 */
class EditorInteractive : public InteractiveBase {
public:
	struct Tools {
		explicit Tools(EditorInteractive& parent, const Widelands::Map& map)
		   : current_pointer(&info),
		     use_tool(EditorTool::First),
		     info(parent),
		     set_height(parent),
		     decrease_height(parent),
		     increase_height(parent, decrease_height, set_height),
		     noise_height(parent, set_height),
		     set_terrain(parent),
		     delete_immovable(parent),
		     place_immovable(parent, delete_immovable),
		     set_starting_pos(parent),
		     delete_critter(parent),
		     place_critter(parent, delete_critter),
		     decrease_resources(parent),
		     set_resources(parent),
		     increase_resources(parent, decrease_resources, set_resources),
		     unset_port_space(parent),
		     set_port_space(parent, unset_port_space),
		     set_origin(parent),
		     resize(parent, map.get_width(), map.get_height()),
		     tool_history(parent) {
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
		EditorUnsetPortSpaceTool unset_port_space;
		EditorSetPortSpaceTool set_port_space;
		EditorSetOriginTool set_origin;
		EditorResizeTool resize;
		EditorHistoryTool tool_history;
	};
	explicit EditorInteractive(Widelands::EditorGameBase&);

	enum class Init {
		kLoadMapDirectly,  // load the given map file, then run the given script if any
		kDefault,          // create new empty map
		kNew,              // show New Map window
		kRandom,           // show Random Map window
		kLoad              // show Load Map window
	};
	static void run_editor(UI::Panel* error_message_parent,
	                       EditorInteractive::Init,
	                       const std::string& filename = "",
	                       const std::string& script_to_run = "");

	void load(const std::string& filename);
	void cleanup_for_load() override;

	// leaf functions from base class
	void start() override;
	void think() override;

	void map_clicked(const Widelands::NodeAndTriangle<>& node_and_triangle, bool draw);
	void set_sel_pos(Widelands::NodeAndTriangle<>) override;
	void set_sel_radius(uint32_t) override;
	void set_sel_radius_and_update_menu(uint32_t);
	void stop_painting();

	//  Handle UI elements.
	bool handle_key(bool down, SDL_Keysym) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;
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
	enum class MapWas { kGloballyMutated, kReplaced, kResized };
	void map_changed(const MapWas& action);

	// Access to the tools.
	Tools* tools();

	/// Access to the editor categories
	const std::vector<std::unique_ptr<EditorCategory>>&
	editor_categories(Widelands::MapObjectType type) const;

	/// Ensure all world units have been loaded and fill editor categories
	static void load_world_units(EditorInteractive*, Widelands::EditorGameBase&);

	EditorHistory& history();

	// Returns window for given tool if it's open, otherwise return nullptr
	UI::UniqueWindow* get_open_tool_window(WindowID window_id);
	UI::UniqueWindow::Registry& get_registry_for_window(WindowID window_id);
	void restore_tool_configuration(const ToolConf& conf);

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
		kFieldInfo,
		kToolHistory,
	};

	// For referencing the items in showhidemenu_
	enum class ShowHideEntry {
		kBuildingSpaces,
		kMaximumBuildingSpaces,
		kGrid,
		kAnimals,
		kImmovables,
		kResources
	};

	static void do_run_editor(EditorInteractive::Init, const std::string&, const std::string&);

	// Adds the mainmenu_ to the toolbar
	void add_main_menu();
	// Takes the appropriate action when an item in the mainmenu_ is selected
	void main_menu_selected(MainMenuEntry entry);
	// Adds the toolmenu_ to the toolbar
	void add_tool_menu();
	// Takes the appropriate action when an item in the toolmenu_ is selected
	void tool_menu_selected(ToolMenuEntry entry);

	// Adds the showhidemenu_ to the toolbar
	void add_showhide_menu();
	void rebuild_showhide_menu() override;
	// Takes the appropriate action when an item in the showhidemenu_ is selected
	void showhide_menu_selected(ShowHideEntry entry);

	bool player_hears_field(const Widelands::Coords& coords) const override;

	// Toggles the buildhelp for maximum building spaces and calls rebuild_showhide_menu
	void toggle_maximum_buildhelp();
	// Show / hide the resources overlays in the mapview
	void toggle_resources();
	// Show / hide the immovables in the mapview
	void toggle_immovables();
	// Show / hide the bobs in the mapview
	void toggle_bobs();
	void toggle_grid();

	void update_tool_history_window();

	//  state variables
	bool need_save_;
	uint32_t realtime_;
	bool is_painting_;

	// All unique menu windows
	struct EditorMenuWindows {
		UI::UniqueWindow::Registry newmap;
		UI::UniqueWindow::Registry newrandommap;
		UI::UniqueWindow::Registry savemap;
		UI::UniqueWindow::Registry loadmap;
		UI::UniqueWindow::Registry mapoptions;

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
		UI::UniqueWindow::Registry toolhistory;
	} tool_windows_;

	std::map<Widelands::MapObjectType, std::vector<std::unique_ptr<EditorCategory>>>
	   editor_categories_;

	// Main menu on the toolbar
	UI::Dropdown<MainMenuEntry> mainmenu_;
	// Tools menu on the toolbar
	UI::Dropdown<ToolMenuEntry> toolmenu_;
	// Show / Hide menu on the toolbar
	UI::Dropdown<ShowHideEntry> showhidemenu_;

	UI::Button* undo_;
	UI::Button* redo_;

	std::unique_ptr<Tools> tools_;
	std::unique_ptr<EditorHistory> history_;

	bool cleaning_up_ = false;
	UI::UniqueWindow::Registry* registry_to_open_ = nullptr;

	// Mapping between tools_ and tool_windows_
	std::map<EditorTool*, UI::UniqueWindow::Registry*> tool_to_window_map_;

	/// Set to true when tool settings are changed in tool window.
	/// Set to false when the tool is used with the new settings.
	bool tool_settings_changed_ = true;
};

#endif  // end of include guard: WL_EDITOR_EDITORINTERACTIVE_H
