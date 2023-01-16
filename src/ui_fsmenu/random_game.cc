/*
 * Copyright (C) 2020-2023 by the Widelands Development Team
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

#include "ui_fsmenu/random_game.h"

#include "editor/editorinteractive.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/launch_spg.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {

RandomGame::RandomGame(MenuCapsule& m)
   : TwoColumnsFullNavigationMenu(m, _("New Random Game")),
     menu_(left_column_box_, UI::PanelStyle::kFsMenu, 350, 64, 64, ok_, back_),
     icon_(&right_column_content_box_,
           UI::PanelStyle::kFsMenu,
           g_image_cache->get("images/logos/wl-ico-128.png")) {
	m.set_visible(false);

	{  // Do this first to prevent crashes with incompatible add-on types
		std::vector<AddOns::AddOnState> new_g_addons;
		for (const AddOns::AddOnState& s : AddOns::g_addons) {
			if (s.second && s.first->category != AddOns::AddOnCategory::kTribes &&
			    s.first->category != AddOns::AddOnCategory::kScript) {
				new_g_addons.push_back(s);
			}
		}
		AddOns::g_addons = new_g_addons;
	}

	settings_.reset(new SinglePlayerGameSettingsProvider());
	game_.reset(m.menu().create_safe_game());
	if (game_ == nullptr) {
		die();
		return;
	}
	game_->logic_rand_seed(RNG::static_rand());
	m.set_visible(true);

	left_column_box_.add_inf_space();
	left_column_box_.add(&menu_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	left_column_box_.add_inf_space();

	// There's so much unused space in this screen, let's beautify it a bit
	right_column_content_box_.add_inf_space();
	right_column_content_box_.add_inf_space();
	right_column_content_box_.add(&icon_, UI::Box::Resizing::kExpandBoth);
	right_column_content_box_.add_inf_space();
	layout();
	{
		UI::MultilineTextarea* txt = new UI::MultilineTextarea(
		   &right_column_content_box_, 0, 0, 100, 100, UI::PanelStyle::kFsMenu, "", UI::Align::kLeft,
		   UI::MultilineTextarea::ScrollMode::kNoScrolling);
		txt->set_style(UI::FontStyle::kFsMenuInfoPanelParagraph);
		txt->set_text(
		   _("The random map generator is still in beta stage. The maps it generates are usually – "
		     "but not always – well playable. Seafaring and artifacts are not supported. Add-ons of "
		     "the Tribes and Script categories are disabled in random matches."));
		right_column_content_box_.add(txt, UI::Box::Resizing::kFullSize);
		right_column_content_box_.add_inf_space();
	}

	reactivated();
	assert(progress_window_);
	progress_window_->set_visible(true);
	EditorInteractive::load_world_units(nullptr, *game_);
	progress_window_->set_visible(false);

	layout();
	initialization_complete();
}

RandomGame::~RandomGame() {
	if (game_ != nullptr) {
		game_->cleanup_objects();
	}
	if (progress_window_ != nullptr) {
		game_->release_loader_ui();
		progress_window_ = nullptr;
	}
}

void RandomGame::reactivated() {
	if (progress_window_ == nullptr) {
		progress_window_ =
		   &game_->create_loader_ui({"general_game", "singleplayer"}, false, "", "", true, &capsule_);
		progress_window_->set_visible(false);
	}
}

void RandomGame::layout() {
	TwoColumnsFullNavigationMenu::layout();
	if (progress_window_ != nullptr) {
		progress_window_->set_size(capsule_.get_inner_w(), capsule_.get_inner_h());
	}
}

void RandomGame::clicked_ok() {
	assert(progress_window_);
	progress_window_->set_visible(true);
	game_->cleanup_objects();

	if (menu_.do_generate_map(*game_, nullptr, settings_.get())) {
		game_->remove_loader_ui();
		progress_window_ = nullptr;
		new LaunchSPG(capsule_, *settings_, game_, nullptr, false);
	} else {
		progress_window_->set_visible(false);
		MainMenu& m = capsule_.menu();
		m.show_messagebox(_("Map Generation Error"),
		                  _("The random map generator was unable to generate a suitable map. "
		                    "This happens occasionally because the generator is still in beta stage. "
		                    "Please try again with slightly different settings."));
	}
}

}  // namespace FsMenu
