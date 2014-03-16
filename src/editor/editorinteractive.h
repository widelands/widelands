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

#ifndef EDITORINTERACTIVE_H
#define EDITORINTERACTIVE_H

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
class Editor_Tool;

/**
 * This is the EditorInteractive. It is like the InteractivePlayer class,
 * but for the Editor instead of the game
 */
struct Editor_Interactive : public Interactive_Base {
	friend struct Editor_Tool_Menu;

	// Runs the Editor via the commandline --editor flag. Will load 'filename' as a
	// map and run 'script_to_run' directly after all initialization is done.
	static void run_editor(const std::string & filename, const std::string& script_to_run);

private:
	Editor_Interactive(Widelands::Editor_Game_Base &);

public:
	void register_overlays();
	void load(const std::string & filename);

	// leaf functions from base class
	void start() override;
	void think() override;

	void map_clicked(bool draw = false);
	virtual void set_sel_pos(Widelands::Node_and_Triangle<>) override;
	void set_sel_radius_and_update_menu(uint32_t);

	//  Handle UI elements.
	bool handle_key(bool down, SDL_keysym) override;
	bool handle_mousepress(Uint8 btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y) override;

	struct Tools {
		Tools()
			:
			current_pointer(&increase_height),
			use_tool(Editor_Tool::First),
			increase_height(decrease_height, set_height),
			noise_height(set_height),
			place_immovable(delete_immovable),
			place_bob(delete_bob),
			increase_resources(decrease_resources, set_resources),
			set_port_space(unset_port_space)

		{}
		Editor_Tool & current() const {return *current_pointer;}
		typedef std::vector<Editor_Tool *> Tool_Vector;
		typedef Tool_Vector::size_type Index;
		//Tool_Vector                     tools;
		Editor_Tool          *          current_pointer;
		Editor_Tool::Tool_Index         use_tool;
		Editor_Info_Tool                info;
		Editor_Set_Height_Tool          set_height;
		Editor_Decrease_Height_Tool     decrease_height;
		Editor_Increase_Height_Tool     increase_height;
		Editor_Noise_Height_Tool        noise_height;
		Editor_Set_Terrain_Tool         set_terrain;
		Editor_Delete_Immovable_Tool    delete_immovable;
		Editor_Place_Immovable_Tool     place_immovable;
		Editor_Set_Starting_Pos_Tool    set_starting_pos;
		Editor_Delete_Bob_Tool          delete_bob;
		Editor_Place_Bob_Tool           place_bob;
		Editor_Decrease_Resources_Tool  decrease_resources;
		Editor_Set_Resources_Tool       set_resources;
		Editor_Increase_Resources_Tool  increase_resources;
		Editor_Set_Port_Space_Tool      set_port_space;
		Editor_Unset_Port_Space_Tool    unset_port_space;
		Editor_Set_Origin_Tool          set_origin;
		Editor_Make_Infrastructure_Tool make_infrastructure;
	} tools;

	void select_tool(Editor_Tool &, Editor_Tool::Tool_Index);

	Widelands::Player * get_player() const override {return nullptr;}

	// action functions
	void exit();

	//  reference functions
	void   reference_player_tribe(Widelands::Player_Number, void const * const) override;
	void unreference_player_tribe(Widelands::Player_Number, void const * const);
	bool is_player_tribe_referenced(Widelands::Player_Number);
	void set_need_save(bool const t) {m_need_save = t;}

	/// Must be called when the world is changed. Takes care of closing the tool
	/// menus that are showing elements from the old world.
	void change_world();

private:
	void toggle_buildhelp();
	void tool_menu_btn();
	void toolsize_menu_btn();
	void toggle_mainmenu();
	void toggle_playermenu();

	//  state variables
	bool m_need_save;
	struct Player_References {
		int32_t      player;
		void const * object;
	};
	std::vector<Player_References> m_player_tribe_references;

	int32_t m_realtime;
	bool m_left_mouse_button_is_down;

	Editor_History m_history;

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

#endif
