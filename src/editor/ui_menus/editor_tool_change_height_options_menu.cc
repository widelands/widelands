/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "editor/ui_menus/editor_tool_change_height_options_menu.h"

#include <cstdio>
#include <string>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/editor_increase_height_tool.h"
#include "editor/tools/editor_set_height_tool.h"
#include "graphic/graphic.h"
#include "logic/widelands_geometry.h"
#include "ui_basic/button.h"

#define width  20
#define height 20
EditorToolChangeHeightOptionsMenu::EditorToolChangeHeightOptionsMenu
	(EditorInteractive          & parent,
	 EditorIncreaseHeightTool & increase_tool,
	 UI::UniqueWindow::Registry  & registry)
	:
	EditorToolOptionsMenu
		(parent, registry, 250, 135, _("Height Tools Options")),
	increase_tool_(increase_tool),
	change_by_label_
		(this,
		 hmargin(), vmargin(), get_inner_w() - 2 * hmargin(), height,
		 _("Increase/Decrease Value"), UI::Align::kBottomCenter),
	change_by_increase_
		(this, "incr_change_by",
		 get_inner_w() - hmargin() - width,
		 change_by_label_.get_y() + change_by_label_.get_h() + spacing(),
		 width, height,
		 g_gr->images().get("images/ui_basic/but1.png"),
		 g_gr->images().get("images/ui_basic/scrollbar_up.png"),
		 std::string(),
		 increase_tool.get_change_by() < MAX_FIELD_HEIGHT_DIFF),
	change_by_decrease_
		(this, "decr_change_by",
		 hmargin(),
		 change_by_increase_.get_y(),
		 width, height,
		 g_gr->images().get("images/ui_basic/but1.png"),
		 g_gr->images().get("images/ui_basic/scrollbar_down.png"),
		 std::string(),
		 1 < increase_tool.get_change_by()),
	change_by_value_
		(this,
		 change_by_increase_.get_x() + change_by_increase_.get_w() +
		 hspacing(),
		 change_by_increase_.get_y(),
		 change_by_decrease_.get_x() - hspacing()
		 -
		 (change_by_increase_.get_x() + change_by_increase_.get_w() +
		  hspacing()),
		 height,
		 UI::Align::kBottomCenter),
	set_to_label_
		(this,
		 vmargin(),
		 change_by_increase_.get_y() + change_by_increase_.get_h() +
		 vspacing(),
		 get_inner_w() - 2 * hmargin(), height,
		 _("Set Value"), UI::Align::kBottomCenter),
	set_to_increase_
		(this, "incr_set_to",
		 change_by_increase_.get_x(),
		 set_to_label_.get_y() + set_to_label_.get_h() + vspacing(),
		 width, height,
		 g_gr->images().get("images/ui_basic/but1.png"),
		 g_gr->images().get("images/ui_basic/scrollbar_up.png"),
		 std::string(),
		 increase_tool.set_tool().get_interval().min < MAX_FIELD_HEIGHT),
	set_to_decrease_
		(this, "decr_set_to",
		 hmargin(),
		 set_to_increase_.get_y(),
		 width, height,
		 g_gr->images().get("images/ui_basic/but1.png"),
		 g_gr->images().get("images/ui_basic/scrollbar_down.png"),
		 std::string(),
		 0 < increase_tool.set_tool().get_interval().min),
	set_to_value_
		(this,
		 change_by_value_.get_x(), set_to_increase_.get_y(),
		 change_by_value_.get_w(), height,
		 UI::Align::kBottomCenter)
{
	change_by_increase_.sigclicked.connect
		(boost::bind
		 (&EditorToolChangeHeightOptionsMenu::clicked_change_by_increment, boost::ref(*this)));
	change_by_decrease_.sigclicked.connect
		(boost::bind
		 (&EditorToolChangeHeightOptionsMenu::clicked_change_by_decrement, boost::ref(*this)));
	set_to_increase_.sigclicked.connect
		(boost::bind(&EditorToolChangeHeightOptionsMenu::clicked_setto_increment, boost::ref(*this)));
	set_to_decrease_.sigclicked.connect
		(boost::bind(&EditorToolChangeHeightOptionsMenu::clicked_setto_decrement, boost::ref(*this)));

	change_by_increase_.set_repeating(true);
	change_by_decrease_.set_repeating(true);
	set_to_increase_   .set_repeating(true);
	set_to_decrease_   .set_repeating(true);
	update();
}


void EditorToolChangeHeightOptionsMenu::clicked_change_by_decrement() {
	int32_t change_by = increase_tool_.get_change_by();
	assert(change_by == increase_tool_.decrease_tool().get_change_by());
	assert(1 < change_by);

	--change_by;

	increase_tool_                .set_change_by(change_by);
	increase_tool_.decrease_tool().set_change_by(change_by);
	change_by_decrease_.set_enabled(1 < change_by);
	change_by_increase_.set_enabled(true);
	select_correct_tool();
	update();
}


void EditorToolChangeHeightOptionsMenu::clicked_change_by_increment() {
	int32_t change_by = increase_tool_.get_change_by();
	assert(change_by == increase_tool_.decrease_tool().get_change_by());
	assert(change_by < MAX_FIELD_HEIGHT_DIFF);

	++change_by;

	increase_tool_                .set_change_by(change_by);
	increase_tool_.decrease_tool().set_change_by(change_by);
	change_by_decrease_.set_enabled(true);
	change_by_increase_.set_enabled(change_by < MAX_FIELD_HEIGHT_DIFF);
	select_correct_tool();
	update();
}


void EditorToolChangeHeightOptionsMenu::clicked_setto_decrement() {
	Widelands::Field::Height setto =
		increase_tool_.set_tool().get_interval().min;
	assert(setto == increase_tool_.set_tool().get_interval().max);
	assert(0 < setto);

	--setto;

	increase_tool_.set_tool().set_interval
		(Widelands::HeightInterval(setto, setto));
	set_to_decrease_.set_enabled(0 < setto);
	set_to_increase_.set_enabled(true);
	select_correct_tool();
	update();
}


void EditorToolChangeHeightOptionsMenu::clicked_setto_increment() {
	Widelands::Field::Height setto =
		increase_tool_.set_tool().get_interval().min;
	assert(setto == increase_tool_.set_tool().get_interval().max);
	assert(setto < MAX_FIELD_HEIGHT);

	++setto;

	increase_tool_.set_tool().set_interval
		(Widelands::HeightInterval(setto, setto));
	set_to_decrease_.set_enabled(true);
	set_to_increase_.set_enabled(setto < MAX_FIELD_HEIGHT);
	select_correct_tool();
	update();
}


/// Update all the textareas, so that they represent the correct values.
void EditorToolChangeHeightOptionsMenu::update() {
	change_by_value_.set_text(std::to_string(increase_tool_.get_change_by()));

	set_to_value_.set_text(std::to_string(
										static_cast<unsigned int>(increase_tool_.set_tool().get_interval().min)));
}
