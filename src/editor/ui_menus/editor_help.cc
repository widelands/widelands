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

#include "editor/ui_menus/editor_help.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/world/terrain_description.h"
#include "logic/world/world.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

#define WINDOW_WIDTH std::min(700, g_gr->get_xres() - 40)
#define WINDOW_HEIGHT std::min(550, g_gr->get_yres() - 40)

constexpr int kPadding = 5;
constexpr int kTabHeight = 35;

using namespace Widelands;

inline EditorInteractive& EditorHelp::eia() const {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

namespace {
const std::string heading(const std::string& text) {
	return ((boost::format("<rt><p font-size=18 font-weight=bold font-color=D1D1D1>"
	                       "%s<br></p><p font-size=8> <br></p></rt>") %
	         text).str());
}
}  // namespace

EditorHelp::EditorHelp(EditorInteractive& parent, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent, "encyclopedia", &registry, WINDOW_WIDTH, WINDOW_HEIGHT, _("Help")),
     tabs_(this, 0, 0, nullptr) {

	const int contents_height = WINDOW_HEIGHT - kTabHeight - 2 * kPadding;
	const int contents_width = WINDOW_WIDTH / 2 - 1.5 * kPadding;

	std::vector<std::unique_ptr<HelpTab>> tab_definitions;

	tab_definitions.push_back(
	   std::unique_ptr<HelpTab>(new HelpTab("terrains",
	                                        "pics/editor_menu_tool_set_terrain.png",
	                                        _("Terrains"),
	                                        "scripting/editor/terrain_help.lua",
	                                        HelpEntry::Type::kTerrain)));

	tab_definitions.push_back(
	   std::unique_ptr<HelpTab>(new HelpTab("trees",
	                                        "world/immovables/trees/alder/old/idle_0.png",
	                                        _("Trees"),
	                                        "scripting/editor/tree_help.lua",
	                                        HelpEntry::Type::kTree)));

	for (const auto& tab : tab_definitions) {
		// Make sure that all paths exist
		if (!g_fs->file_exists(tab->script_path)) {
			throw wexception("Script path %s for tab %s does not exist!",
			                 tab->script_path.c_str(),
			                 tab->key.c_str());
		}
		if (!g_fs->file_exists(tab->image_filename)) {
			throw wexception("Image path %s for tab %s does not exist!",
			                 tab->image_filename.c_str(),
			                 tab->key.c_str());
		}

		wrapper_boxes_.insert(std::make_pair(
		   tab->key, std::unique_ptr<UI::Box>(new UI::Box(&tabs_, 0, 0, UI::Box::Horizontal))));

		boxes_.insert(
		   std::make_pair(tab->key,
		                  std::unique_ptr<UI::Box>(new UI::Box(
		                     wrapper_boxes_.at(tab->key).get(), 0, 0, UI::Box::Horizontal))));

		lists_.insert(
		   std::make_pair(tab->key,
		                  std::unique_ptr<UI::Listselect<Widelands::DescriptionIndex>>(
		                     new UI::Listselect<Widelands::DescriptionIndex>(
		                        boxes_.at(tab->key).get(), 0, 0, contents_width, contents_height))));
		lists_.at(tab->key)->selected.connect(
		   boost::bind(&EditorHelp::entry_selected, this, tab->key, tab->script_path, tab->type));

		contents_.insert(
		   std::make_pair(tab->key,
		                  std::unique_ptr<UI::MultilineTextarea>(new UI::MultilineTextarea(
		                     boxes_.at(tab->key).get(), 0, 0, contents_width, contents_height))));

		boxes_.at(tab->key)->add(lists_.at(tab->key).get(), UI::Align_Left);
		boxes_.at(tab->key)->add_space(kPadding);
		boxes_.at(tab->key)->add(contents_.at(tab->key).get(), UI::Align_Left);

		wrapper_boxes_.at(tab->key)->add_space(kPadding);
		wrapper_boxes_.at(tab->key)->add(boxes_.at(tab->key).get(), UI::Align_Left);

		tabs_.add("editor_help_" + tab->key,
		          g_gr->images().get(tab->image_filename),
		          wrapper_boxes_.at(tab->key).get(),
		          tab->tooltip);
	}
	tabs_.set_size(WINDOW_WIDTH, WINDOW_HEIGHT);

	fill_terrains();
	fill_trees();

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void EditorHelp::fill_entries(const char* key, std::vector<HelpEntry>& entries) {
	std::sort(entries.begin(), entries.end());
	for (uint32_t i = 0; i < entries.size(); i++) {
		HelpEntry cur = entries[i];
		lists_.at(key)->add(cur.descname, cur.index, cur.icon);
	}
	lists_.at(key)->select(0);
}

void EditorHelp::fill_terrains() {
	const World& world = eia().egbase().world();
	std::vector<HelpEntry> entries;

	for (Widelands::DescriptionIndex i = 0; i < world.terrains().size(); ++i) {
		const TerrainDescription& terrain = world.terrain_descr(i);
		upcast(Image const, icon, &terrain.get_texture(0));
		HelpEntry entry(i, terrain.descname(), icon);
		entries.push_back(entry);
	}
	fill_entries("terrains", entries);
}

void EditorHelp::fill_trees() {
	const World& world = eia().egbase().world();
	std::vector<HelpEntry> entries;

	for (Widelands::DescriptionIndex i = 0; i < world.get_nr_immovables(); ++i) {
		const ImmovableDescr* immovable = world.get_immovable_descr(i);
		uint32_t attribute_id = immovable->get_attribute_id("tree");
		if (immovable->has_attribute(attribute_id)) {
			const Image* icon = immovable->representative_image();
			HelpEntry entry(i, immovable->descname(), icon);
			entries.push_back(entry);
		}
	}
	fill_entries("trees", entries);
}

void EditorHelp::entry_selected(const std::string& key,
                                const std::string& script_path,
                                const HelpEntry::Type& type) {
	try {
		std::unique_ptr<LuaTable> table(eia().egbase().lua().run_script(script_path));
		std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));

		std::string descname = "";

		switch (type) {
		case (HelpEntry::Type::kTerrain): {
			const TerrainDescription& descr =
			   eia().egbase().world().terrain_descr(lists_.at(key)->get_selected());
			descname = descr.descname();
			cr->push_arg(descr.name());
			break;
		}
		case (HelpEntry::Type::kTree): {
			const ImmovableDescr* descr =
			   eia().egbase().world().get_immovable_descr(lists_.at(key)->get_selected());
			descname = descr->descname();
			cr->push_arg(descr->name());
			break;
		}
		default:
			throw wexception("EditorHelp: No Type defined for tab.");
		}

		cr->resume();
		const std::string help_text = cr->pop_string();
		contents_.at(key)->set_text((boost::format("%s%s") % heading(descname) % help_text).str());

	} catch (LuaError& err) {
		contents_.at(key)->set_text(err.what());
	}
	contents_.at(key)->scroll_to_top();
}
