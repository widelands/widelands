/*
 * Copyright (C) 2016-2020 by the Widelands Development Team
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

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "graphic/image_io.h"
#include "graphic/minimap_renderer.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "map_io/map_loader.h"

GameDetails::GameDetails(Panel* parent, UI::PanelStyle style, Mode mode)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     style_(style),
     mode_(mode),
     padding_(4),
     name_label_(this,
                 0,
                 0,
                 0,
                 0,
                 style,
                 "",
                 UI::Align::kLeft,
                 UI::MultilineTextarea::ScrollMode::kNoScrolling),
     descr_(this,
            0,
            0,
            0,
            0,
            style,
            "",
            UI::Align::kLeft,
            UI::MultilineTextarea::ScrollMode::kNoScrolling),
     minimap_icon_(this, 0, 0, 0, 0, nullptr),
     button_box_(new UI::Box(this, 0, 0, UI::Box::Vertical)),
     last_game_(""),
     egbase_(nullptr) {

	add(&name_label_, UI::Box::Resizing::kFullSize);
	add_space(padding_);
	add(&descr_, UI::Box::Resizing::kExpandBoth);
	add_space(padding_);
	add(&minimap_icon_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	add_space(padding_);
	add(button_box_, UI::Box::Resizing::kFullSize);

	minimap_icon_.set_visible(false);
	minimap_icon_.set_frame(g_style_manager->minimap_icon_frame());

	// Fast initialize world now
	egbase_.mutable_world(true);
}

void GameDetails::clear() {
	name_label_.set_text("");
	descr_.set_text("");
	minimap_icon_.set_icon(nullptr);
	minimap_icon_.set_visible(false);
	minimap_icon_.set_size(0, 0);
	minimap_image_.reset();
}

void GameDetails::display(const std::vector<SavegameData>& gamedata) {
	if (gamedata.empty()) {
		return;
	} else if (gamedata.size() > 1) {
		show(gamedata);
	} else {
		show(gamedata[0]);
	}
}

void GameDetails::show(const std::vector<SavegameData>& gamedata) {

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
	boost::replace_all(name_list, "\n", "<br> • ");

	const std::string header_second_part(
	   /** TRANSLATORS: This is the second part of "Selected %1% directory/directories and %2%" */
	   (boost::format(ngettext("%d file", "%d files", number_of_files)) % number_of_files).str());

	std::string combined_header = as_richtext(as_heading_with_content(
	   /** TRANSLATORS: %1% = number of selected directories, %2% = number of selected files*/
	   (boost::format(ngettext("Selected %1% directory and %2%:",
	                           "Selected %1% directories and %2%:", number_of_directories)) %
	    number_of_directories % header_second_part)
	      .str(),
	   "", style_, true));

	name_label_.set_text(combined_header);

	std::string combined_description =
	   as_richtext(as_heading_with_content("", name_list, style_, true, true));

	descr_.set_text(combined_description);
	minimap_icon_.set_visible(false);
}

void GameDetails::show(const SavegameData& gamedata) {
	if (last_game_ == gamedata.filename) {
		return;
	}

	clear();
	last_game_ = gamedata.filename;
	if (gamedata.is_directory()) {
		name_label_.set_text(as_richtext(
		   as_heading_with_content(_("Directory Name:"), gamedata.filename, style_, true)));

		layout();
		return;
	}

	if (!gamedata.errormessage.empty()) {
		name_label_.set_text(as_richtext(
		   as_heading_with_content(_("Error:"), gamedata.errormessage, style_, true, true)));
		layout();
		return;
	}

	name_label_.set_text(
	   as_richtext(as_heading_with_content(_("Map Name:"), gamedata.mapname, style_, true)));

	show_game_description(gamedata);

	show_minimap(gamedata);

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
	   gamedata.gametime, style_);

	description = (boost::format("%s%s") % description %
	               as_heading_with_content(_("Players:"), gamedata.nrplayers, style_))
	                 .str();

	description = (boost::format("%s%s") % description %
	               as_heading_with_content(_("Widelands Version:"), gamedata.version, style_))
	                 .str();

	description = (boost::format("%s%s") % description %
	               as_heading_with_content(_("Win Condition:"), gamedata.wincondition, style_))
	                 .str();

	std::string filename = gamedata.filename;
	// Remove first directory from filename. This will be the save/ or replays/ folder
	assert(filename.find('/') != std::string::npos);
	filename.erase(0, filename.find('/') + 1);
	assert(!filename.empty());
	description = (boost::format("%s%s") % description %
	               as_heading_with_content(_("Filename:"), filename, style_))
	                 .str();

	descr_.set_text(as_richtext(description));
}

void GameDetails::show_minimap(const SavegameData& gamedata) {
	std::string minimap_path = gamedata.minimap_path;
	if (!minimap_path.empty()) {
		try {
			// Load the image
			minimap_image_ = load_image(
			   minimap_path,
			   std::unique_ptr<FileSystem>(g_fs->make_sub_file_system(gamedata.filename)).get());
			minimap_icon_.set_visible(true);
			minimap_icon_.set_icon(minimap_image_.get());
		} catch (const std::exception& e) {
			log_err("Failed to load the minimap image : %s\n", e.what());
		}
	} else if (mode_ == Mode::kReplay) {
		// Render minimap
		egbase_.cleanup_for_load();
		std::string filename(gamedata.filename);
		filename.append(kSavegameExtension);
		std::unique_ptr<Widelands::MapLoader> ml(egbase_.mutable_map()->get_correct_loader(filename));
		if (ml.get() && 0 == ml->load_map_for_render(egbase_)) {
			minimap_image_ = draw_minimap(egbase_, nullptr, Rectf(), MiniMapType::kStaticMap,
			                              MiniMapLayer::Terrain | MiniMapLayer::StartingPositions);
			minimap_icon_.set_icon(minimap_image_.get());
			minimap_icon_.set_visible(true);
		}
	}
}

void GameDetails::layout() {
	if (get_w() == 0 && get_h() == 0) {
		return;
	}
	UI::Box::layout();
	if (minimap_icon_.icon() == nullptr) {
		descr_.set_scrollmode(UI::MultilineTextarea::ScrollMode::kScrollNormal);
		minimap_icon_.set_desired_size(0, 0);
	} else {
		descr_.set_scrollmode(UI::MultilineTextarea::ScrollMode::kNoScrolling);

		// Downscale the minimap image
		const float available_width = get_w() - 4 * padding_;
		const float available_height =
		   get_h() - name_label_.get_h() - descr_.get_h() - button_box_->get_h() - 4 * padding_;

		const float scale =
		   std::min(1.f, std::min<float>(available_width / minimap_image_->width(),
		                                 available_height / minimap_image_->height()));

		const int w = scale * minimap_image_->width();
		const int h = scale * minimap_image_->height();

		// Center the minimap in the available space
		const int xpos = (get_w() - w) / 2;
		int ypos = name_label_.get_h() + descr_.get_h() + 2 * padding_;

		// Set small minimaps higher up for a more harmonious look
		if (h < available_height * 2 / 3) {
			ypos += (available_height - h) / 3;
		} else {
			ypos += (available_height - h) / 2;
		}

		minimap_icon_.set_desired_size(w, h);
		minimap_icon_.set_pos(Vector2i(xpos, ypos));
	}
}
