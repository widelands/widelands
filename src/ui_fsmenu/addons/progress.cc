/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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

#include "ui_fsmenu/addons/progress.h"

#include "ui_fsmenu/addons/manager.h"

namespace FsMenu {
namespace AddOnsUI {

ProgressIndicatorWindow::ProgressIndicatorWindow(UI::Panel* parent, const std::string& title)
   : UI::Window(parent,
                UI::WindowStyle::kFsMenu,
                "progress",
                0,
                0,
                parent->get_inner_w() - 2 * kRowButtonSize,
                2 * kRowButtonSize,
                title),
     modal_(*this),
     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, get_inner_w()),
     hbox_(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     txt1_(&box_,
           UI::PanelStyle::kFsMenu,
           UI::FontStyle::kFsMenuInfoPanelHeading,
           "",
           UI::Align::kCenter),
     txt2_(&hbox_,
           UI::PanelStyle::kFsMenu,
           UI::FontStyle::kFsMenuInfoPanelParagraph,
           "",
           UI::Align::kLeft),
     txt3_(&hbox_,
           UI::PanelStyle::kFsMenu,
           UI::FontStyle::kFsMenuInfoPanelParagraph,
           "",
           UI::Align::kRight),
     progress_(&box_,
               UI::PanelStyle::kFsMenu,
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
}  // namespace FsMenu
