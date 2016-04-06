/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_TOOL_NOISE_HEIGHT_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_TOOL_NOISE_HEIGHT_OPTIONS_MENU_H

#include "editor/ui_menus/tool_options_menu.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"

class EditorInteractive;
struct EditorNoiseHeightTool;

struct EditorToolNoiseHeightOptionsMenu : public EditorToolOptionsMenu {
	EditorToolNoiseHeightOptionsMenu
		(EditorInteractive         &,
		 EditorNoiseHeightTool   &,
		 UI::UniqueWindow::Registry &);

private:
	EditorNoiseHeightTool& noise_tool_;
	UI::Textarea lower_label_, upper_label_;
	UI::Button lower_decrease_, lower_increase_, upper_decrease_, upper_increase_;
	UI::Textarea set_label_;
	UI::Button setto_decrease_, setto_increase_;

	void clicked_lower_decrease();
	void clicked_lower_increase();
	void clicked_upper_decrease();
	void clicked_upper_increase();
	void clicked_setto_decrease();
	void clicked_setto_increase();
	void update();
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOL_NOISE_HEIGHT_OPTIONS_MENU_H
