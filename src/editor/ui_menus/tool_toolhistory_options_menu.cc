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

#include "editor/ui_menus/tool_toolhistory_options_menu.h"

#include <vector>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/history.h"
#include "editor/tools/tool.h"
#include "editor/tools/toolhistory_tool.h"

inline EditorInteractive& EditorToolhistoryOptionsMenu::eia() const {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

/**
 * Create all the buttons etc...
 */
EditorToolhistoryOptionsMenu::EditorToolhistoryOptionsMenu(EditorInteractive& parent,
                                                           EditorHistoryTool& history_tool,
                                                           UI::UniqueWindow::Registry& registry)
     : EditorToolOptionsMenu(parent, registry, 500, 100, _("Tool History"), history_tool),
       history_tool_(history_tool),
       margin_(4),
       box_width_(get_inner_w() - 2 * margin_),
       box_(this, UI::PanelStyle::kWui, hmargin(), vmargin(), UI::Box::Vertical, 0, 0, vspacing()),
       list_(&box_, 0, 0, box_width_, 330, UI::PanelStyle::kWui) {

	box_.set_size(100, 20);
	box_.add(&list_);
        /** TRANSLATORS: Help tooltip in tool history window. */
        box_.set_tooltip(_("Click to select, Ctrl + Click to delete, and Shift + Click to pin an item."));
        set_center_panel(&box_);
        rebuild_list();

        list_.clicked.connect([this] {
                                      list_item_clicked(list_.get_selected());
                               });

	initialization_complete();
}


void EditorToolhistoryOptionsMenu::list_item_clicked(const std::string& selected) {

        if ((SDL_GetModState() & KMOD_CTRL) != 0) {
                history_tool_.remove_configuration(selected);
                rebuild_list();
        } else if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
                history_tool_.toggle_sticky(selected);
                rebuild_list();
        } else {
                const ToolConf* conf = history_tool_.get_configuration_for(selected);
                assert(conf != nullptr);
                eia().restore_tool_configuration(*conf);
                log_dbg("Restored: %s", selected.c_str());
        }

}


void EditorToolhistoryOptionsMenu::rebuild_list() {
	list_.clear();

        int count = 0;
        for (const auto& it: history_tool_) {
                if (it.sticky) {
                        list_.add(it.key, it.key, g_image_cache->get("images/wui/editor/sticky_list_item.png"));
                } else {
                        list_.add(it.key, it.key);
                }

                count++;
        }
}

void EditorToolhistoryOptionsMenu::update() {
        rebuild_list();
}
