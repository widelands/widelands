/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "editor_tool_set_terrain_options_menu.h"

#include "editor_set_terrain_tool.h"
#include "editorinteractive.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"
#include "rendertarget.h"
#include <SDL_keysym.h>
#include "wlapplication.h"
#include "world.h"
#include "worlddata.h"

#include "ui_button.h"
#include "ui_panel.h"
#include "ui_checkbox.h"

using Widelands::Terrain_Descr;

Editor_Tool_Set_Terrain_Tool_Options_Menu::
Editor_Tool_Set_Terrain_Tool_Options_Menu
(Editor_Interactive         & parent,
 Editor_Set_Terrain_Tool    & tool,
 UI::UniqueWindow::Registry & registry)
:
Editor_Tool_Options_Menu(parent, registry, 0, 0, _("Terrain Select")),

m_cur_selection(this, 0, 0, 0, 20, Align_Center),
m_tool(tool)
{
	Widelands::World & world = parent.egbase().map().world();
	const Terrain_Descr::Index nr_terrains = world.get_nr_terrains();
	const uint32_t terrains_in_row = static_cast<uint32_t>
		(ceil(sqrt(static_cast<float>(nr_terrains))));


	int32_t check[] = {
		0,                                            //  "green"
		TERRAIN_DRY,                                  //  "dry"
		TERRAIN_DRY|TERRAIN_MOUNTAIN,                 //  "mountain"
		TERRAIN_DRY|TERRAIN_UNPASSABLE,               //  "unpassable"
		TERRAIN_ACID|TERRAIN_DRY|TERRAIN_UNPASSABLE,  //  "dead" or "acid"
		TERRAIN_UNPASSABLE|TERRAIN_DRY|TERRAIN_WATER,
	};

	m_checkboxes.resize(nr_terrains);

	const uint32_t green = g_gr->get_picture(PicMod_Game, "pics/terrain_green.png");
	const uint32_t water = g_gr->get_picture(PicMod_Game, "pics/terrain_water.png");
	const uint32_t mountain =
		g_gr->get_picture(PicMod_Game, "pics/terrain_mountain.png");
	const uint32_t dead =  g_gr->get_picture(PicMod_Game, "pics/terrain_dead.png");
	const uint32_t unpassable =
		g_gr->get_picture(PicMod_Game, "pics/terrain_unpassable.png");
	const uint32_t dry =   g_gr->get_picture(PicMod_Game, "pics/terrain_dry.png");
	uint32_t small_picw, small_pich;
	g_gr->get_picture_size(dry, small_picw, small_pich);

	uint32_t cur_x = 0;
	Point pos(hmargin(), vmargin());
	for (size_t checkfor = 0; checkfor < 6; ++checkfor)
		for (Terrain_Descr::Index i  = 0; i < nr_terrains; ++i) {

			const uint8_t ter_is = world.get_ter(i).get_is();
			if (ter_is != check[checkfor]) continue;

			if (cur_x == terrains_in_row) {
				cur_x = 0;
				pos.x  = hmargin();
				pos.y += TEXTURE_HEIGHT + vspacing();
			}

			//  create a surface for this
			uint32_t picw, pich;
			g_gr->get_picture_size
				(g_gr->get_picture
				 	(PicMod_Game, g_gr->get_maptexture_picture(i + 1)),
				 picw, pich);
			uint32_t surface = g_gr->create_surface(picw, pich);

			//  get the rendertarget for this
			RenderTarget & target = *g_gr->get_surface_renderer(surface);

			//  firts, blit the terrain texture
			target.blit
				(Point(0, 0),
				 g_gr->get_picture
				 	(PicMod_Game, g_gr->get_maptexture_picture(i + 1)));

			Point pic(1, pich - small_pich - 1);

			//  check is green
			if (ter_is == 0) {
				target.blit(pic, green);
				pic.x += small_picw + 1;
			} else {
				if (ter_is & TERRAIN_WATER) {
					target.blit(pic, water);
					pic.x += small_picw + 1;
				}
				if (ter_is & TERRAIN_MOUNTAIN) {
					target.blit(pic, mountain);
					pic.x += small_picw + 1;
				}
				if (ter_is & TERRAIN_ACID) {
					target.blit(pic, dead);
					pic.x += small_picw + 1;
				}
				if (ter_is & TERRAIN_UNPASSABLE) {
					target.blit(pic, unpassable);
					pic.x += small_picw + 1;
				}
				if (ter_is & TERRAIN_DRY)
					target.blit(pic, dry);
			}

			//  Save this surface, so we can free it later on.
			m_surfaces.push_back(surface);

			UI::Checkbox & cb = *new UI::Checkbox(this, pos.x, pos.y, surface);
			cb.set_size(TEXTURE_WIDTH + 1, TEXTURE_HEIGHT + 1);
			cb.set_id(i);
			cb.set_state(m_tool.is_enabled(i));
			cb.changedtoid.set
				(this, &Editor_Tool_Set_Terrain_Tool_Options_Menu::selected);
			m_checkboxes[i] = &cb;

			pos.x += TEXTURE_WIDTH + hspacing();
			++cur_x;
		}
	pos.y += TEXTURE_HEIGHT + vspacing();

	set_inner_size
		(terrains_in_row * (TEXTURE_WIDTH + hspacing()) + 2 * hmargin() - hspacing(),
		 pos.y + m_cur_selection.get_h() + vmargin());
	pos.x = get_inner_w() / 2;
	m_cur_selection.set_pos(pos);

	std::string buf = _("Current:");
	uint32_t j = m_tool.get_nr_enabled();
	for (Terrain_Descr::Index i = 0; j; ++i)
		if (m_tool.is_enabled(i)) {
			buf += " ";
			buf += world.get_ter(i).name();
			--j;
		}
	m_cur_selection.set_text(buf);
}


Editor_Tool_Set_Terrain_Tool_Options_Menu::~Editor_Tool_Set_Terrain_Tool_Options_Menu()  {
	const std::vector<uint32_t>::const_iterator surfaces_end = m_surfaces.end();
	for
		(std::vector<uint32_t>::const_iterator it = m_surfaces.begin();
		 it != surfaces_end;
		 ++it)
		g_gr->free_surface(*it);
}


void Editor_Tool_Set_Terrain_Tool_Options_Menu::do_nothing(int32_t, bool) {}


void Editor_Tool_Set_Terrain_Tool_Options_Menu::selected(int32_t n, bool t) {
	//  FIXME This code is erroneous. It checks the current key state. What it
	//  FIXME needs is the key state at the time the mouse was clicked. See the
	//  FIXME usage comment for get_key_state.
	const bool multiselect =
		get_key_state(SDLK_LCTRL) | get_key_state(SDLK_RCTRL);
	if (not t and (not multiselect or m_tool.get_nr_enabled() == 1))
		m_checkboxes[n]->set_state(true);
	else {
		if (not multiselect) {
			for (uint32_t i = 0; m_tool.get_nr_enabled(); ++i) m_tool.enable(i, false);
			//  disable all checkboxes
			const uint32_t size = m_checkboxes.size();
			//TODO: the uint32_t cast is ugly!
			for (uint32_t i = 0; i < size; ++i, i += i == static_cast<uint32_t>(n)) {
				m_checkboxes[i]->changedtoid.set
					(this, &Editor_Tool_Set_Terrain_Tool_Options_Menu::do_nothing);
				m_checkboxes[i]->set_state(false);
				m_checkboxes[i]->changedtoid.set
					(this, &Editor_Tool_Set_Terrain_Tool_Options_Menu::selected);
			}
		}

		m_tool.enable(n, t);
		select_correct_tool();

		std::string buf = _("Current:");
		Widelands::World const & world =
			dynamic_cast<Editor_Interactive &>(*get_parent())
			.egbase().map().world();
		uint32_t j = m_tool.get_nr_enabled();
		for (Terrain_Descr::Index i = 0; j; ++i)
			if (m_tool.is_enabled(i)) {
				buf += " ";
				buf += world.get_ter(i).name();
				--j;
			}

		m_cur_selection.set_text(buf.c_str());
		m_cur_selection.set_pos
			(Point
			 	((get_inner_w() - m_cur_selection.get_w()) / 2,
			 	 m_cur_selection.get_y()));
	}
}
