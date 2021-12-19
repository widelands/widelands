/*
 * Copyright (C) 2016-2021 by the Widelands Development Team
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

#include "wui/gamedetails.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/string.h"
#include "graphic/image_io.h"
#include "graphic/minimap_renderer.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/addons.h"
#include "logic/filesystem_constants.h"
#include "map_io/map_loader.h"

GameDetails::GameDetails(Panel* parent,
                         UI::PanelStyle style,
                         Mode mode,
                         Widelands::EditorGameBase& egbase)
   : UI::Panel(parent, style, 0, 0, 0, 0),
     mode_(mode),
     padding_(4),
     has_conflicts_(false),
     main_box_(this, style, 0, 0, UI::Box::Vertical, 0, 0, 0),
     descr_box_(&main_box_, style, 0, 0, UI::Box::Vertical, 0, 0, 0),
     name_label_(&main_box_,
                 0,
                 0,
                 0,
                 0,
                 style,
                 "",
                 UI::Align::kLeft,
                 UI::MultilineTextarea::ScrollMode::kNoScrolling),
     descr_(&descr_box_,
            0,
            0,
            UI::Scrollbar::kSize,
            0,
            style,
            "",
            UI::Align::kLeft,
            UI::MultilineTextarea::ScrollMode::kNoScrolling),
     minimap_icon_(&descr_box_, style, 0, 0, 0, 0, nullptr),
     button_box_(new UI::Box(&main_box_, style, 0, 0, UI::Box::Vertical)),
     egbase_(egbase) {
	descr_.set_handle_mouse(false);
	descr_box_.add(&descr_, UI::Box::Resizing::kFullSize);
	descr_box_.add_space(padding_);
	descr_box_.add(&minimap_icon_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	main_box_.add(&name_label_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(padding_);
	main_box_.add(&descr_box_, UI::Box::Resizing::kExpandBoth);
	main_box_.add_space(padding_);
	main_box_.add(button_box_, UI::Box::Resizing::kFullSize);

	minimap_icon_.set_visible(false);
	minimap_icon_.set_frame(g_style_manager->minimap_icon_frame());
}

void GameDetails::clear() {
	name_label_.set_text("");
	descr_.set_text("");
	minimap_icon_.set_icon(nullptr);
	minimap_icon_.set_visible(false);
	minimap_icon_.set_size(0, 0);
}

void GameDetails::display(const std::vector<SavegameData>& gamedata) {
	if (gamedata.empty()) {
		return;
	}
	if (gamedata.size() > 1) {
		show(gamedata);
	} else {
		show(gamedata[0]);
	}
}

void GameDetails::show(const std::vector<SavegameData>& gamedata) {
	clear();

	size_t number_of_files = 0;
	size_t number_of_directories = 0;
	for (const SavegameData& g : gamedata) {
		if (g.is_directory()) {
			number_of_directories++;
		} else {
			number_of_files++;
		}
	}
	std::string name_list = richtext_escape(as_filename_list(gamedata));
	replace_all(name_list, "\n", "<br> • ");

	const std::string header_second_part(
	   /** TRANSLATORS: This is the second part of "Selected %1% directory/directories and %2%" */
	   format(ngettext("%d file", "%d files", number_of_files), number_of_files));

	std::string combined_header = as_richtext(as_heading_with_content(
	   /** TRANSLATORS: %1% = number of selected directories, %2% = number of selected files*/
	   format(ngettext("Selected %1% directory and %2%:", "Selected %1% directories and %2%:",
	                   number_of_directories),
	          number_of_directories, header_second_part),
	   "", panel_style_, true));

	name_label_.set_text(combined_header);

	std::string combined_description =
	   as_richtext(as_heading_with_content("", name_list, panel_style_, true, true));

	descr_.set_text(combined_description);
}

void GameDetails::show(const SavegameData& gamedata) {
	clear();
	last_game_ = gamedata.filename;
	if (gamedata.is_directory()) {
		name_label_.set_text(as_richtext(
		   as_heading_with_content(_("Directory Name:"), gamedata.filename, panel_style_, true)));

		layout();
		return;
	}

	if (!gamedata.errormessage.empty()) {
		name_label_.set_text(as_richtext(
		   as_heading_with_content(_("Error:"), gamedata.errormessage, panel_style_, true, true)));
		layout();
		return;
	}

	name_label_.set_text(
	   as_richtext(as_heading_with_content(_("Map Name:"), gamedata.mapname, panel_style_, true)));

	show_game_description(gamedata);

	layout();
}

void GameDetails::show_game_description(const SavegameData& gamedata) {
	std::string description = as_heading_with_content(
	   mode_ == Mode::kReplay ?
            /** TRANSLATORS: The time a replay starts. Shown in the replay loading screen*/
            _("Start of Replay:") :
            /** TRANSLATORS: The current time of a savegame. Shown in the game saving and
               loading screens. */
            _("Game Time:"),
	   gamedata.gametime, panel_style_);

	description = format("%s%s", description,
	                     as_heading_with_content(_("Players:"), gamedata.nrplayers, panel_style_));

	description =
	   format("%s%s", description,
	          as_heading_with_content(_("Widelands Version:"), gamedata.version, panel_style_));

	description =
	   format("%s%s", description,
	          as_heading_with_content(_("Win Condition:"), gamedata.wincondition, panel_style_));

	AddOns::AddOnConflict addons = AddOns::check_requirements(gamedata.required_addons);
	has_conflicts_ = addons.second;

	description =
	   format("%s%s", description,
	          as_heading_with_content(_("Add-Ons:"), addons.first, panel_style_, false, true));

	std::string filename = gamedata.filename;
	// Remove first directory from filename. This will be the save/ or replays/ folder
	assert(filename.find('/') != std::string::npos);
	filename.erase(0, filename.find('/') + 1);
	assert(!filename.empty());
	description =
	   format("%s%s", description, as_heading_with_content(_("Filename:"), filename, panel_style_));

	const std::string err = show_minimap(gamedata);
	if (!err.empty()) {
		// Critical error, put this on top
		description = format(
		   "%s%s", as_heading_with_content(_("Game data error:"), err, panel_style_), description);
	}

	descr_.set_text(as_richtext(description));
}

std::string GameDetails::show_minimap(const SavegameData& gamedata) {
	std::string minimap_path = gamedata.minimap_path;
	if (!minimap_path.empty()) {
		try {
			// Load the image
			minimap_cache_[last_game_] = load_image(
			   minimap_path,
			   std::unique_ptr<FileSystem>(g_fs->make_sub_file_system(gamedata.filename)).get());
			minimap_icon_.set_visible(true);
			minimap_icon_.set_icon(minimap_cache_.at(last_game_).get());
		} catch (const std::exception& e) {
			log_err("Failed to load the minimap image : %s\n", e.what());
		}
	} else if (mode_ == Mode::kReplay) {
		// Render minimap
		auto minimap = minimap_cache_.find(last_game_);
		if (minimap != minimap_cache_.end()) {
			minimap_icon_.set_icon(minimap->second.get());
			minimap_icon_.set_visible(true);
		} else {
			try {
				egbase_.cleanup_for_load();
				std::string filename(last_game_);
				filename.append(kSavegameExtension);
				std::unique_ptr<Widelands::MapLoader> ml(
				   egbase_.mutable_map()->get_correct_loader(filename));
				if (ml.get() && 0 == ml->load_map_for_render(egbase_, &egbase_.enabled_addons())) {
					minimap_cache_[last_game_] =
					   draw_minimap(egbase_, nullptr, Rectf(), MiniMapType::kStaticMap,
					                MiniMapLayer::Terrain | MiniMapLayer::StartingPositions);
					minimap_icon_.set_icon(minimap_cache_.at(last_game_).get());
					minimap_icon_.set_visible(true);
				}
			} catch (const std::exception& e) {
				log_err("Failed to load the minimap image: %s", e.what());
				has_conflicts_ = true;
				return e.what();
			}
		}
	}
	return std::string();
}

void GameDetails::layout() {
	main_box_.set_size(get_w(), get_h());

	if (minimap_icon_.icon() == nullptr) {
		minimap_icon_.set_desired_size(0, 0);
	} else {
		// Fit minimap to width
		const int width = std::min<int>(main_box_.get_w() - UI::Scrollbar::kSize - 2 * padding_,
		                                minimap_cache_.at(last_game_)->width());
		const float scale = static_cast<float>(width) / minimap_cache_.at(last_game_)->width();
		const int height = scale * minimap_cache_.at(last_game_)->height();

		minimap_icon_.set_desired_size(width, height);
	}

	const int full_height = descr_.get_h() + minimap_icon_.get_h();
	const int descr_height =
	   main_box_.get_h() - name_label_.get_h() - button_box_->get_h() - 2 * padding_;
	descr_box_.set_force_scrolling(full_height > descr_height);
	descr_box_.set_size(main_box_.get_w(), descr_height);
}
