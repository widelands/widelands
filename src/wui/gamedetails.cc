/*
 * Copyright (C) 2016 by the Widelands Development Team
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
#include "graphic/text_constants.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"

// TODO(GunChleoc): Arabic: line height broken for descriptions for Arabic.
namespace {
// 'is_first' omits the vertical gap before the line.
// 'noescape' is needed for error message formatting and does not call richtext_escape.
std::string as_header_with_content(const std::string& header,
                                   const std::string& content,
                                   GameDetails::Style style,
                                   bool is_first = false,
                                   bool noescape = false) {
	switch (style) {
	case GameDetails::Style::kFsMenu:
		return (boost::format(
		           "<p><font size=%i bold=1 shadow=1>%s%s <font color=D1D1D1>%s</font></font></p>") %
		        UI_FONT_SIZE_SMALL % (is_first ? "" : "<vspace gap=9>") %
		        (noescape ? header : richtext_escape(header)) %
		        (noescape ? content : richtext_escape(content)))
		   .str();
	case GameDetails::Style::kWui:
		return (boost::format(
		           "<p><font size=%i>%s<font bold=1 color=D1D1D1>%s</font> %s</font></p>") %
		        UI_FONT_SIZE_SMALL % (is_first ? "" : "<vspace gap=6>") %
		        (noescape ? header : richtext_escape(header)) %
		        (noescape ? content : richtext_escape(content)))
		   .str();
	default:
		NEVER_HERE();
	}
}

}  // namespace

SavegameData::SavegameData()
   : gametime(""),
     nrplayers("0"),
     savetimestamp(0),
     gametype(GameController::GameType::SINGLEPLAYER) {
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

GameDetails::GameDetails(Panel* parent, Style style, Mode mode)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     style_(style),
     mode_(mode),
     padding_(4),
     name_label_(
        this,
        0,
        0,
        0,
        0,
        "",
        UI::Align::kLeft,
        g_gr->images().get(style == GameDetails::Style::kFsMenu ? "images/ui_basic/but3.png" :
                                                                  "images/ui_basic/but1.png"),
        UI::MultilineTextarea::ScrollMode::kNoScrolling),
     descr_(this,
            0,
            0,
            0,
            0,
            "",
            UI::Align::kLeft,
            g_gr->images().get(style == GameDetails::Style::kFsMenu ? "images/ui_basic/but3.png" :
                                                                      "images/ui_basic/but1.png"),
            UI::MultilineTextarea::ScrollMode::kNoScrolling),
     minimap_icon_(this, 0, 0, 0, 0, nullptr),
     button_box_(new UI::Box(this, 0, 0, UI::Box::Vertical)) {
	name_label_.force_new_renderer();
	descr_.force_new_renderer();

	add(&name_label_, UI::Box::Resizing::kFullSize);
	add_space(padding_);
	add(&descr_, UI::Box::Resizing::kExpandBoth);
	add_space(padding_);
	add(&minimap_icon_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	add_space(padding_);
	add(button_box_, UI::Box::Resizing::kFullSize);

	minimap_icon_.set_visible(false);
	minimap_icon_.set_frame(UI_FONT_CLR_FG);
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

	if (gamedata.errormessage.empty()) {
		if (gamedata.filename_list.empty()) {
			name_label_.set_text(
			   (boost::format("<rt>%s</rt>") %
			    as_header_with_content(_("Map Name:"), gamedata.mapname, style_, true))
			      .str());

			// Show game information
			std::string description =
			   as_header_with_content(
			      mode_ == Mode::kReplay ?
			         /** TRANSLATORS: The time a replay starts. Shown in the replay loading screen*/
			         _("Start of Replay:") :
			         /** TRANSLATORS: The current time of a savegame. Shown in the game saving and
			            loading screens. */
			         _("Game Time:"),
			      gamedata.gametime, style_);

			description = (boost::format("%s%s") % description %
			               as_header_with_content(_("Players:"), gamedata.nrplayers, style_))
			                 .str();

			description = (boost::format("%s%s") % description %
			               as_header_with_content(_("Widelands Version:"), gamedata.version, style_))
			                 .str();

			description = (boost::format("%s%s") % description %
			               as_header_with_content(_("Win Condition:"), gamedata.wincondition, style_))
			                 .str();

			description = (boost::format("<rt>%s</rt>") % description).str();
			descr_.set_text(description);

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
		} else {
			std::string filename_list = richtext_escape(gamedata.filename_list);
			boost::replace_all(filename_list, "\n", "<br> • ");
			name_label_.set_text((boost::format("<rt>%s</rt>") %
			                      as_header_with_content(gamedata.mapname, "", style_, true))
			                        .str());

			descr_.set_text((boost::format("<rt>%s</rt>") %
			                 as_header_with_content("", filename_list, style_, true, true))
			                   .str());
			minimap_icon_.set_visible(false);
		}
	} else {
		name_label_.set_text(
		   (boost::format("<rt>%s</rt>") %
		    as_header_with_content(_("Error:"), gamedata.errormessage, style_, true, true))
		      .str());
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

		// Scale the minimap image.
		const float available_width = get_w() - 4 * padding_;
		const float available_height =
		   get_h() - name_label_.get_h() - descr_.get_h() - button_box_->get_h() - 4 * padding_;

		// Scale it
		float scale = available_width / minimap_image_->width();
		const float scale_y = available_height / minimap_image_->height();
		if (scale_y < scale) {
			scale = scale_y;
		}
		// Don't make the image too big; fuzziness will result
		scale = std::min(1.f, scale);

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
