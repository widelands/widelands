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

#include "editor/ui_menus/tool_noise_height_options_menu.h"

#include <cstdio>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/decrease_height_tool.h"
#include "editor/tools/increase_height_tool.h"
#include "editor/tools/noise_height_tool.h"
#include "graphic/graphic.h"
#include "logic/widelands_geometry.h"


using Widelands::Field;

#define width  20
#define height 20
EditorToolNoiseHeightOptionsMenu::EditorToolNoiseHeightOptionsMenu
	(EditorInteractive         & parent,
	 EditorNoiseHeightTool   & noise_tool,
	 UI::UniqueWindow::Registry & registry)
	:
	EditorToolOptionsMenu
		(parent, registry, 250, 3 * height + 4 * vspacing() + 2 * vmargin(), _("Noise Height Options")),
	noise_tool_(noise_tool),
	lower_label_
		(this,
		 hmargin(),
		 vmargin(),
		 width, height,
		 UI::Align::kLeft),
	upper_label_
		(this,
		 hmargin(),
		 lower_label_.get_y() + lower_label_.get_h() + 2 * vspacing(),
		 width, height,
		 UI::Align::kLeft),
	lower_decrease_
		(this, "decr_lower",
		 get_inner_w() - 2 * width - hspacing(),
		 lower_label_.get_y(),
		 width, height,
		 g_gr->images().get("images/ui_basic/but0.png"),
		 g_gr->images().get("images/ui_basic/scrollbar_down.png"),
		 std::string(),
		 0 < noise_tool.get_interval().min),
	lower_increase_
		(this, "incr_lower",
		 get_inner_w() - width - hspacing(),
		 lower_label_.get_y(),
		 width, height,
		 g_gr->images().get("images/ui_basic/but0.png"),
		 g_gr->images().get("images/ui_basic/scrollbar_up.png"),
		 std::string(),
		 noise_tool.get_interval().min < MAX_FIELD_HEIGHT),
	upper_decrease_
		(this, "decr_upper",
		 get_inner_w() - 2 * width - hspacing(),
		 upper_label_.get_y(),
		 width, height,
		 g_gr->images().get("images/ui_basic/but0.png"),
		 g_gr->images().get("images/ui_basic/scrollbar_down.png"),
		 std::string(),
		 0 < noise_tool.get_interval().max),
	upper_increase_
		(this, "incr_upper",
		 get_inner_w() - width - hspacing(),
		 upper_label_.get_y(),
		 width, height,
		 g_gr->images().get("images/ui_basic/but0.png"),
		 g_gr->images().get("images/ui_basic/scrollbar_up.png"),
		 std::string(),
		 noise_tool.get_interval().max < MAX_FIELD_HEIGHT),
	set_label_
		(this,
		 hmargin(),
		 upper_label_.get_y() + upper_label_.get_h() + 2 * vspacing(),
		 width, height,
		 UI::Align::kLeft),
	setto_decrease_
		(this, "decr_set_to",
		 get_inner_w() - 2 * width - hspacing(),
		 set_label_.get_y(),
		 width, height,
		 g_gr->images().get("images/ui_basic/but1.png"),
		 g_gr->images().get("images/ui_basic/scrollbar_down.png"),
		 std::string(),
		 0 < noise_tool.set_tool().get_interval().min),
	setto_increase_
		(this, "incr_set_to",
		 get_inner_w() - width - hspacing(),
		 set_label_.get_y(),
		 width, height,
		 g_gr->images().get("images/ui_basic/but1.png"),
		 g_gr->images().get("images/ui_basic/scrollbar_up.png"),
		 std::string(),
		 noise_tool.set_tool().get_interval().max < MAX_FIELD_HEIGHT)
{
	lower_increase_.sigclicked.connect
		(boost::bind(&EditorToolNoiseHeightOptionsMenu::clicked_lower_increase, boost::ref(*this)));
	lower_decrease_.sigclicked.connect
		(boost::bind(&EditorToolNoiseHeightOptionsMenu::clicked_lower_decrease, boost::ref(*this)));
	upper_increase_.sigclicked.connect
		(boost::bind(&EditorToolNoiseHeightOptionsMenu::clicked_upper_increase, boost::ref(*this)));
	upper_decrease_.sigclicked.connect
		(boost::bind(&EditorToolNoiseHeightOptionsMenu::clicked_upper_decrease, boost::ref(*this)));
	setto_increase_.sigclicked.connect
		(boost::bind(&EditorToolNoiseHeightOptionsMenu::clicked_setto_increase, boost::ref(*this)));
	setto_decrease_.sigclicked.connect
		(boost::bind(&EditorToolNoiseHeightOptionsMenu::clicked_setto_decrease, boost::ref(*this)));

	lower_increase_.set_repeating(true);
	lower_decrease_.set_repeating(true);
	upper_increase_.set_repeating(true);
	upper_decrease_.set_repeating(true);
	setto_increase_.set_repeating(true);
	setto_decrease_.set_repeating(true);
	update();
}

/**
 * Update all textareas
*/
void EditorToolNoiseHeightOptionsMenu::update() {
	const Widelands::HeightInterval height_interval = noise_tool_.get_interval();

	lower_label_.set_text((boost::format(_("Minimum: %u"))
									% static_cast<unsigned int>(height_interval.min)).str());

	upper_label_.set_text((boost::format(_("Maximum: %u"))
									% static_cast<unsigned int>(height_interval.max)).str());

	set_label_.set_text((boost::format(_("Set value: %u"))
								 % static_cast<unsigned int>(noise_tool_.set_tool().get_interval().min)).str());

	select_correct_tool();
}


void EditorToolNoiseHeightOptionsMenu::clicked_lower_decrease() {
	Widelands::HeightInterval height_interval = noise_tool_.get_interval();

	assert(height_interval.valid());
	assert(0 < height_interval.min);

	--height_interval.min;

	assert(height_interval.valid());

	noise_tool_.set_interval(height_interval);
	lower_decrease_.set_enabled(0 < height_interval.min);
	lower_increase_.set_enabled(true);
	update();
}


void EditorToolNoiseHeightOptionsMenu::clicked_lower_increase() {
	Widelands::HeightInterval height_interval = noise_tool_.get_interval();

	assert(height_interval.valid());
	assert(height_interval.min < MAX_FIELD_HEIGHT);

	++height_interval.min;
	height_interval.max = std::max(height_interval.min, height_interval.max);

	assert(height_interval.valid());

	noise_tool_.set_interval(height_interval);
	lower_decrease_.set_enabled(true);
	lower_increase_.set_enabled(height_interval.min < MAX_FIELD_HEIGHT);
	upper_decrease_.set_enabled(true);
	upper_increase_.set_enabled(height_interval.max < MAX_FIELD_HEIGHT);

	update();
}


void EditorToolNoiseHeightOptionsMenu::clicked_upper_decrease() {
	Widelands::HeightInterval height_interval = noise_tool_.get_interval();

	assert(height_interval.valid());
	assert(0 < noise_tool_.get_interval().max);

	--height_interval.max;
	height_interval.min = std::min(height_interval.min, height_interval.max);

	assert(height_interval.valid());

	noise_tool_.set_interval(height_interval);
	lower_decrease_.set_enabled(0 < height_interval.min);
	lower_increase_.set_enabled(true);
	upper_decrease_.set_enabled(0 < height_interval.max);
	upper_increase_.set_enabled(true);
	update();
}


void EditorToolNoiseHeightOptionsMenu::clicked_upper_increase() {
	Widelands::HeightInterval height_interval = noise_tool_.get_interval();

	assert(noise_tool_.get_interval().valid());
	assert(noise_tool_.get_interval().max < MAX_FIELD_HEIGHT);

	++height_interval.max;

	assert(noise_tool_.get_interval().valid());

	noise_tool_.set_interval(height_interval);
	upper_decrease_.set_enabled(true);
	upper_increase_.set_enabled(height_interval.max < MAX_FIELD_HEIGHT);
	update();
}


void EditorToolNoiseHeightOptionsMenu::clicked_setto_decrease() {
	EditorSetHeightTool & set_tool = noise_tool_.set_tool();
	Field::Height h = set_tool.get_interval().min;

	assert(h == set_tool.get_interval().max);
	assert(0 < h);

	--h;

	set_tool.set_interval(Widelands::HeightInterval(h, h));
	setto_decrease_.set_enabled(0 < h);
	setto_increase_.set_enabled(true);
	update();
}


void EditorToolNoiseHeightOptionsMenu::clicked_setto_increase() {
	EditorSetHeightTool & set_tool = noise_tool_.set_tool();
	Field::Height h = set_tool.get_interval().min;

	assert(h == set_tool.get_interval().max);
	assert(h < MAX_FIELD_HEIGHT);

	++h;

	set_tool.set_interval(Widelands::HeightInterval(h, h));
	setto_decrease_.set_enabled(true);
	setto_increase_.set_enabled(h < MAX_FIELD_HEIGHT);
	update();
}
