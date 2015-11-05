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
const std::string heading(const std::string& text) {
	return ((boost::format("<rt><p font-size=18 font-weight=bold font-color=D1D1D1>"
	                       "%s<br></p><p font-size=8> <br></p></rt>") %
	         text).str());
}
}  // namespace

EncyclopediaWindow::EncyclopediaWindow(InteractivePlayer& parent,
													UI::UniqueWindow::Registry& registry) :
	UI::UniqueWindow(
		&parent, "encyclopedia", &registry, WINDOW_WIDTH, WINDOW_HEIGHT, _("Tribal Encyclopedia")),
     tabs_(this, 0, 0, nullptr) {
	std::set<std::string> keys({"buildings", "wares", "workers"});

	const int contents_height = WINDOW_HEIGHT - kTabHeight - 2 * kPadding;
	const int contents_width = WINDOW_WIDTH / 2 - 1.5 * kPadding;

	for (const std::string& key : keys) {
		wrapper_boxes_.insert(std::make_pair(
		   key, std::unique_ptr<UI::Box>(new UI::Box(&tabs_, 0, 0, UI::Box::Horizontal))));

		boxes_.insert(std::make_pair(key,
		                             std::unique_ptr<UI::Box>(new UI::Box(
		                                wrapper_boxes_.at(key).get(), 0, 0, UI::Box::Horizontal))));

		lists_.insert(
		   std::make_pair(key,
		                  std::unique_ptr<UI::Listselect<Widelands::WareIndex>>(
		                     new UI::Listselect<Widelands::WareIndex>(
		                        boxes_.at(key).get(), 0, 0, contents_width, contents_height))));

		contents_.insert(
		   std::make_pair(key,
		                  std::unique_ptr<UI::MultilineTextarea>(new UI::MultilineTextarea(
		                     boxes_.at(key).get(), 0, 0, contents_width, contents_height))));

		boxes_.at(key)->add(lists_.at(key).get(), UI::Align_Left);
		boxes_.at(key)->add_space(kPadding);
		boxes_.at(key)->add(contents_.at(key).get(), UI::Align_Left);

		wrapper_boxes_.at(key)->add_space(kPadding);
		wrapper_boxes_.at(key)->add(boxes_.at(key).get(), UI::Align_Left);
	}

	tabs_.add("encyclopedia_wares",
	          g_gr->images().get("pics/genstats_nrwares.png"),
	          wrapper_boxes_.at("wares").get(),
	          _("Wares"));
	tabs_.add("encyclopedia_workers",
	          g_gr->images().get("pics/genstats_nrworkers.png"),
	          wrapper_boxes_.at("workers").get(),
	          _("Workers"));
	tabs_.add("encyclopedia_buildings",
	          g_gr->images().get("pics/genstats_nrbuildings.png"),
	          wrapper_boxes_.at("buildings").get(),
	          _("Buildings"));
	tabs_.set_size(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Content
	lists_.at("buildings")
	   ->selected.connect(boost::bind(&EncyclopediaWindow::building_selected, this, _1));
	lists_.at("wares")->selected.connect(boost::bind(&EncyclopediaWindow::ware_selected, this, _1));
	lists_.at("workers")
	   ->selected.connect(boost::bind(&EncyclopediaWindow::worker_selected, this, _1));

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
		lists_.at(key)->add(cur.descname_, cur.index_, cur.icon_);
	}
	lists_.at(key)->select(0);
}

void EncyclopediaWindow::fill_buildings() {
	const Tribes& tribes = iaplayer().egbase().tribes();
	const TribeDescr& tribe = iaplayer().player().tribe();
	std::vector<EncyclopediaEntry> entries;

	for (BuildingIndex i = 0; i < tribes.nrbuildings(); ++i) {
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

	for (const WareIndex& i : tribe.wares()) {
		const WareDescr* ware = tribe.get_ware_descr(i);
		EncyclopediaEntry entry(i, ware->descname(), ware->icon());
		entries.push_back(entry);
	}
	fill_entries("wares", entries);
}

void EncyclopediaWindow::fill_workers() {
	const TribeDescr& tribe = iaplayer().player().tribe();
	std::vector<EncyclopediaEntry> entries;

	for (const WareIndex& i : tribe.workers()) {
		const WorkerDescr* worker = tribe.get_worker_descr(i);
		EncyclopediaEntry entry(i, worker->descname(), worker->icon());
		entries.push_back(entry);
	}
	fill_entries("workers", entries);
}

template <typename T>
void EncyclopediaWindow::entry_selected(const Widelands::TribeDescr& tribe,
                                        const T& map_object,
                                        const char* tab,
                                        const char* script_name) {
	try {
		std::unique_ptr<LuaTable> t(iaplayer().egbase().lua().run_script(script_name));
		std::unique_ptr<LuaCoroutine> cr(t->get_coroutine("func"));
		cr->push_arg(tribe.name());
		cr->push_arg(&map_object);
		cr->resume();
		const std::string help_text = cr->pop_string();
		contents_.at(tab)
		   ->set_text((boost::format("%s%s") % heading(map_object.descname()) % help_text).str());
	} catch (LuaError& err) {
		contents_.at(tab)->set_text(err.what());
	}
	contents_.at(tab)->scroll_to_top();
}


void EncyclopediaWindow::building_selected(uint32_t) {
	const TribeDescr& tribe = iaplayer().player().tribe();
	entry_selected<Widelands::BuildingDescr>(
	   tribe,
	   *tribe.get_building_descr(lists_.at("buildings")->get_selected()),
	   "buildings",
	   "tribes/scripting/help/building_help.lua");
}

void EncyclopediaWindow::ware_selected(uint32_t) {
	const TribeDescr& tribe = iaplayer().player().tribe();
	entry_selected<Widelands::WareDescr>(tribe,
	                                     *tribe.get_ware_descr(lists_.at("wares")->get_selected()),
	                                     "wares",
	                                     "tribes/scripting/help/ware_help.lua");
}

void EncyclopediaWindow::worker_selected(uint32_t) {
	const TribeDescr& tribe = iaplayer().player().tribe();
	entry_selected<Widelands::WorkerDescr>(
	   tribe,
	   *tribe.get_worker_descr(lists_.at("workers")->get_selected()),
	   "workers",
	   "tribes/scripting/help/worker_help.lua");
}
