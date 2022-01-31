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

#ifndef WL_WUI_ENCYCLOPEDIA_WINDOW_H
#define WL_WUI_ENCYCLOPEDIA_WINDOW_H

#include <memory>

#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/box.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/unique_window.h"

class InteractiveBase;

namespace UI {

struct EncyclopediaWindow : public UI::UniqueWindow {
	EncyclopediaWindow(InteractiveBase&, UI::UniqueWindow::Registry&, LuaInterface* const lua);

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kEncyclopedia;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

protected:
	void init(InteractiveBase& parent, std::unique_ptr<LuaTable> table);

	LuaInterface* const lua_;

private:
	struct EncyclopediaEntry {
		EncyclopediaEntry(const std::string& init_script_path,
		                  const std::vector<std::string>& init_script_parameters)
		   : script_path(init_script_path), script_parameters(init_script_parameters) {
		}
		const std::string script_path;
		const std::vector<std::string> script_parameters;
	};

	// Update contents when an entry is selected
	void entry_selected(const std::string& tab_name);

	// UI elements
	UI::TabPanel tabs_;

	// Wrapper boxes so we can add some padding
	std::map<std::string, std::unique_ptr<UI::Box>> wrapper_boxes_;
	// Main contents boxes for each tab
	std::map<std::string, std::unique_ptr<UI::Box>> boxes_;
	// A tab's table of contents
	std::map<std::string, std::unique_ptr<UI::Listselect<EncyclopediaEntry>>> lists_;
	// The contents shown when an entry is selected in a tab
	std::map<std::string, std::unique_ptr<UI::MultilineTextarea>> contents_;
};

}  // namespace UI

#endif  // end of include guard: WL_WUI_ENCYCLOPEDIA_WINDOW_H
