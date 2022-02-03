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

#include "editor/tools/toolhistory_tool.h"

#include <sstream>

#include "editor/editorinteractive.h"
#include "editor/tools/action_args.h"
#include "editor/tools/tool_action.h"
#include "editor/ui_menus/tool_options_menu.h"
#include "editor/ui_menus/tool_toolhistory_options_menu.h"        
#include "ui_basic/unique_window.h"

/// Sets the heights to random values. Changes surrounding nodes if necessary.
// int32_t EditorHistoryTool::handle_click_impl(const Widelands::NodeAndTriangle<>&,
//                                                  EditorInteractive&,
//                                                  EditorActionArgs*,
//                                                  Widelands::Map*) {
// 	return 0;
// }
        
        
bool EditorHistoryTool::add_configuration(EditorTool& primary,
                                   EditorTool::ToolIndex idx,
                                   EditorInteractive& parent) {

        EditorActionArgs args = primary.format_args(idx, parent);
        std::string str = primary.format_args_string(idx, parent);

	const auto ret = tool_settings_.insert({str, &args});
        const bool success = ret.second;
        if (success) {
                //keys_.push_front(str);
        }
        
        return success;
}


std::vector<std::string>&
EditorHistoryTool::get_list() {
        keys_.clear();
        
        for (auto it = tool_settings_.begin(); it != tool_settings_.end(); ++it) {
                keys_.push_back(it->first);
        }

        return keys_;
}

EditorActionArgs* EditorHistoryTool::get_configuration(std::string& key) {
        return tool_settings_[key];
}
