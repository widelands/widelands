/*
 * Copyright (C) 2002-2004, 2006-2009, 2011, 2013 by the Widelands Development Team
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

#include "editor/ui_menus/editor_tool_menu.h"

#include "editor/editorinteractive.h"
#include "editor/tools/editor_decrease_height_tool.h"
#include "editor/tools/editor_decrease_resources_tool.h"
#include "editor/tools/editor_increase_height_tool.h"
#include "editor/tools/editor_increase_resources_tool.h"
#include "editor/tools/editor_noise_height_tool.h"
#include "editor/tools/editor_place_bob_tool.h"
#include "editor/tools/editor_place_immovable_tool.h"
#include "editor/tools/editor_set_port_space_tool.h"
#include "editor/tools/editor_set_terrain_tool.h"
#include "editor/ui_menus/editor_tool_change_height_options_menu.h"
#include "editor/ui_menus/editor_tool_change_resources_options_menu.h"
#include "editor/ui_menus/editor_tool_noise_height_options_menu.h"
#include "editor/ui_menus/editor_tool_place_bob_options_menu.h"
#include "editor/ui_menus/editor_tool_place_immovable_options_menu.h"
#include "editor/ui_menus/editor_tool_set_terrain_options_menu.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/textarea.h"

Editor_Tool_Menu::Editor_Tool_Menu
	(Editor_Interactive & parent, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow(&parent, "tool_menu", &registry, 350, 400, _("Tools"))
{

#define spacing 5
	Point   const offs     (spacing, spacing);
	Point         pos     = offs;
	int32_t const width   = 34;
	int32_t const height  = 34;


	int32_t const num_tools = 7;
#define ADD_BUTTON(pic, tooltip)                                              \
   m_radioselect.add_button                                                   \
      (this,                                                                  \
       pos,                                                                   \
       g_gr->images().get("pics/editor_menu_tool_" pic ".png"),       \
       tooltip);                                                              \
   pos.x += width + spacing;                                                  \

	ADD_BUTTON("change_height",    _("Change height"));
	ADD_BUTTON("noise_height",     _("Noise height"));
	ADD_BUTTON("set_terrain",      _("Terrain"));
	ADD_BUTTON("place_immovable",  _("Immovables"));
	ADD_BUTTON("place_bob",        _("Bobs"));
	ADD_BUTTON("change_resources", _("Resources"));
	ADD_BUTTON("set_port_space",   _("Set port space"));

	set_inner_size
		(offs.x + (width + spacing) * num_tools, offs.y + (height + spacing));

	{
		const Editor_Tool & current = parent.tools.current();
		m_radioselect.set_state
			(&current == &parent.tools.noise_height       ? 1 :
			 &current == &parent.tools.set_terrain        ? 2 :
			 &current == &parent.tools.place_immovable    ? 3 :
			 &current == &parent.tools.place_bob          ? 4 :
			 &current == &parent.tools.increase_resources ? 5 :
			 &current == &parent.tools.set_port_space     ? 6 :
			 0);
	}

	m_radioselect.changed.connect(boost::bind(&Editor_Tool_Menu::changed_to, this));
	m_radioselect.clicked.connect(boost::bind(&Editor_Tool_Menu::changed_to, this));

	if (get_usedefaultpos())
		center_to_parent();
}

/**
 * Called when the radiogroup changes or is reclicked
*/
void Editor_Tool_Menu::changed_to() {
	const int32_t n = m_radioselect.get_state();

	Editor_Interactive & parent =
		ref_cast<Editor_Interactive, UI::Panel>(*get_parent());

	Editor_Tool                * current_tool_pointer = nullptr;
	UI::UniqueWindow::Registry * current_registry_pointer = nullptr;
	switch (n) {
	case 0:
		current_tool_pointer     = &parent.tools.increase_height;
		current_registry_pointer = &parent.m_heightmenu;
		break;
	case 1:
		current_tool_pointer     = &parent.tools.noise_height;
		current_registry_pointer = &parent.m_noise_heightmenu;
		break;
	case 2:
		current_tool_pointer     = &parent.tools.set_terrain;
		current_registry_pointer = &parent.m_terrainmenu;
		break;
	case 3:
		current_tool_pointer     = &parent.tools.place_immovable;
		current_registry_pointer = &parent.m_immovablemenu;
		break;
	case 4:
		current_tool_pointer     = &parent.tools.place_bob;
		current_registry_pointer = &parent.m_bobmenu;
		break;
	case 5:
		current_tool_pointer     = &parent.tools.increase_resources;
		current_registry_pointer = &parent.m_resourcesmenu;
		break;
	case 6:
		current_tool_pointer     = &parent.tools.set_port_space;
		current_registry_pointer = nullptr; // no need for a window
		break;
	default:
		assert(false);
		break;
	}

	parent.select_tool(*current_tool_pointer, Editor_Tool::First);
	if (current_tool_pointer == &parent.tools.set_port_space) {
		// Set correct overlay
		Widelands::Map & map = parent.egbase().map();
		map.overlay_manager().register_overlay_callback_function(
				boost::bind(&Editor_Tool_Set_Port_Space_Callback, _1, boost::ref(map)));
		map.recalc_whole_map();
		update();
	}

	if (current_registry_pointer) {
		if (UI::Window * const window = current_registry_pointer->window) {
			// There is already a window. If it is minimal, restore it.
			if (window->is_minimal())
				window->restore();
			else
				delete window;
		} else
			switch (n) { //  create window
			case 0:
				new Editor_Tool_Change_Height_Options_Menu
					(parent,
					parent.tools.increase_height,
					*current_registry_pointer);
				break;
			case 1:
				new Editor_Tool_Noise_Height_Options_Menu
					(parent,
					parent.tools.noise_height,
					*current_registry_pointer);
				break;
			case 2:
				new Editor_Tool_Set_Terrain_Options_Menu
					(parent,
					parent.tools.set_terrain,
					*current_registry_pointer);
				break;
			case 3:
				new Editor_Tool_Place_Immovable_Options_Menu
					(parent,
					parent.tools.place_immovable,
					*current_registry_pointer);
				break;
			case 4:
				new Editor_Tool_Place_Bob_Options_Menu
					(parent,
					parent.tools.place_bob,
					*current_registry_pointer);
				break;
			case 5:
				new Editor_Tool_Change_Resources_Options_Menu
					(parent,
					parent.tools.increase_resources,
					*current_registry_pointer);
				break;
			default:
				assert(false);
			}
	}
}
