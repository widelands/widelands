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
#include "logic/world.h"
#include "logic/worlddata.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/panel.h"
#include "wlapplication.h"

Editor_Tool_Set_Terrain_Options_Menu:: Editor_Tool_Set_Terrain_Options_Menu
	(Editor_Interactive         & parent,
	 Editor_Set_Terrain_Tool    & tool,
	 UI::UniqueWindow::Registry & registry)
	:
	Editor_Tool_Options_Menu(parent, registry, 0, 0, _("Terrain Select")),
	m_cur_selection         (this, 0, 0, 0, 20, UI::Align_Center),
	m_tool                  (tool),
	m_select_recursion_protect(false)
{
	Widelands::World & world = parent.egbase().map().world();
	Widelands::Terrain_Index const nr_terrains = world.get_nr_terrains();
	const uint32_t terrains_in_row = static_cast<uint32_t>
		(ceil(sqrt(static_cast<float>(nr_terrains))));

	static const int32_t check[] = {
		0,                                            //  "green"
		TERRAIN_DRY,                                  //  "dry"
		TERRAIN_DRY|TERRAIN_MOUNTAIN,                 //  "mountain"
		TERRAIN_DRY|TERRAIN_UNPASSABLE,               //  "unpassable"
		TERRAIN_ACID|TERRAIN_DRY|TERRAIN_UNPASSABLE,  //  "dead" or "acid"
		TERRAIN_UNPASSABLE|TERRAIN_DRY|TERRAIN_WATER,
	};

	m_checkboxes.resize(nr_terrains);

	const Image* green =
		g_gr->images().get("pics/terrain_green.png");
	const Image* water =
		g_gr->images().get("pics/terrain_water.png");
	const Image* mountain =
		g_gr->images().get("pics/terrain_mountain.png");
	const Image* dead =
		g_gr->images().get("pics/terrain_dead.png");
	const Image* unpassable =
		g_gr->images().get("pics/terrain_unpassable.png");
	const Image* dry =
		g_gr->images().get("pics/terrain_dry.png");

	static const int small_pich = 20;
	static const int small_picw = 20;

	uint32_t cur_x = 0;
	Point pos(hmargin(), vmargin());
	for (size_t checkfor = 0; checkfor < 6; ++checkfor) {
		for (Widelands::Terrain_Index i  = 0; i < nr_terrains; ++i) {
			const uint8_t ter_is = world.get_ter(i).get_is();
			if (ter_is != check[checkfor])
				continue;

			if (cur_x == terrains_in_row) {
				cur_x = 0;
				pos.x  = hmargin();
				pos.y += TEXTURE_HEIGHT + vspacing();
			}

			Surface* surf = Surface::create(64, 64);
			const Image* tex = g_gr->images().get
				(g_gr->get_maptexture_data(world.terrain_descr(i).get_texture())->get_texture_image());
			surf->blit(Point(0, 0), tex->surface(), Rect(0, 0, tex->width(), tex->height()), CM_Solid);

			Point pt(1, 64 - small_pich - 1);

			//  check is green
			if (ter_is == 0) {
				surf->blit(pt, green->surface(), Rect(0, 0, green->width(), green->height()));
				pt.x += small_picw + 1;
			} else {
				if (ter_is & TERRAIN_WATER) {
					surf->blit(pt, water->surface(), Rect(0, 0, water->width(), water->height()));
					pt.x += small_picw + 1;
				}
				if (ter_is & TERRAIN_MOUNTAIN) {
					surf->blit(pt, mountain->surface(), Rect(0, 0, mountain->width(), mountain->height()));
					pt.x += small_picw + 1;
				}
				if (ter_is & TERRAIN_ACID) {
					surf->blit(pt, dead->surface(), Rect(0, 0, dead->width(), dead->height()));
					pt.x += small_picw + 1;
				}
				if (ter_is & TERRAIN_UNPASSABLE) {
					surf->blit(pt, unpassable->surface(), Rect(0, 0, unpassable->width(), unpassable->height()));
					pt.x += small_picw + 1;
				}
				if (ter_is & TERRAIN_DRY)
					surf->blit(pt, dry->surface(), Rect(0, 0, dry->width(), dry->height()));
			}
			// Make sure we delete this later on.
			offscreen_images_.push_back(new_in_memory_image("dummy_hash", surf));

			UI::Checkbox & cb = *new UI::Checkbox(this, pos, offscreen_images_.back());
			cb.set_size(TEXTURE_WIDTH + 1, TEXTURE_HEIGHT + 1);
			cb.set_state(m_tool.is_enabled(i));
			cb.changedto.connect
				(boost::bind(&Editor_Tool_Set_Terrain_Options_Menu::selected, this, i, _1));
			m_checkboxes[i] = &cb;

			pos.x += TEXTURE_WIDTH + hspacing();
			++cur_x;
		}
	}
	pos.y += TEXTURE_HEIGHT + vspacing();

	set_inner_size
		(terrains_in_row * (TEXTURE_WIDTH + hspacing()) +
		 2 * hmargin() - hspacing(),
		 pos.y + m_cur_selection.get_h() + vmargin());
	pos.x = get_inner_w() / 2;
	m_cur_selection.set_pos(pos);

	std::string buf = _("Current:");
	uint32_t j = m_tool.get_nr_enabled();
	for (Widelands::Terrain_Index i = 0; j; ++i)
		if (m_tool.is_enabled(i)) {
			buf += " ";
			buf += world.get_ter(i).descname();
			--j;
		}
	m_cur_selection.set_text(buf);
}


Editor_Tool_Set_Terrain_Options_Menu::~Editor_Tool_Set_Terrain_Options_Menu()
{
	BOOST_FOREACH(const Image* pic, offscreen_images_)
		delete pic;
	offscreen_images_.clear();
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

		std::string buf = _("Current:");
		const Widelands::World & world =
			ref_cast<Editor_Interactive, UI::Panel>(*get_parent())
			.egbase().map().world();
		uint32_t j = m_tool.get_nr_enabled();
		for (Widelands::Terrain_Index i = 0; j; ++i)
			if (m_tool.is_enabled(i)) {
				buf += " ";
				buf += world.get_ter(i).descname();
				--j;
			}

		m_cur_selection.set_text(buf.c_str());
		m_cur_selection.set_pos
			(Point
			 	((get_inner_w() - m_cur_selection.get_w()) / 2,
			 	 m_cur_selection.get_y()));
	}
}
