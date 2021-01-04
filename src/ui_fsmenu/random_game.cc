/*
 * Copyright (C) 2020 by the Widelands Development Team
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

#include "ui_fsmenu/random_game.h"

#include "editor/editorinteractive.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/launch_spg.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {

RandomGame::RandomGame(MenuCapsule& m)
   : TwoColumnsFullNavigationMenu(m, _("New Random Game")),
     menu_(left_column_box_, UI::PanelStyle::kFsMenu, 350, 64, 64),
     icon_(&right_column_content_box_,
           UI::PanelStyle::kFsMenu,
           g_image_cache->get("images/logos/wl-ico-128.png")),
     progress_window_(nullptr) {
	left_column_box_.add_inf_space();
	left_column_box_.add(&menu_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	left_column_box_.add_inf_space();

	// There's so much unused space in this screen, let's beautify it a bit
	right_column_content_box_.add_inf_space();
	right_column_content_box_.add(&icon_, UI::Box::Resizing::kExpandBoth);
	right_column_content_box_.add_inf_space();

	reactivated();
	assert(progress_window_);
	progress_window_->set_visible(true);
	EditorInteractive::load_world_units(nullptr, game_);
	progress_window_->set_visible(false);

	layout();
	initialization_complete();
}

RandomGame::~RandomGame() {
	game_.cleanup_objects();
}

void RandomGame::reactivated() {
	if (!progress_window_) {
		progress_window_ =
		   &game_.create_loader_ui({"general_game", "singleplayer"}, false, "", "", &capsule_);
		progress_window_->set_visible(false);
	}
}

void RandomGame::layout() {
	TwoColumnsFullNavigationMenu::layout();
	if (progress_window_) {
		progress_window_->set_size(capsule_.get_inner_w(), capsule_.get_inner_h());
	}
}

void RandomGame::clicked_ok() {
	assert(progress_window_);
	progress_window_->set_visible(true);
	game_.cleanup_objects();

	if (menu_.do_generate_map(game_, nullptr, &settings_)) {
		game_.remove_loader_ui();
		progress_window_ = nullptr;
		new LaunchSPG(capsule_, settings_, game_, nullptr, false);
	} else {
		progress_window_->set_visible(false);
		MainMenu& m = capsule_.menu();
		UI::WLMessageBox mbox(
		   &m, UI::WindowStyle::kFsMenu, _("Map Generation Error"),
		   _("The random map generator was unable to generate a suitable map. "
		     "This happens occasionally because the generator is still in beta stage. "
		     "Please try again with slightly different settings."),
		   UI::WLMessageBox::MBoxType::kOkCancel);
		if (mbox.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			die();
		}
	}
}

}  // namespace FsMenu
