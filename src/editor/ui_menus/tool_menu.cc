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

#include "editor/ui_menus/tool_menu.h"

#include "base/i18n.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "editor/tools/decrease_height_tool.h"
#include "editor/tools/decrease_resources_tool.h"
#include "editor/tools/increase_height_tool.h"
#include "editor/tools/increase_resources_tool.h"
#include "editor/tools/noise_height_tool.h"
#include "editor/tools/place_bob_tool.h"
#include "editor/tools/place_immovable_tool.h"
#include "editor/tools/set_port_space_tool.h"
#include "editor/tools/set_terrain_tool.h"
#include "editor/ui_menus/tool_change_height_options_menu.h"
#include "editor/ui_menus/tool_change_resources_options_menu.h"
#include "editor/ui_menus/tool_noise_height_options_menu.h"
#include "editor/ui_menus/tool_place_bob_options_menu.h"
#include "editor/ui_menus/tool_place_immovable_options_menu.h"
#include "editor/ui_menus/tool_set_terrain_options_menu.h"
#include "graphic/graphic.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/textarea.h"

EditorToolMenu::EditorToolMenu(EditorInteractive& parent, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent, "tool_menu", &registry, 350, 400, _("Tools")) {

#define spacing 5
	Point const offs(spacing, spacing);
	Point pos = offs;
	int32_t const width = 34;
	int32_t const height = 34;

	int32_t const num_tools = 8;
#define ADD_BUTTON(pic, tooltip)                                                                   \
	radioselect_.add_button(                                                                        \
	   this, pos, g_gr->images().get("images/wui/editor/editor_menu_tool_" pic ".png"), tooltip);   \
	pos.x += width + spacing;

	ADD_BUTTON("change_height", _("Change height"));
	ADD_BUTTON("noise_height", _("Random height"));
	ADD_BUTTON("set_terrain", _("Terrain"));
	ADD_BUTTON("place_immovable", _("Immovables"));
	ADD_BUTTON("place_bob", _("Animals"));
	ADD_BUTTON("change_resources", _("Resources"));
	ADD_BUTTON("set_port_space", _("Set port space"));
	ADD_BUTTON("set_origin", _("Set the position that will have the coordinates (0, 0). This will "
	                           "be the top-left corner of a generated minimap."));

	set_inner_size(offs.x + (width + spacing) * num_tools, offs.y + (height + spacing));

	{
		const EditorTool& current = parent.tools()->current();
		radioselect_.set_state(&current == &parent.tools()->noise_height ?
		                          1 :
		                          &current == &parent.tools()->set_terrain ?
		                          2 :
		                          &current == &parent.tools()->place_immovable ?
		                          3 :
		                          &current == &parent.tools()->place_bob ?
		                          4 :
		                          &current == &parent.tools()->increase_resources ?
		                          5 :
		                          &current == &parent.tools()->set_port_space ?
		                          6 :
		                          &current == &parent.tools()->set_origin ? 7 : 0);
	}

	radioselect_.changed.connect(boost::bind(&EditorToolMenu::changed_to, this));
	radioselect_.clicked.connect(boost::bind(&EditorToolMenu::changed_to, this));

	if (get_usedefaultpos())
		center_to_parent();
}

/**
 * Called when the radiogroup changes or is reclicked
*/
void EditorToolMenu::changed_to() {
	const int32_t n = radioselect_.get_state();

	EditorInteractive& parent = dynamic_cast<EditorInteractive&>(*get_parent());

	EditorTool* current_tool_pointer = nullptr;
	UI::UniqueWindow::Registry* current_registry_pointer = nullptr;
	switch (n) {
	case 0:
		current_tool_pointer = &parent.tools()->increase_height;
		current_registry_pointer = &parent.heightmenu_;
		break;
	case 1:
		current_tool_pointer = &parent.tools()->noise_height;
		current_registry_pointer = &parent.noise_heightmenu_;
		break;
	case 2:
		current_tool_pointer = &parent.tools()->set_terrain;
		current_registry_pointer = &parent.terrainmenu_;
		break;
	case 3:
		current_tool_pointer = &parent.tools()->place_immovable;
		current_registry_pointer = &parent.immovablemenu_;
		break;
	case 4:
		current_tool_pointer = &parent.tools()->place_bob;
		current_registry_pointer = &parent.bobmenu_;
		break;
	case 5:
		current_tool_pointer = &parent.tools()->increase_resources;
		current_registry_pointer = &parent.resourcesmenu_;
		break;
	case 6:
		current_tool_pointer = &parent.tools()->set_port_space;
		current_registry_pointer = nullptr;  // no need for a window
		break;
	case 7:
		current_tool_pointer = &parent.tools()->set_origin;
		current_registry_pointer = nullptr;  // no need for a window
		break;
	default:
		NEVER_HERE();
	}

	parent.select_tool(*current_tool_pointer, EditorTool::First);
	if (current_tool_pointer == &parent.tools()->set_port_space) {
		// Set correct overlay
		Widelands::Map& map = parent.egbase().map();
		parent.mutable_field_overlay_manager()->register_overlay_callback_function(
		   boost::bind(&editor_Tool_set_port_space_callback, _1, boost::ref(map)));
		map.recalc_whole_map(parent.egbase().world());
	}

	if (current_registry_pointer) {
		if (UI::Window* const window = current_registry_pointer->window) {
			// There is already a window. If it is minimal, restore it.
			if (window->is_minimal())
				window->restore();
			else
				delete window;
		} else
			switch (n) {  //  create window
			case 0:
				new EditorToolChangeHeightOptionsMenu(
				   parent, parent.tools()->increase_height, *current_registry_pointer);
				break;
			case 1:
				new EditorToolNoiseHeightOptionsMenu(
				   parent, parent.tools()->noise_height, *current_registry_pointer);
				break;
			case 2:
				new EditorToolSetTerrainOptionsMenu(
				   parent, parent.tools()->set_terrain, *current_registry_pointer);
				break;
			case 3:
				new EditorToolPlaceImmovableOptionsMenu(
				   parent, parent.tools()->place_immovable, *current_registry_pointer);
				break;
			case 4:
				new EditorToolPlaceBobOptionsMenu(
				   parent, parent.tools()->place_bob, *current_registry_pointer);
				break;
			case 5:
				new EditorToolChangeResourcesOptionsMenu(
				   parent, parent.tools()->increase_resources, *current_registry_pointer);
				break;
			default:
				NEVER_HERE();
			}
	}
}
