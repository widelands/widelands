/*
 * Copyright (C) 2002, 2006-2008, 2011, 2015 by the Widelands Development Team
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

#include <memory>

#include "editor/tools/history.h"
#include "editor/tools/increase_height_tool.h"
#include "editor/tools/increase_resources_tool.h"
#include "editor/tools/info_tool.h"
#include "editor/tools/make_infrastructure_tool.h"
#include "editor/tools/noise_height_tool.h"
#include "editor/tools/place_bob_tool.h"
#include "editor/tools/place_immovable_tool.h"
#include "editor/tools/set_origin_tool.h"
#include "editor/tools/set_port_space_tool.h"
#include "editor/tools/set_starting_pos_tool.h"
#include "editor/tools/set_terrain_tool.h"
#include "logic/map.h"
#include "notifications/notifications.h"
#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_base.h"

class Editor;
class EditorTool;

/**
 * This is the EditorInteractive. It is like the InteractivePlayer class,
 * but for the Editor instead of the game
 */
class EditorInteractive : public InteractiveBase {
public:
	struct Tools {
		Tools()
			:
			current_pointer(&info),
			use_tool(EditorTool::First),
			increase_height(decrease_height, set_height),
			noise_height(set_height),
			place_immovable(delete_immovable),
			place_bob(delete_bob),
			increase_resources(decrease_resources, set_resources),
			set_port_space(unset_port_space),
			set_origin()
		{}
		EditorTool & current() const {return *current_pointer;}
		using ToolVector = std::vector<EditorTool *>;
		EditorTool          *        current_pointer;
		EditorTool::ToolIndex        use_tool;
		EditorInfoTool               info;
		EditorSetHeightTool          set_height;
		EditorDecreaseHeightTool     decrease_height;
		EditorIncreaseHeightTool     increase_height;
		EditorNoiseHeightTool        noise_height;
		EditorSetTerrainTool         set_terrain;
		EditorDeleteImmovableTool    delete_immovable;
		EditorPlaceImmovableTool     place_immovable;
		EditorSetStartingPosTool     set_starting_pos;
		EditorDeleteBobTool          delete_bob;
		EditorPlaceBobTool           place_bob;
		EditorDecreaseResourcesTool  decrease_resources;
		EditorSetResourcesTool       set_resources;
		EditorIncreaseResourcesTool  increase_resources;
		EditorSetPortSpaceTool       set_port_space;
		EditorUnsetPortSpaceTool     unset_port_space;
		EditorSetOriginTool          set_origin;
		EditorMakeInfrastructureTool make_infrastructure;
	};
	EditorInteractive(Widelands::EditorGameBase &);

	// Runs the Editor via the commandline --editor flag. Will load 'filename' as a
	// map and run 'script_to_run' directly after all initialization is done.
	static void run_editor(const std::string & filename, const std::string& script_to_run);

	void load(const std::string & filename);

	// leaf functions from base class
	void start() override;
	void think() override;

	void map_clicked(bool draw = false);
	void set_sel_pos(Widelands::NodeAndTriangle<>) override;
	void set_sel_radius_and_update_menu(uint32_t);
	void start_painting();
	void stop_painting();

	//  Handle UI elements.
	bool handle_key(bool down, SDL_Keysym) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;


	void select_tool(EditorTool &, EditorTool::ToolIndex);

	Widelands::Player * get_player() const override {return nullptr;}

	// action functions
	void exit();

	//  reference functions
	void   reference_player_tribe(Widelands::PlayerNumber, void const * const) override;
	void unreference_player_tribe(Widelands::PlayerNumber, void const * const);
	bool is_player_tribe_referenced(Widelands::PlayerNumber);
	void set_need_save(bool const t) {need_save_ = t;}

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

	UI::UniqueWindow::Registry window_help;

private:
	friend struct EditorToolMenu;

	struct PlayerReferences {
		int32_t      player;
		void const * object;
	};

	// Registers the overlays for player starting positions.
	void register_overlays();

	void tool_menu_btn();
	void toolsize_menu_btn();
	void toggle_mainmenu();
	void toggle_playermenu();
	void toggle_help();

	//  state variables
	bool need_save_;
	std::vector<PlayerReferences> player_tribe_references_;
	uint32_t realtime_;
	bool is_painting_;

	std::unique_ptr<Tools> tools_;
	std::unique_ptr<EditorHistory> history_;

	std::unique_ptr<Notifications::Subscriber<Widelands::NoteFieldResourceChanged>>
	   field_resource_changed_subscriber_;
	UI::UniqueWindow::Registry toolmenu_;

	UI::UniqueWindow::Registry toolsizemenu_;
	UI::UniqueWindow::Registry playermenu_;
	UI::UniqueWindow::Registry mainmenu_;
	UI::UniqueWindow::Registry heightmenu_;
	UI::UniqueWindow::Registry noise_heightmenu_;
	UI::UniqueWindow::Registry terrainmenu_;
	UI::UniqueWindow::Registry immovablemenu_;
	UI::UniqueWindow::Registry bobmenu_;
	UI::UniqueWindow::Registry resourcesmenu_;
	UI::UniqueWindow::Registry helpmenu_;

	UI::Button toggle_main_menu_;
	UI::Button toggle_tool_menu_;
	UI::Button toggle_toolsize_menu_;
	UI::Button toggle_minimap_;
	UI::Button toggle_buildhelp_;
	UI::Button toggle_player_menu_;
	UI::Button undo_;
	UI::Button redo_;
	UI::Button toggle_help_;
};

#endif  // end of include guard: WL_EDITOR_EDITORINTERACTIVE_H
