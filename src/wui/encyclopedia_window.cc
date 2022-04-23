/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/encyclopedia_window.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "graphic/graphic.h"
#include "graphic/text_layout.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "scripting/lua_coroutine.h"
#include "ui_basic/messagebox.h"
#include "wui/interactive_player.h"

namespace {

#define WINDOW_WIDTH std::min(700, g_gr->get_xres() - 40)
#define WINDOW_HEIGHT std::min(550, g_gr->get_yres() - 40)

constexpr int kPadding = 5;
constexpr int kTabHeight = 35;

}  // namespace

namespace UI {

EncyclopediaWindow::EncyclopediaWindow(InteractiveBase& parent,
                                       UI::UniqueWindow::Registry& registry,
                                       LuaInterface* const lua)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "encyclopedia", &registry, WINDOW_WIDTH, WINDOW_HEIGHT, ""),
     lua_(lua),
     tabs_(this, UI::TabPanelStyle::kWuiLight) {
}

static const std::string kTabNamePrefix = "encyclopedia_";

void EncyclopediaWindow::init(InteractiveBase& parent, std::unique_ptr<LuaTable> table) {

	const int contents_height = WINDOW_HEIGHT - kTabHeight - 2 * kPadding;
	const int contents_width = WINDOW_WIDTH / 2.f - 1.5f * kPadding;

	try {
		set_title(table->get_string("title"));

		// Read tab definitions
		std::unique_ptr<LuaTable> tabs_table = table->get_table("tabs");
		for (const auto& tab_table : tabs_table->array_entries<std::unique_ptr<LuaTable>>()) {
			const std::string tab_name = tab_table->get_string("name");
			const std::string tab_icon =
			   tab_table->has_key("icon") ? tab_table->get_string("icon") : "";
			const std::string tab_title = tab_table->get_string("title");

			wrapper_boxes_.insert(
			   std::make_pair(tab_name, std::unique_ptr<UI::Box>(new UI::Box(
			                               &tabs_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal))));

			boxes_.insert(std::make_pair(
			   tab_name, std::unique_ptr<UI::Box>(new UI::Box(wrapper_boxes_.at(tab_name).get(),
			                                                  UI::PanelStyle::kWui, 0, 0,
			                                                  UI::Box::Horizontal))));

			lists_.insert(std::make_pair(
			   tab_name, std::unique_ptr<UI::Listselect<EncyclopediaEntry>>(
			                new UI::Listselect<EncyclopediaEntry>(boxes_.at(tab_name).get(), 0, 0,
			                                                      contents_width, contents_height,
			                                                      UI::PanelStyle::kWui))));
			lists_.at(tab_name)->selected.connect(
			   [this, tab_name](unsigned /* index */) { entry_selected(tab_name); });

			contents_.insert(std::make_pair(
			   tab_name, std::unique_ptr<UI::MultilineTextarea>(
			                new UI::MultilineTextarea(boxes_.at(tab_name).get(), 0, 0, contents_width,
			                                          contents_height, UI::PanelStyle::kWui))));

			boxes_.at(tab_name)->add(lists_.at(tab_name).get());
			boxes_.at(tab_name)->add_space(kPadding);
			boxes_.at(tab_name)->add(contents_.at(tab_name).get());

			wrapper_boxes_.at(tab_name)->add_space(kPadding);
			wrapper_boxes_.at(tab_name)->add(boxes_.at(tab_name).get());

			if (tab_icon.empty()) {
				tabs_.add(kTabNamePrefix + tab_name, tab_title, wrapper_boxes_.at(tab_name).get());
			} else if (g_fs->file_exists(tab_icon)) {
				tabs_.add(kTabNamePrefix + tab_name, g_image_cache->get(tab_icon),
				          wrapper_boxes_.at(tab_name).get(), tab_title);
			} else {
				throw wexception(
				   "Icon path '%s' for tab '%s' does not exist!", tab_icon.c_str(), tab_name.c_str());
			}

			// Now fill the lists
			std::unique_ptr<LuaTable> entries_table = tab_table->get_table("entries");
			for (const auto& entry_table : entries_table->array_entries<std::unique_ptr<LuaTable>>()) {
				const std::string entry_name = entry_table->get_string("name");
				const std::string entry_title = entry_table->get_string("title");
				const std::string entry_icon =
				   entry_table->has_key("icon") ? entry_table->get_string("icon") : "";
				const std::string entry_script = entry_table->get_string("script");

				// Make sure that all paths exist
				if (!g_fs->file_exists(entry_script)) {
					throw wexception("Script path %s for entry %s does not exist!", entry_script.c_str(),
					                 entry_name.c_str());
				}

				EncyclopediaEntry entry(
				   entry_script,
				   entry_table->get_table("script_parameters")->array_entries<std::string>());

				if (entry_icon.empty()) {
					lists_.at(tab_name)->add(entry_title, entry);
				} else if (g_fs->file_exists(entry_icon)) {
					lists_.at(tab_name)->add(entry_title, entry, g_image_cache->get(entry_icon));
				} else {
					throw wexception("Icon path '%s' for tab entry '%s' does not exist!",
					                 entry_icon.c_str(), entry_name.c_str());
				}
			}
		}
	} catch (WException& err) {
		log_err_time(parent.egbase().get_gametime(), "Error loading script for encyclopedia:\n%s\n",
		             err.what());
		UI::WLMessageBox wmb(&parent, UI::WindowStyle::kWui, _("Error!"),
		                     format("Error loading script for encyclopedia:\n%s", err.what()),
		                     UI::WLMessageBox::MBoxType::kOk);
		wmb.run<UI::Panel::Returncodes>();
	}

	for (const auto& list : lists_) {
		if (!(list.second->empty())) {
			list.second->select(0);
		}
	}

	tabs_.set_size(WINDOW_WIDTH, WINDOW_HEIGHT);

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void EncyclopediaWindow::entry_selected(const std::string& tab_name) {
	const EncyclopediaEntry& entry = lists_.at(tab_name)->get_selected();
	try {
		std::unique_ptr<LuaTable> table(lua_->run_script(entry.script_path));
		if (!entry.script_parameters.empty()) {
			std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
			for (const std::string& parameter : entry.script_parameters) {
				cr->push_arg(parameter);
			}
			cr->resume();
			table = cr->pop_table();
		}
		contents_.at(tab_name)->set_text(
		   as_message(table->get_string("title"), table->get_string("text")));
	} catch (LuaError& err) {
		contents_.at(tab_name)->set_text(err.what());
	}
	contents_.at(tab_name)->scroll_to_top();
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& EncyclopediaWindow::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r = dynamic_cast<InteractivePlayer&>(ib).encyclopedia_;
			r.create();
			assert(r.window);
			EncyclopediaWindow& m = dynamic_cast<EncyclopediaWindow&>(*r.window);
			const std::string tab = fr.string();
			m.tabs_.activate(kTabNamePrefix + tab);

			const std::string entry_path = fr.string();
			std::vector<std::string> entry_params;
			for (size_t i = fr.unsigned_32(); i != 0u; --i) {
				entry_params.push_back(fr.string());
			}
			const size_t nr_entries = m.lists_.at(tab)->size();
			for (size_t i = 0; i < nr_entries; ++i) {
				if ((*m.lists_.at(tab))[i].script_path == entry_path &&
				    (*m.lists_.at(tab))[i].script_parameters == entry_params) {
					m.lists_.at(tab)->select(i);
					return m;
				}
			}
			log_warn("EncyclopediaWindow::load: could not find a suitable list entry for '%s' in '%s'",
			         entry_path.c_str(), tab.c_str());
			return m;
		}
		throw Widelands::UnhandledVersionError("Encyclopedia", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("encyclopedia: %s", e.what());
	}
}
void EncyclopediaWindow::save(FileWrite& fw, Widelands::MapObjectSaver& /* mos */) const {
	fw.unsigned_16(kCurrentPacketVersion);

	const std::string tab = tabs_.tabs()[tabs_.active()]->get_name().substr(kTabNamePrefix.size());
	fw.string(tab);

	const EncyclopediaEntry& e = lists_.at(tab)->get_selected();
	fw.string(e.script_path);
	fw.unsigned_32(e.script_parameters.size());
	for (const std::string& str : e.script_parameters) {
		fw.string(str);
	}
}

}  // namespace UI
