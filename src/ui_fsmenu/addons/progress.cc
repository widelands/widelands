/*
 * Copyright (C) 2021-2023 by the Widelands Development Team
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

#include "ui_fsmenu/addons/progress.h"

#include "ui_fsmenu/addons/manager.h"

namespace AddOnsUI {

ProgressIndicatorWindow::ProgressIndicatorWindow(UI::Panel* parent,
                                                 UI::WindowStyle style,
                                                 const std::string& title)
   : UI::Window(parent,
                style,
                "progress",
                0,
                0,
                parent->get_inner_w() - 2 * kRowButtonSize,
                2 * kRowButtonSize,
                title),
     modal_(*this),
     box_(this, panel_style_, "main_box", 0, 0, UI::Box::Vertical, get_inner_w()),
     hbox_(&box_, panel_style_, "hbox", 0, 0, UI::Box::Horizontal),
     txt1_(&box_,
           panel_style_,
           "text_1",
           style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelParagraph :
                                               UI::FontStyle::kWuiInfoPanelParagraph,
           "",
           UI::Align::kCenter),
     txt2_(&hbox_,
           panel_style_,
           "text_2",
           style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelParagraph :
                                               UI::FontStyle::kWuiInfoPanelParagraph,
           "",
           UI::Align::kLeft),
     txt3_(&hbox_,
           panel_style_,
           "text_3",
           style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelParagraph :
                                               UI::FontStyle::kWuiInfoPanelParagraph,
           "",
           UI::Align::kRight),
     progress_(&box_,
               panel_style_,
               "progress",
               0,
               0,
               get_w(),
               kRowButtonSize,
               UI::ProgressBar::Horizontal) {

	hbox_.add(&txt2_, UI::Box::Resizing::kExpandBoth);
	hbox_.add(&txt3_, UI::Box::Resizing::kExpandBoth);
	box_.add(&txt1_, UI::Box::Resizing::kFullSize);
	box_.add_space(kRowButtonSpacing);
	box_.add(&hbox_, UI::Box::Resizing::kFullSize);
	box_.add_space(2 * kRowButtonSpacing);
	box_.add(&progress_, UI::Box::Resizing::kFullSize);

	set_center_panel(&box_);
	center_to_parent();

	initialization_complete();
}

}  // namespace AddOnsUI
