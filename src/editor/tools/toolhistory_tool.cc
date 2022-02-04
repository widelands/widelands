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
                                          const ToolConf& conf,
                                          EditorInteractive&) {

        std::string str = primary.format_conf_string(idx, conf);

	const auto ret = tool_settings_.insert(std::make_pair(str, conf));
        const bool success = ret.second;

        log_dbg("added configuration, primary %d, change by %d", static_cast<int>(conf.toolId), conf.change_by);
        log_dbg("insert success? %i", static_cast<int>(success));
        
        return success;
}


const std::vector<std::string>&
EditorHistoryTool::get_list() {
        keys_.clear();
        
        for (auto it = tool_settings_.rbegin(); it != tool_settings_.rend(); ++it) {
                keys_.push_back(it->first);
        }

        return keys_;
}

const ToolConf* EditorHistoryTool::get_configuration_for(const std::string& key) {
        if (tool_settings_.count(key) == 1) {
                return &tool_settings_[key];
        }

        return nullptr;
}
