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

#ifndef WL_WUI_HELPWINDOW_H
#define WL_WUI_HELPWINDOW_H

#include <memory>

#include "logic/map_objects/tribes/tribe_descr.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/unique_window.h"

class LuaInterface;

namespace UI {

/*
 * This HelpWindow takes a Lua script, runs it and displays it's formatted content
 * as a static text
 */
class BuildingHelpWindow : public UI::UniqueWindow {
public:
	BuildingHelpWindow(Panel* parent,
	                   UI::UniqueWindow::Registry& reg,
	                   const Widelands::BuildingDescr& building_description,
	                   const Widelands::TribeDescr& tribe,
	                   LuaInterface* const lua,
	                   uint32_t width = 300,
	                   uint32_t height = 400);

private:
	std::unique_ptr<MultilineTextarea> textarea_;
};

}  // namespace UI

#endif  // end of include guard: WL_WUI_HELPWINDOW_H
