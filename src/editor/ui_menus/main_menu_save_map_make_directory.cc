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

#include "editor/ui_menus/main_menu_save_map_make_directory.h"

#include "base/i18n.h"
#include "base/string.h"
#include "graphic/font_handler.h"
#include "io/filesystem/illegal_filename_check.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"

MainMenuSaveMapMakeDirectory::MainMenuSaveMapMakeDirectory(UI::Panel* const parent,
                                                           char const* dirname)
   : UI::Window(
        parent, UI::WindowStyle::kWui, "make_directory", 0, 0, 330, 100, _("Make Directory")),
     padding_(5),
     butw_(get_inner_w() / 2 - 3 * padding_),
     buth_(20),
     dirname_(dirname),
     vbox_(this,
           UI::PanelStyle::kWui,
           padding_,
           padding_,
           UI::Box::Vertical,
           get_inner_w() - 2 * padding_,
           get_inner_h() - 3 * padding_ - buth_,
           padding_ / 2),
     label_(&vbox_,
            UI::PanelStyle::kWui,
            UI::FontStyle::kWuiLabel,
            0,
            0,
            get_inner_w() - 2 * padding_,
            buth_,
            _("Enter Directory Name:")),
     edit_(&vbox_, 0, 0, get_inner_w() - 2 * padding_, UI::PanelStyle::kWui),
     ok_button_(this,
                "ok",
                UI::g_fh->fontset()->is_rtl() ? padding_ : get_inner_w() - butw_ - padding_,
                get_inner_h() - padding_ - buth_,
                butw_,
                buth_,
                UI::ButtonStyle::kWuiPrimary,
                _("OK")),
     cancel_button_(this,
                    "cancel",
                    UI::g_fh->fontset()->is_rtl() ? get_inner_w() - butw_ - padding_ : padding_,
                    get_inner_h() - padding_ - buth_,
                    butw_,
                    buth_,
                    UI::ButtonStyle::kWuiSecondary,
                    _("Cancel")),
     illegal_filename_tooltip_(FileSystemHelper::illegal_filename_tooltip()) {

	vbox_.add(&label_);
	vbox_.add_space(padding_);
	vbox_.add(&edit_);
	vbox_.set_size(get_inner_w() - 2 * padding_, get_inner_h() - 3 * padding_ - buth_);

	edit_.set_text(dirname_);
	edit_.changed.connect([this]() { edit_changed(); });
	edit_.ok.connect([this]() { clicked_ok(); });
	edit_.cancel.connect(
	   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack); });
	ok_button_.sigclicked.connect([this]() { clicked_ok(); });
	ok_button_.set_enabled(!dirname_.empty());
	cancel_button_.sigclicked.connect(
	   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack); });
	center_to_parent();

	initialization_complete();
}

void MainMenuSaveMapMakeDirectory::start() {
	edit_.focus();
}

/**
 * Editbox changed
 */
void MainMenuSaveMapMakeDirectory::edit_changed() {
	const std::string& text = edit_.text();
	// Prevent the user from creating nonsense directory names, like e.g. ".." or "...".
	const bool is_legal_filename = FileSystemHelper::is_legal_filename(text);
	// Prevent the user from creating directory names that the game would
	// try to interpret as maps
	const bool has_map_extension = ends_with(text, kWidelandsMapExtension, false) ||
	                               ends_with(text, kS2MapExtension1, false) ||
	                               ends_with(text, kS2MapExtension2, false);
	ok_button_.set_enabled(is_legal_filename && !has_map_extension);
	edit_.set_tooltip(is_legal_filename ?
                        (has_map_extension ? _("This extension is reserved!") : "") :
                        illegal_filename_tooltip_);
	dirname_ = text;
}

/**
 * Clicked OK button oder pressed Enter in edit box
 */
void MainMenuSaveMapMakeDirectory::clicked_ok() {
	if (!ok_button_.enabled()) {
		return;
	}
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
}
