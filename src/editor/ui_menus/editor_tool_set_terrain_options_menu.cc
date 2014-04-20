/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "editor/ui_menus/editor_tool_set_terrain_options_menu.h"

#include <SDL_keysym.h>
#include <boost/foreach.hpp>

#include "editor/editorinteractive.h"
#include "editor/tools/editor_set_terrain_tool.h"
#include "graphic/graphic.h"
#include "graphic/in_memory_image.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "graphic/texture.h"
#include "i18n.h"
#include "logic/map.h"
#include "logic/world/editor_category.h"
#include "logic/world/terrain_description.h"
#include "logic/world/world.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/panel.h"
#include "ui_basic/tabpanel.h"
#include "wlapplication.h"

namespace {

using namespace Widelands;

static const int32_t check[] = {
   TerrainDescription::GREEN,                                 //  "green"
   TerrainDescription::DRY,                                   //  "dry"
   TerrainDescription::DRY | TerrainDescription::MOUNTAIN,    //  "mountain"
   TerrainDescription::DRY | TerrainDescription::UNPASSABLE,  //  "unpassable"
   TerrainDescription::ACID | TerrainDescription::DRY |
      TerrainDescription::UNPASSABLE,  //  "dead" or "acid"
   TerrainDescription::UNPASSABLE | TerrainDescription::DRY | TerrainDescription::WATER,
   -1,  // end marker
};


}  // namespace

Editor_Tool_Set_Terrain_Options_Menu::Editor_Tool_Set_Terrain_Options_Menu(
   Editor_Interactive& parent, Editor_Set_Terrain_Tool& tool, UI::UniqueWindow::Registry& registry)
   : Editor_Tool_Options_Menu(parent, registry, 0, 0, _("Terrain Select")),
     world_(parent.egbase().world()),
     m_cur_selection(this, 0, 0, 0, 20, UI::Align_Center),
     m_tool(tool),
     m_select_recursion_protect(false) {

	UI::Box* vertical = new UI::Box(this, 0, 0, UI::Box::Vertical);

	UI::Tab_Panel* tab_panel = new UI::Tab_Panel(vertical, 0, 0, nullptr);
	vertical->add(tab_panel, UI::Align_Center);

	for (uint32_t i = 0; i < world_.editor_categories().get_nitems(); ++i) {
		const EditorCategory& editor_category = *world_.editor_categories().get(i);

		std::vector<Terrain_Index> terrain_indices;
		for (Terrain_Index j = 0; j < world_.terrains().get_nitems(); ++j) {
			if (world_.terrain_descr(j).editor_category().name() != editor_category.name()) {
				continue;
			}
			terrain_indices.push_back(j);
		}

		tab_panel->add(editor_category.name(),
				editor_category.picture(),
				add_checkboxes(tab_panel, terrain_indices),
				editor_category.descname());
	}
	vertical->add(&m_cur_selection, UI::Align_Center, true);

	update_label();
	set_center_panel(vertical);
}

void Editor_Tool_Set_Terrain_Options_Menu::update_label() {
	std::string buf = _("Current:");
	int j = m_tool.get_nr_enabled();
	for (Terrain_Index i = 0; j; ++i) {
		if (m_tool.is_enabled(i)) {
			buf += " ";
			buf += world_.terrain_descr(i).descname();
			--j;
		}
	}
	m_cur_selection.set_text(buf);
}

UI::Box* Editor_Tool_Set_Terrain_Options_Menu::add_checkboxes(UI::Panel* parent,
                          const std::vector<Terrain_Index>& terrain_indices) {
	const uint32_t terrains_in_row =
	   static_cast<uint32_t>(ceil(sqrt(static_cast<float>(terrain_indices.size()))));

	UI::Box* vertical = new UI::Box(parent, 0, 0, UI::Box::Vertical);
	vertical->add_space(vspacing());

	const Image* green = g_gr->images().get("pics/terrain_green.png");
	const Image* water = g_gr->images().get("pics/terrain_water.png");
	const Image* mountain = g_gr->images().get("pics/terrain_mountain.png");
	const Image* dead = g_gr->images().get("pics/terrain_dead.png");
	const Image* unpassable = g_gr->images().get("pics/terrain_unpassable.png");
	const Image* dry = g_gr->images().get("pics/terrain_dry.png");

	constexpr int kSmallPicHeight = 20;
	constexpr int kSmallPicWidth = 20;
	UI::Box* horizontal;
	int nterrains_handled = 0;
	for (const Terrain_Index& i : terrain_indices) {
		for (size_t checkfor = 0; check[checkfor] >= 0; ++checkfor) {
			const TerrainDescription::Type ter_is = world_.terrain_descr(i).get_is();
			if (ter_is != check[checkfor])
				continue;

			if (nterrains_handled % terrains_in_row == 0) {
				horizontal = new UI::Box(vertical, 0, 0, UI::Box::Horizontal);
				horizontal->add_space(hspacing());

				vertical->add(horizontal, UI::Align_Left);
				vertical->add_space(vspacing());
			}

			Surface* surf = Surface::create(TEXTURE_WIDTH, TEXTURE_HEIGHT);
			const Image* tex = g_gr->images().get(
			   g_gr->get_maptexture_data(world_.terrain_descr(i).get_texture())->get_texture_image());
			surf->blit(Point(0, 0), tex->surface(), Rect(0, 0, tex->width(), tex->height()), CM_Solid);
			Point pt(1, TEXTURE_HEIGHT - kSmallPicHeight - 1);

			if (ter_is == TerrainDescription::GREEN) {
				surf->blit(pt, green->surface(), Rect(0, 0, green->width(), green->height()));
				pt.x += kSmallPicWidth + 1;
			} else {
				if (ter_is & TerrainDescription::WATER) {
					surf->blit(pt, water->surface(), Rect(0, 0, water->width(), water->height()));
					pt.x += kSmallPicWidth + 1;
				}
				if (ter_is & TerrainDescription::MOUNTAIN) {
					surf->blit(
					   pt, mountain->surface(), Rect(0, 0, mountain->width(), mountain->height()));
					pt.x += kSmallPicWidth + 1;
				}
				if (ter_is & TerrainDescription::ACID) {
					surf->blit(pt, dead->surface(), Rect(0, 0, dead->width(), dead->height()));
					pt.x += kSmallPicWidth + 1;
				}
				if (ter_is & TerrainDescription::UNPASSABLE) {
					surf->blit(
					   pt, unpassable->surface(), Rect(0, 0, unpassable->width(), unpassable->height()));
					pt.x += kSmallPicWidth + 1;
				}
				if (ter_is & TerrainDescription::DRY)
					surf->blit(pt, dry->surface(), Rect(0, 0, dry->width(), dry->height()));
			}
			// Make sure we delete this later on.
			offscreen_images_.emplace_back(new_in_memory_image("dummy_hash", surf));

			UI::Checkbox* cb =
			   new UI::Checkbox(horizontal, Point(0, 0), offscreen_images_.back().get());
			cb->set_desired_size(TEXTURE_WIDTH + 1, TEXTURE_HEIGHT + 1);
			cb->set_state(m_tool.is_enabled(i));
			cb->changedto.connect(
			   boost::bind(&Editor_Tool_Set_Terrain_Options_Menu::selected, this, i, _1));
			m_checkboxes[i] = cb;
			horizontal->add(cb, UI::Align_Left);
			horizontal->add_space(hspacing());
		}
		++nterrains_handled;
	}
	return vertical;
}

Editor_Tool_Set_Terrain_Options_Menu::~Editor_Tool_Set_Terrain_Options_Menu() {
}

void Editor_Tool_Set_Terrain_Options_Menu::selected
	(int32_t const n, bool const t)
{
	if (m_select_recursion_protect)
		return;

	//  FIXME This code is erroneous. It checks the current key state. What it
	//  FIXME needs is the key state at the time the mouse was clicked. See the
	//  FIXME usage comment for get_key_state.
	const bool multiselect =
		get_key_state(SDLK_LCTRL) | get_key_state(SDLK_RCTRL);
	if (not t and (not multiselect or m_tool.get_nr_enabled() == 1))
		m_checkboxes[n]->set_state(true);
	else {
		if (not multiselect) {
			for (uint32_t i = 0; m_tool.get_nr_enabled(); ++i)
				m_tool.enable(i, false);
			//  disable all checkboxes
			m_select_recursion_protect = true;
			const int32_t size = m_checkboxes.size();
			for (int32_t i = 0; i < size; ++i) {
				if (i != n)
					m_checkboxes[i]->set_state(false);
			}
			m_select_recursion_protect = false;
		}

		m_tool.enable(n, t);
		select_correct_tool();
		update_label();
	}
}
