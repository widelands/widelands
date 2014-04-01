/*
 * Copyright (C) 2002, 2006-2009, 2011 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_MAPSELECT_H
#define FULLSCREEN_MENU_MAPSELECT_H

#include <set>
#include <string>

#include "ui_fsmenu/base.h"
#include "logic/map.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"


using Widelands::Map;
class GameController;
struct GameSettingsProvider;

namespace UI {
	struct Box;
}

/**
 * Data about a map that we're interested in.
 */
struct MapData {
	typedef std::set<std::string> Tags;

	std::string filename;
	std::string name;
	std::string author;
	std::string description;
	std::string hint;
	std::string world;
	Tags tags;

	uint32_t width;
	uint32_t height;
	uint32_t nrplayers;
	bool scenario; // is this a scenario we should list?

	MapData()
		: width(0), height(0), nrplayers(0), scenario(false) {}
};

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */

struct Fullscreen_Menu_MapSelect : public Fullscreen_Menu_Base {
	Fullscreen_Menu_MapSelect(GameSettingsProvider *, GameController *);

	bool is_scenario();
	MapData const * get_map() const;
	void think() override;

private:
	void ok();
	void map_selected(uint32_t);
	void changed(bool);
	void double_clicked(uint32_t);
	void fill_list();
	bool compare_maprows(uint32_t, uint32_t);

	UI::Checkbox * _add_tag_checkbox(UI::Box *, std::string, std::string);
	void _tagbox_changed(int32_t, bool);

	uint32_t     m_butw;
	uint32_t     m_buth;
	UI::Textarea m_title;
	UI::Textarea m_label_load_map_as_scenario;
	UI::Textarea m_label_name,       m_name;
	UI::Textarea m_label_author,     m_author;
	UI::Textarea m_label_size,       m_size;
	UI::Textarea m_label_world,      m_world;
	UI::Textarea m_label_nr_players, m_nr_players;
	UI::Textarea m_label_descr;
	UI::Multiline_Textarea m_descr;
	UI::Button m_back, m_ok;
	UI::Checkbox                      m_load_map_as_scenario;
	UI::Checkbox *                    m_show_all_maps;
	std::vector<UI::Checkbox *>       m_tags_checkboxes;
	UI::Table<uintptr_t const>        m_table;
	std::string                       m_curdir, m_basedir;
	Map::ScenarioTypes  m_scenario_types;

	std::vector<std::string> m_tags_ordered;
	std::set<uint32_t> m_req_tags;

	std::vector<MapData> m_maps_data;

	GameSettingsProvider * m_settings;
	GameController       * m_ctrl;
};

#endif
