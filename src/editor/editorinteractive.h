/*
 * Copyright (C) 2002, 2006-2008, 2011 by the Widelands Development Team
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

#include "editor/tools/editor_history.h"
#include "editor/tools/editor_increase_height_tool.h"
#include "editor/tools/editor_increase_resources_tool.h"
#include "editor/tools/editor_info_tool.h"
#include "editor/tools/editor_make_infrastructure_tool.h"
#include "editor/tools/editor_noise_height_tool.h"
#include "editor/tools/editor_place_bob_tool.h"
#include "editor/tools/editor_place_immovable_tool.h"
#include "editor/tools/editor_set_origin_tool.h"
#include "editor/tools/editor_set_port_space_tool.h"
#include "editor/tools/editor_set_starting_pos_tool.h"
#include "editor/tools/editor_set_terrain_tool.h"
#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_base.h"

class Editor;
class EditorTool;

/**
 * This is the EditorInteractive. It is like the InteractivePlayer class,
 * but for the Editor instead of the game
 */
struct EditorInteractive : public InteractiveBase {
	friend struct EditorToolMenu;

	// Runs the Editor via the commandline --editor flag. Will load 'filename' as a
	// map and run 'script_to_run' directly after all initialization is done.
	static void run_editor(const std::string & filename, const std::string& script_to_run);

private:
	EditorInteractive(Widelands::EditorGameBase &);

public:
	void register_overlays();
	void load(const std::string & filename);

	// leaf functions from base class
	void start() override;
	void think() override;

	void map_clicked(bool draw = false);
	void set_sel_pos(Widelands::NodeAndTriangle<>) override;
	void set_sel_radius_and_update_menu(uint32_t);

	//  Handle UI elements.
	bool handle_key(bool down, SDL_Keysym) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;

	struct Tools {
		Tools()
			:
			current_pointer(&increase_height),
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
		//ToolVector                     tools;
		EditorTool          *          current_pointer;
		EditorTool::ToolIndex         use_tool;
		EditorInfoTool                info;
		EditorSetHeightTool          set_height;
		EditorDecreaseHeightTool     decrease_height;
		EditorIncreaseHeightTool     increase_height;
		EditorNoiseHeightTool        noise_height;
		EditorSetTerrainTool         set_terrain;
		EditorDeleteImmovableTool    delete_immovable;
		EditorPlaceImmovableTool     place_immovable;
		EditorSetStartingPosTool    set_starting_pos;
		EditorDeleteBobTool          delete_bob;
		EditorPlaceBobTool           place_bob;
		EditorDecreaseResourcesTool  decrease_resources;
		EditorSetResourcesTool       set_resources;
		EditorIncreaseResourcesTool  increase_resources;
		EditorSetPortSpaceTool      set_port_space;
		EditorUnsetPortSpaceTool    unset_port_space;
		EditorSetOriginTool          set_origin;
		EditorMakeInfrastructureTool make_infrastructure;
	} tools;

	void select_tool(EditorTool &, EditorTool::ToolIndex);

	Widelands::Player * get_player() const override {return nullptr;}

	// action functions
	void exit();

	//  reference functions
	void   reference_player_tribe(Widelands::PlayerNumber, void const * const) override;
	void unreference_player_tribe(Widelands::PlayerNumber, void const * const);
	bool is_player_tribe_referenced(Widelands::PlayerNumber);
	void set_need_save(bool const t) {m_need_save = t;}

private:
	void toggle_buildhelp();
	void tool_menu_btn();
	void toolsize_menu_btn();
	void toggle_mainmenu();
	void toggle_playermenu();

	//  state variables
	bool m_need_save;
	struct PlayerReferences {
		int32_t      player;
		void const * object;
	};
	std::vector<PlayerReferences> m_player_tribe_references;

	int32_t m_realtime;
	bool m_left_mouse_button_is_down;

	EditorHistory m_history;

	UI::UniqueWindow::Registry m_toolmenu;

	UI::UniqueWindow::Registry m_toolsizemenu;
	UI::UniqueWindow::Registry m_playermenu;
	UI::UniqueWindow::Registry m_mainmenu;
	UI::UniqueWindow::Registry m_heightmenu;
	UI::UniqueWindow::Registry m_noise_heightmenu;
	UI::UniqueWindow::Registry m_terrainmenu;
	UI::UniqueWindow::Registry m_immovablemenu;
	UI::UniqueWindow::Registry m_bobmenu;
	UI::UniqueWindow::Registry m_resourcesmenu;

	UI::Button m_toggle_main_menu;
	UI::Button m_toggle_tool_menu;
	UI::Button m_toggle_toolsize_menu;
	UI::Button m_toggle_minimap;
	UI::Button m_toggle_buildhelp;
	UI::Button m_toggle_player_menu;
	UI::Button m_undo;
	UI::Button m_redo;
};

#endif  // end of include guard: WL_EDITOR_EDITORINTERACTIVE_H
