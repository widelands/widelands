/*
 * Copyright (C) 2015-2021 by the Widelands Development Team
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

#include "editor/ui_menus/help.h"

#include "base/i18n.h"
#include "base/log.h"
#include "editor/editorinteractive.h"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"

EditorHelp::EditorHelp(EditorInteractive& parent,
                       UI::UniqueWindow::Registry& registry,
                       LuaInterface* const lua)
   : EncyclopediaWindow(parent, registry, lua) {
	try {
		init(parent, lua_->run_script("scripting/editor/editor_help.lua"));
	} catch (LuaError& err) {
		log_err("Error loading script for editor help:\n%s\n", err.what());
		UI::WLMessageBox wmb(&parent, UI::WindowStyle::kWui, _("Error!"),
		                     format("Error loading script for editor help:\n%s", err.what()),
		                     UI::WLMessageBox::MBoxType::kOk);
		wmb.run<UI::Panel::Returncodes>();
	}

	initialization_complete();
}
