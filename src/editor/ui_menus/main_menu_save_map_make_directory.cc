/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "editor/ui_menus/main_menu_save_map_make_directory.h"

#include "base/i18n.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"

MainMenuSaveMapMakeDirectory::MainMenuSaveMapMakeDirectory(UI::Panel* const parent,
                                                           char const* dirname)
   : UI::Window(parent, "make_directory", 0, 0, 330, 100, _("Make Directory")),
     padding_(5),
     butw_(get_inner_w() / 2 - 3 * padding_),
     buth_(20),
     dirname_(dirname),
     vbox_(this,
           padding_,
           padding_,
           UI::Box::Vertical,
           get_inner_w() - 2 * padding_,
           get_inner_h() - 3 * padding_ - buth_,
           padding_ / 2),
     label_(&vbox_,
            0, 0,
            get_inner_w() - 2 * padding_,
            buth_,
            _("Enter Directory Name:"),
            UI::Align::kTopLeft),
     edit_(&vbox_,
           0, 0,
           get_inner_w() - 2 * padding_,
           0, 4,
           g_gr->images().get("images/ui_basic/but1.png")),
     ok_button_(this,
                "ok",
                UI::g_fh1->fontset()->is_rtl() ? padding_ : get_inner_w() - butw_ - padding_,
                get_inner_h() - padding_ - buth_,
                butw_,
                buth_,
                g_gr->images().get("images/ui_basic/but5.png"),
                _("OK")),
     cancel_button_(this,
                    "cancel",
                    UI::g_fh1->fontset()->is_rtl() ? get_inner_w() - butw_ - padding_ : padding_,
                    get_inner_h() - padding_ - buth_,
                    butw_,
                    buth_,
                    g_gr->images().get("images/ui_basic/but1.png"),
                    _("Cancel")) {

	vbox_.add(&label_, UI::HAlign::kLeft);
	vbox_.add_space(padding_);
	vbox_.add(&edit_, UI::HAlign::kLeft);
	vbox_.set_size(get_inner_w() - 2 * padding_, get_inner_h() - 3 * padding_ - buth_);

	edit_.set_text(dirname_);
	edit_.changed.connect(boost::bind(&MainMenuSaveMapMakeDirectory::edit_changed, this));
	ok_button_.sigclicked.connect(
	   boost::bind(&MainMenuSaveMapMakeDirectory::end_modal<UI::Panel::Returncodes>,
	               boost::ref(*this), UI::Panel::Returncodes::kOk));
	ok_button_.set_enabled(false);
	cancel_button_.sigclicked.connect(
	   boost::bind(&MainMenuSaveMapMakeDirectory::end_modal<UI::Panel::Returncodes>,
	               boost::ref(*this), UI::Panel::Returncodes::kBack));
	center_to_parent();
	edit_.focus();
}

/**
 * Editbox changed
 */
void MainMenuSaveMapMakeDirectory::edit_changed() {
	const std::string& text = edit_.text();
	// Prevent the user from creating nonsense directory names, like e.g. ".." or "...".
	ok_button_.set_enabled(LayeredFileSystem::is_legal_filename(text));
	dirname_ = text;
}
