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


bool EditorHistoryTool::add_configuration(const std::string& key, const ToolConf& conf) {

        ListItem item(key, conf);

        if (find_item(key) != tool_settings_.end()) {
                return false;
        }
        
	tool_settings_.push_front(item);
        log_dbg("Added configuration for tool %d: %s", static_cast<int>(conf.tool->get_window_id()), key.c_str());

        if (tool_settings_.size() > static_cast<long unsigned int>(MAX_SIZE)) {
                truncate();
        }

        return true;
}

std::list<ListItem>::iterator EditorHistoryTool::begin() {
        return tool_settings_.begin();
}

std::list<ListItem>::iterator EditorHistoryTool::end() {
        return tool_settings_.end();
}


const ToolConf* EditorHistoryTool::get_configuration_for(const std::string& key) {
        log_dbg("get conf for %s", key.c_str());
        auto it = find_item(key);
        if (it != tool_settings_.end()) {
                return &it->data;
        }

        return nullptr;
}

void EditorHistoryTool::remove_configuration(const std::string& key) {
        auto it = find_item(key);
        if (it != tool_settings_.end()) {
                tool_settings_.erase(it);
        }
}

void EditorHistoryTool::toggle_sticky(const std::string& key) {
        auto it = find_item(key);
        if (it != tool_settings_.end()) {
                it->sticky = !it->sticky;
        }
}

std::list<ListItem>::iterator EditorHistoryTool::find_item(const std::string& key) {
        for (auto it = tool_settings_.begin(); it != tool_settings_.end(); ++it) {
                if (it->key == key) {
                        return it;
                }
        }

        return tool_settings_.end();
}


void EditorHistoryTool::truncate() {
        int count = tool_settings_.size();
        if (count <= MAX_SIZE) {
                return;
        }
        
        for (auto it = tool_settings_.rbegin(); it != tool_settings_.rend(); ++it) {
                if (count > MAX_SIZE && !it->sticky) {
                        // remove last
                        tool_settings_.erase((++it).base());
                        count--;
                }
        }
}
