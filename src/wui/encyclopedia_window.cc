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

#include "wui/encyclopedia_window.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/building.h"
#include "logic/player.h"
#include "logic/tribes/tribe_descr.h"
#include "logic/tribes/tribes.h"
#include "logic/ware_descr.h"
#include "logic/worker_descr.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "wui/interactive_player.h"

#define WINDOW_WIDTH std::min(700, g_gr->get_xres() - 40)
#define WINDOW_HEIGHT std::min(550, g_gr->get_yres() - 40)

constexpr int kPadding = 5;
constexpr int kTabHeight = 35;

using namespace Widelands;

inline InteractivePlayer& EncyclopediaWindow::iaplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

namespace {

struct EncyclopediaTab {
	EncyclopediaTab(const std::string& _key,
	                const std::string& _image_filename,
	                const std::string& _tooltip,
	                const std::string& _script_path,
	                const Widelands::MapObjectType _type)
	   : key(_key),
	     image_filename(_image_filename),
	     tooltip(_tooltip),
	     script_path(_script_path),
	     type(_type) {
	}
	const std::string key;
	const std::string image_filename;
	const std::string tooltip;
	const std::string script_path;
	const Widelands::MapObjectType type;
};

const std::string heading(const std::string& text) {
	return ((boost::format("<rt><p font-size=18 font-weight=bold font-color=D1D1D1>"
	                       "%s<br></p><p font-size=8> <br></p></rt>") %
	         text).str());
}
}  // namespace

EncyclopediaWindow::EncyclopediaWindow(InteractivePlayer& parent, UI::UniqueWindow::Registry& registry)
	: UI::UniqueWindow(
        &parent, "encyclopedia", &registry, WINDOW_WIDTH, WINDOW_HEIGHT, _("Tribal Encyclopedia")),
     tabs_(this, 0, 0, nullptr) {

	const int contents_height = WINDOW_HEIGHT - kTabHeight - 2 * kPadding;
	const int contents_width = WINDOW_WIDTH / 2 - 1.5 * kPadding;

	std::vector<std::unique_ptr<EncyclopediaTab>> tab_definitions;

	tab_definitions.push_back(
	   std::unique_ptr<EncyclopediaTab>(new EncyclopediaTab("wares",
	                                                        "pics/menu_tab_wares.png",
	                                                        _("Wares"),
	                                                        "tribes/scripting/help/ware_help.lua",
	                                                        Widelands::MapObjectType::WARE)));

	tab_definitions.push_back(
	   std::unique_ptr<EncyclopediaTab>(new EncyclopediaTab("workers",
	                                                        "pics/menu_tab_workers.png",
	                                                        _("Workers"),
	                                                        "tribes/scripting/help/worker_help.lua",
	                                                        Widelands::MapObjectType::WORKER)));

	tab_definitions.push_back(std::unique_ptr<EncyclopediaTab>(
	   new EncyclopediaTab("buildings",
	                       "pics/genstats_nrbuildings.png",
	                       _("Buildings"),
	                       "tribes/scripting/help/building_help.lua",
	                       Widelands::MapObjectType::BUILDING)));

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
		lists_.at(tab->key)->selected.connect(boost::bind(
		   &EncyclopediaWindow::entry_selected, this, tab->key, tab->script_path, tab->type));

		contents_.insert(
		   std::make_pair(tab->key,
		                  std::unique_ptr<UI::MultilineTextarea>(new UI::MultilineTextarea(
		                     boxes_.at(tab->key).get(), 0, 0, contents_width, contents_height))));

		boxes_.at(tab->key)->add(lists_.at(tab->key).get(), UI::Align::kLeft);
		boxes_.at(tab->key)->add_space(kPadding);
		boxes_.at(tab->key)->add(contents_.at(tab->key).get(), UI::Align::kLeft);

		wrapper_boxes_.at(tab->key)->add_space(kPadding);
		wrapper_boxes_.at(tab->key)->add(boxes_.at(tab->key).get(), UI::Align::kLeft);

		tabs_.add("encyclopedia_" + tab->key,
		          g_gr->images().get(tab->image_filename),
		          wrapper_boxes_.at(tab->key).get(),
		          tab->tooltip);
	}
	tabs_.set_size(WINDOW_WIDTH, WINDOW_HEIGHT);

	fill_buildings();
	fill_wares();
	fill_workers();

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void EncyclopediaWindow::fill_entries(const char* key, std::vector<EncyclopediaEntry>& entries) {
	std::sort(entries.begin(), entries.end());
	for (uint32_t i = 0; i < entries.size(); i++) {
		EncyclopediaEntry cur = entries[i];
		lists_.at(key)->add(cur.descname, cur.index, cur.icon);
	}
	lists_.at(key)->select(0);
}

void EncyclopediaWindow::fill_buildings() {
	const Tribes& tribes = iaplayer().egbase().tribes();
	const TribeDescr& tribe = iaplayer().player().tribe();
	std::vector<EncyclopediaEntry> entries;

	for (Widelands::DescriptionIndex i = 0; i < tribes.nrbuildings(); ++i) {
		const BuildingDescr* building = tribes.get_building_descr(i);
		if (tribe.has_building(i) || building->type() == MapObjectType::MILITARYSITE) {
			EncyclopediaEntry entry(i, building->descname(), building->icon());
			entries.push_back(entry);
		}
	}
	fill_entries("buildings", entries);
}

void EncyclopediaWindow::fill_wares() {
	const TribeDescr& tribe = iaplayer().player().tribe();
	std::vector<EncyclopediaEntry> entries;

	for (const Widelands::DescriptionIndex& i : tribe.wares()) {
		const WareDescr* ware = tribe.get_ware_descr(i);
		EncyclopediaEntry entry(i, ware->descname(), ware->icon());
		entries.push_back(entry);
	}
	fill_entries("wares", entries);
}

void EncyclopediaWindow::fill_workers() {
	const TribeDescr& tribe = iaplayer().player().tribe();
	std::vector<EncyclopediaEntry> entries;

	for (const Widelands::DescriptionIndex& i : tribe.workers()) {
		const WorkerDescr* worker = tribe.get_worker_descr(i);
		EncyclopediaEntry entry(i, worker->descname(), worker->icon());
		entries.push_back(entry);
	}
	fill_entries("workers", entries);
}

void EncyclopediaWindow::entry_selected(const std::string& key,
                                        const std::string& script_path,
                                        const Widelands::MapObjectType& type) {
	const TribeDescr& tribe = iaplayer().player().tribe();
	try {
		std::unique_ptr<LuaTable> table(iaplayer().egbase().lua().run_script(script_path));
		std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
		cr->push_arg(tribe.name());

		std::string descname = "";

		switch (type) {
		case (Widelands::MapObjectType::BUILDING):
		case (Widelands::MapObjectType::CONSTRUCTIONSITE):
		case (Widelands::MapObjectType::DISMANTLESITE):
		case (Widelands::MapObjectType::WAREHOUSE):
		case (Widelands::MapObjectType::PRODUCTIONSITE):
		case (Widelands::MapObjectType::MILITARYSITE):
		case (Widelands::MapObjectType::TRAININGSITE): {
			const Widelands::BuildingDescr* descr =
			   tribe.get_building_descr(lists_.at(key)->get_selected());
			descname = descr->descname();
			cr->push_arg(descr);
			break;
		}
		case (Widelands::MapObjectType::WARE): {
			const Widelands::WareDescr* descr = tribe.get_ware_descr(lists_.at(key)->get_selected());
			descname = descr->descname();
			cr->push_arg(descr);
			break;
		}
		case (Widelands::MapObjectType::WORKER):
		case (Widelands::MapObjectType::CARRIER):
		case (Widelands::MapObjectType::SOLDIER): {
			const Widelands::WorkerDescr* descr =
			   tribe.get_worker_descr(lists_.at(key)->get_selected());
			descname = descr->descname();
			cr->push_arg(descr);
			break;
		}
		case (Widelands::MapObjectType::MAPOBJECT):
		case (Widelands::MapObjectType::BATTLE):
		case (Widelands::MapObjectType::FLEET):
		case (Widelands::MapObjectType::BOB):
		case (Widelands::MapObjectType::CRITTER):
		case (Widelands::MapObjectType::SHIP):
		case (Widelands::MapObjectType::IMMOVABLE):
		case (Widelands::MapObjectType::FLAG):
		case (Widelands::MapObjectType::ROAD):
		case (Widelands::MapObjectType::PORTDOCK):
		default:
			throw wexception("EncyclopediaWindow: No MapObjectType defined for tab.");
		}

		cr->resume();
		const std::string help_text = cr->pop_string();
		contents_.at(key)->set_text((boost::format("%s%s") % heading(descname) % help_text).str());
	} catch (LuaError& err) {
		contents_.at(key)->set_text(err.what());
	}
	contents_.at(key)->scroll_to_top();
}
