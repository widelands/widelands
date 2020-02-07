/*
 * Copyright (C) 2016-2019 by the Widelands Development Team
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
#include <boost/lexical_cast.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/time_string.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/text_layout.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"

SavegameData::SavegameData()
   : gametime(""),
     nrplayers("0"),
     savetimestamp(0),
     gametype(GameController::GameType::kSingleplayer) {
}

SavegameData::SavegameData(const std::string& fname)
   : SavegameData(fname, SavegameType::kSavegame) {
}
SavegameData::SavegameData(const std::string& fname, const SavegameType& type)
   : filename(fname),
     gametime(""),
     nrplayers("0"),
     savetimestamp(0),
     gametype(GameController::GameType::kSingleplayer),
     type_(type) {
}

void SavegameData::set_gametime(uint32_t input_gametime) {
	gametime = gametimestring(input_gametime);
}
void SavegameData::set_nrplayers(Widelands::PlayerNumber input_nrplayers) {
	nrplayers = boost::lexical_cast<std::string>(static_cast<unsigned int>(input_nrplayers));
}
void SavegameData::set_mapname(const std::string& input_mapname) {
	i18n::Textdomain td("maps");
	mapname = _(input_mapname);
}

bool SavegameData::is_directory() const {
	return is_sub_directory() || is_parent_directory();
}

bool SavegameData::is_parent_directory() const {
	return type_ == SavegameType::kParentDirectory;
}

bool SavegameData::is_sub_directory() const {
	return type_ == SavegameType::kSubDirectory;
}

bool SavegameData::compare_save_time(const SavegameData& other) const {
	if (is_directory() || other.is_directory()) {
		return compare_directories(other);
	}
	return savetimestamp < other.savetimestamp;
}

bool SavegameData::compare_map_name(const SavegameData& other) const {
	if (is_directory() || other.is_directory()) {
		return compare_directories(other);
	}
	return mapname < other.mapname;
}

bool SavegameData::compare_directories(const SavegameData& other) const {
	// parent directory always on top
	if (is_parent_directory()) {
		return false;
	}
	if (other.is_parent_directory()) {
		return true;
	}
	// sub directory before non-sub directory (aka actual savegame)
	if (is_sub_directory() && !other.is_directory()) {
		return false;
	}
	if (!is_sub_directory() && other.is_sub_directory()) {
		return true;
	}
	// sub directories sort after name
	if (is_sub_directory() && other.is_sub_directory()) {
		return filename > other.filename;
	}

	return false;
}

// static
SavegameData SavegameData::create_parent_dir(const std::string& current_dir) {
	std::string filename = FileSystem::fs_dirname(current_dir);
	if (!filename.empty()) {
		// fs_dirname always returns a directory with a separator at the end.
		filename.pop_back();
	}
	return SavegameData(filename, SavegameData::SavegameType::kParentDirectory);
}

SavegameData SavegameData::create_sub_dir(const std::string& directory) {
	return SavegameData(directory, SavegameData::SavegameType::kSubDirectory);
}

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
     button_box_(new UI::Box(this, 0, 0, UI::Box::Vertical)) {

	add(&name_label_, UI::Box::Resizing::kFullSize);
	add_space(padding_);
	add(&descr_, UI::Box::Resizing::kExpandBoth);
	add_space(padding_);
	add(&minimap_icon_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	add_space(padding_);
	add(button_box_, UI::Box::Resizing::kFullSize);

	minimap_icon_.set_visible(false);
	minimap_icon_.set_frame(g_gr->styles().minimap_icon_frame());
}

void GameDetails::clear() {
	name_label_.set_text("");
	descr_.set_text("");
	minimap_icon_.set_icon(nullptr);
	minimap_icon_.set_visible(false);
	minimap_icon_.set_size(0, 0);
	minimap_image_.reset();
}

void GameDetails::update(const SavegameData& gamedata) {
	clear();
	// Do not display anything if gamedata is empty
	if (gamedata.errormessage.empty() && gamedata.filename_list.empty() &&
	    gamedata.mapname.empty()) {
		return;
	}

	if (gamedata.errormessage.empty()) {
		if (gamedata.filename_list.empty() && !gamedata.filename.empty()) {
			name_label_.set_text(
			   as_richtext(as_heading_with_content(_("Map Name:"), gamedata.mapname, style_, true)));

			// Show game information
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
					log("Failed to load the minimap image : %s\n", e.what());
				}
			}
		} else if (!gamedata.filename_list.empty()) {
			std::string filename_list = richtext_escape(gamedata.filename_list);
			boost::replace_all(filename_list, "\n", "<br> • ");
			name_label_.set_text(
			   as_richtext(as_heading_with_content(gamedata.mapname, "", style_, true)));

			descr_.set_text(
			   as_richtext(as_heading_with_content("", filename_list, style_, true, true)));
			minimap_icon_.set_visible(false);
		}
	} else {
		name_label_.set_text(as_richtext(
		   as_heading_with_content(_("Error:"), gamedata.errormessage, style_, true, true)));
	}
	layout();
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
