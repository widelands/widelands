/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "editor/ui_menus/tool_place_critter_options_menu.h"

#include <SDL_keycode.h>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/place_critter_tool.h"
#include "graphic/graphic.h"
#include "logic/map.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/world.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"
#include "wlapplication.h"

namespace {
UI::Checkbox* create_critter_checkbox(UI::Panel* parent, const Widelands::CritterDescr& critter_descr) {
	const Image* pic = critter_descr.representative_image();
	UI::Checkbox* cb = new UI::Checkbox(parent, Vector2i(0, 0), pic, critter_descr.descname());
	const int kMinClickableArea = 24;
	cb->set_desired_size(std::max<int>(pic->width(), kMinClickableArea),
	                     std::max<int>(pic->height(), kMinClickableArea));
	return cb;
}

}  // namespace

EditorToolPlaceCritterOptionsMenu::EditorToolPlaceCritterOptionsMenu(
   EditorInteractive& parent, EditorPlaceCritterTool& tool, UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 0, 0, _("Animals")) {
	const Widelands::World& world = parent.egbase().world();
	multi_select_menu_.reset(
	   new CategorizedItemSelectionMenu<Widelands::CritterDescr, EditorPlaceCritterTool>(
	      this, world.editor_critter_categories(), world.critters(),
			[this](UI::Panel* cb_parent, const Widelands::CritterDescr& critter_descr) {
		      return create_critter_checkbox(cb_parent, critter_descr);
		   },
	      [this] { select_correct_tool(); }, &tool));
	set_center_panel(multi_select_menu_.get());
}

EditorToolPlaceCritterOptionsMenu::~EditorToolPlaceCritterOptionsMenu() {
}
