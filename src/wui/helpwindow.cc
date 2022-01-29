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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wui/helpwindow.h"

#include <memory>

#include "base/i18n.h"
#include "graphic/text_layout.h"
#include "logic/map_objects/tribes/building.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

namespace UI {

BuildingHelpWindow::BuildingHelpWindow(Panel* const parent,
                                       UI::UniqueWindow::Registry& reg,
                                       const Widelands::BuildingDescr& building_description,
                                       const Widelands::TribeDescr& tribe,
                                       LuaInterface* const lua,
                                       uint32_t width,
                                       uint32_t height)
   : UI::UniqueWindow(parent,
                      UI::WindowStyle::kWui,
                      "help_window",
                      &reg,
                      width,
                      height,
                      format(_("Help: %s"), building_description.descname())),
     textarea_(new MultilineTextarea(this, 5, 5, width - 10, height - 10, UI::PanelStyle::kWui)) {
	assert(tribe.has_building(tribe.building_index(building_description.name())) ||
	       building_description.type() == Widelands::MapObjectType::MILITARYSITE);
	try {
		std::unique_ptr<LuaTable> t(lua->run_script("tribes/scripting/help/building_help.lua"));
		std::unique_ptr<LuaCoroutine> cr(t->get_coroutine("func"));
		cr->push_arg(tribe.name());
		cr->push_arg(building_description.name());
		cr->resume();
		std::unique_ptr<LuaTable> return_table = cr->pop_table();
		return_table->do_not_warn_about_unaccessed_keys();  // We won't display the title here
		textarea_->set_text(as_richtext(return_table->get_string("text")));
	} catch (LuaError& err) {
		textarea_->set_text(err.what());
	}

	initialization_complete();
}

}  // namespace UI
