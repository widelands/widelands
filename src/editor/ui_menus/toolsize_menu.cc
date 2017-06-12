/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "editor/ui_menus/toolsize_menu.h"

#include <cstdio>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"
#include "graphic/graphic.h"

inline EditorInteractive& EditorToolsizeMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

/**
 * Create all the buttons etc...
*/
EditorToolsizeMenu::EditorToolsizeMenu(EditorInteractive& parent,
                                       UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent, "toolsize_menu", &registry, 250, 50, _("Tool Size")),
     textarea_(this, 5, 10, 240, 10, std::string(), UI::Align::kCenter),
     increase_(this,
               "incr",
               get_inner_w() / 2 - 10,
               25,
               20,
               20,
               UI::ButtonStyle::kWuiSecondary,
               g_gr->images().get("images/ui_basic/scrollbar_up.png")),
     decrease_(this,
               "decr",
               get_inner_w() / 2 + 10,
               25,
               20,
               20,
               UI::ButtonStyle::kWuiSecondary,
               g_gr->images().get("images/ui_basic/scrollbar_down.png")),
     value_(0) {
	increase_.sigclicked.connect(
	   boost::bind(&EditorToolsizeMenu::increase_radius, boost::ref(*this)));
	decrease_.sigclicked.connect(
	   boost::bind(&EditorToolsizeMenu::decrease_radius, boost::ref(*this)));

	increase_.set_repeating(true);
	decrease_.set_repeating(true);
	update(parent.get_sel_radius());

	if (eia().tools()->current().has_size_one()) {
		set_buttons_enabled(false);
	}

	if (get_usedefaultpos())
		center_to_parent();
}

void EditorToolsizeMenu::update(uint32_t const val) {
	value_ = val;
	eia().set_sel_radius(val);
	set_buttons_enabled(true);
	textarea_.set_text((boost::format(_("Current Size: %u")) % (val + 1)).str());
}

void EditorToolsizeMenu::set_buttons_enabled(bool enable) {
	decrease_.set_enabled(enable && 0 < value_);
	increase_.set_enabled(enable && value_ < MAX_TOOL_AREA);
}

void EditorToolsizeMenu::decrease_radius() {
	assert(0 < eia().get_sel_radius());
	update(eia().get_sel_radius() - 1);
}
void EditorToolsizeMenu::increase_radius() {
	assert(eia().get_sel_radius() < MAX_TOOL_AREA);
	update(eia().get_sel_radius() + 1);
}
