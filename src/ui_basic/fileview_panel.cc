/*
 * Copyright (C) 2016-2021 by the Widelands Development Team
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

#include "ui_basic/fileview_panel.h"

#include <memory>

#include "base/macros.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

namespace UI {

FileViewPanel::FileViewPanel(Panel* parent, TabPanelStyle background_style)
   : TabPanel(parent, background_style), padding_(5), contents_width_(0), contents_height_(0) {
	layout();

	sigclicked.connect([this]() { load_tab_contents(); });
}

void FileViewPanel::load_tab_contents() {
	const unsigned index = active();
	const std::string& lua_script = script_paths_.at(index);
	std::string contents;
	if (!lua_script.empty()) {
		try {
			LuaInterface lua;
			std::unique_ptr<LuaTable> t(lua.run_script(lua_script));
			contents = t->get_string(1);
		} catch (LuaError& err) {
			contents = err.what();
		}
		textviews_.at(index)->set_text(contents);
		script_paths_.at(index) = "";
	}
}

void FileViewPanel::add_tab(const std::string& title, const std::string& lua_script) {
	script_paths_.push_back(lua_script);
	boxes_.push_back(std::unique_ptr<UI::Box>(
	   new UI::Box(this, panel_style_, 0, 0, UI::Box::Vertical, 0, 0, padding_)));
	size_t index = boxes_.size() - 1;

	UI::MultilineTextarea* textarea =
	   new UI::MultilineTextarea(boxes_.at(index).get(), 0, 0, Scrollbar::kSize, 0, panel_style_);

	textviews_.push_back(std::unique_ptr<UI::MultilineTextarea>(textarea));
	add(bformat("about_%" PRIuS, index), title, boxes_.at(index).get(), "");

	assert(boxes_.size() == textviews_.size());
	assert(tabs().size() == textviews_.size());
	update_tab_size(index);
}

void FileViewPanel::add_tab_without_script(const std::string& name,
                                           const std::string& title,
                                           Panel* panel,
                                           const std::string& tooltip) {
	script_paths_.push_back("");
	boxes_.push_back(nullptr);
	textviews_.push_back(nullptr);
	add(name, title, panel, tooltip);
}

void FileViewPanel::update_tab_size(size_t index) {
	assert(get_inner_w() >= 0 && get_inner_h() >= 0);
	assert(contents_width_ >= 0 && contents_height_ >= 0);

	if (boxes_.at(index).get() != nullptr) {
		boxes_.at(index)->set_size(get_inner_w(), get_inner_h());
	}
	if (textviews_.at(index).get() != nullptr) {
		textviews_.at(index)->set_size(contents_width_, contents_height_);
	} else {
		tabs().at(index)->panel->set_size(contents_width_, contents_height_);
	}
}

void FileViewPanel::layout() {
	assert(boxes_.size() == textviews_.size());
	assert(get_inner_w() >= 0 && get_inner_h() >= 0);

	// If there is a border, we have less space for the contents
	contents_width_ = std::max(0, get_w() - 2 * padding_);

	contents_height_ = std::max(0, panel_style_ == UI::PanelStyle::kFsMenu ?
                                     get_inner_h() - 2 * padding_ - UI::kTabPanelButtonHeight :
                                     get_inner_h() - 3 * padding_ - UI::kTabPanelButtonHeight);

	for (size_t i = 0; i < boxes_.size(); ++i) {
		update_tab_size(i);
	}
}

}  // namespace UI
