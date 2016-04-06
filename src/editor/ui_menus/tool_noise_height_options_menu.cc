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
#include "ui_basic/textarea.h"


using Widelands::Field;

EditorToolNoiseHeightOptionsMenu::EditorToolNoiseHeightOptionsMenu
	(EditorInteractive         & parent,
	 EditorNoiseHeightTool   & noise_tool,
	 UI::UniqueWindow::Registry & registry)
	:
	EditorToolOptionsMenu(parent, registry, 300, 120, _("Noise Height Options")),
	noise_tool_(noise_tool),
	box_(this, hmargin(), vmargin(), UI::Box::Vertical, 0, 0, vspacing()),
	lower_(&box_, 0, 0, get_inner_w() - 2 * hmargin(), 80,
				  10, 1, MAX_FIELD_HEIGHT,
				  _("Minimum Height:"), UI::SpinBox::Units::kNone,
				  g_gr->images().get("images/ui_basic/but1.png"),
				  UI::SpinBox::Type::kSmall),
	upper_(&box_, 0, 0, get_inner_w() - 2 * hmargin(), 80,
				  14, 0, MAX_FIELD_HEIGHT,
				  _("Maximum Height:"), UI::SpinBox::Units::kNone,
				  g_gr->images().get("images/ui_basic/but1.png"),
				  UI::SpinBox::Type::kSmall),
	set_to_(&box_, 0, 0, get_inner_w() - 2 * hmargin(), 80,
			  10, 0, MAX_FIELD_HEIGHT,
			  _("Set Value:"), UI::SpinBox::Units::kNone,
			  g_gr->images().get("images/ui_basic/but1.png"),
			  UI::SpinBox::Type::kSmall)
{
	lower_.set_tooltip(
				/** TRANSLATORS: Editor noise height access keys. **/
				_("Click to set the height to a random value within the specified range"));
	upper_.set_tooltip(
				/** TRANSLATORS: Editor noise height access keys. **/
				_("Click to set the height to a random value within the specified range"));
	set_to_.set_tooltip(
				/** TRANSLATORS: Editor set hoise height access keys. **/
				(boost::format(_("Use %s or %s to set a fixed height"))
				 /** TRANSLATORS: This is an access key combination. Localize, but do not change the key. **/
				 % _("Shift + Click")
				 /** TRANSLATORS: This is an access key combination. Localize, but do not change the key. **/
				 % _("Ctrl + Click")).str());

	upper_.changed.connect
		(boost::bind
		 (&EditorToolNoiseHeightOptionsMenu::update_interval, boost::ref(*this)));
	lower_.changed.connect
		(boost::bind
		 (&EditorToolNoiseHeightOptionsMenu::update_interval, boost::ref(*this)));
	set_to_.changed.connect
		(boost::bind
		 (&EditorToolNoiseHeightOptionsMenu::update_set_to, boost::ref(*this)));

	UI::Textarea* label = new UI::Textarea(&box_, 0, 0, 0, 0, _("Random Height"), UI::Align::kCenter);
	label->set_fixed_width(get_inner_w() - 2 * hmargin());
	box_.add(label, UI::Align::kLeft);
	box_.add(&upper_, UI::Align::kLeft);
	box_.add(&lower_, UI::Align::kLeft);

	box_.add_space(2 * vspacing());
	label = new UI::Textarea(&box_, 0, 0, 0, 0, _("Fixed Height"), UI::Align::kCenter);
	label->set_fixed_width(get_inner_w() - 2 * hmargin());
	box_.add(label, UI::Align::kLeft);
	box_.add(&set_to_, UI::Align::kLeft);

	box_.set_size(get_inner_w() - 2 * hmargin(),
					  upper_.get_h() + lower_.get_h() + set_to_.get_h() + 2 * label->get_h() + 7 * vspacing());

	set_inner_size(box_.get_w() + 2 * hmargin(), box_.get_h() + 2 * vspacing());
}

void EditorToolNoiseHeightOptionsMenu::update_interval() {
	int32_t upper = upper_.get_value();
	int32_t lower = lower_.get_value();
	assert(lower >= 0);
	assert(lower <= MAX_FIELD_HEIGHT);
	assert(upper >= 0);
	assert(upper <= MAX_FIELD_HEIGHT);

	Widelands::HeightInterval height_interval(lower,upper);
	height_interval.min = std::min(height_interval.min, height_interval.max);
	height_interval.max = std::max(height_interval.min, height_interval.max);
	assert(height_interval.valid());

	noise_tool_.set_interval(height_interval);
	select_correct_tool();
}

void EditorToolNoiseHeightOptionsMenu::update_set_to() {
	int32_t set_to = set_to_.get_value();
	assert(set_to >= 0);
	assert(set_to <= MAX_FIELD_HEIGHT);
	noise_tool_.set_tool().set_interval(Widelands::HeightInterval(set_to, set_to));
	select_correct_tool();
}
