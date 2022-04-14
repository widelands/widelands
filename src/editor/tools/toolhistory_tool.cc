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


bool EditorHistoryTool::add_configuration(const std::string& name, const ToolConf& conf) {



	const auto ret = tool_settings_.insert(std::make_pair(name, conf));
        const bool success = ret.second;

        log_dbg("Added configuration for tool %d: %s", static_cast<int>(conf.tool->get_tool_id()), name.c_str());

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

void EditorHistoryTool::remove_configuration(const std::string& key) {
        tool_settings_.erase(key);
}
