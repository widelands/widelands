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

#include <boost/format.hpp>

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
		           "<p><font size=%i bold=1 shadow=1>%s<font color=D1D1D1>%s</font> %s</font></p>") %
		        UI_FONT_SIZE_SMALL % (is_first ? "" : "<vspace gap=6>") %
		        (noescape ? header : richtext_escape(header)) %
		        (noescape ? content : richtext_escape(content)))
		   .str();
	default:
		NEVER_HERE();
	}
}

}  // namespace

GameDetails::GameDetails(
   Panel* parent, int32_t x, int32_t y, int32_t max_w, int32_t max_h, Style style)
   : UI::Panel(parent, x, y, max_w, max_h),
     style_(style),
     padding_(4),
     main_box_(this, 0, 0, UI::Box::Vertical, max_w, max_h, 0),
     name_label_(&main_box_,
                 0,
                 0,
                 max_w - padding_,
                 20,
                 "",
                 UI::Align::kLeft,
                 UI::MultilineTextarea::ScrollMode::kNoScrolling),
     descr_(&main_box_,
            0,
            0,
            max_w,
            80,
            "",
            UI::Align::kLeft,
            UI::MultilineTextarea::ScrollMode::kNoScrolling),
     minimap_icon_(&main_box_, 0, 0, max_w - 2 * padding_, max_h - 2 * padding_, nullptr) {
	name_label_.force_new_renderer();
	descr_.force_new_renderer();

	main_box_.add(&name_label_, UI::Align::kLeft);
	main_box_.add_space(padding_);
	main_box_.add(&descr_, UI::Align::kLeft);
	main_box_.add_space(padding_);
	main_box_.add(&minimap_icon_, UI::Align::kLeft);
	main_box_.set_size(max_w, max_h);

	minimap_icon_.set_visible(false);
}

void GameDetails::clear() {
	name_label_.set_text("");
	descr_.set_text("");
	minimap_icon_.set_icon(nullptr);
	minimap_icon_.set_visible(false);
	minimap_icon_.set_no_frame();
	minimap_image_.reset();
}

void GameDetails::update(const SavegameData& gamedata) {
	clear();

	if (gamedata.errormessage.empty()) {
		name_label_.set_text((boost::format("<rt>%s</rt>") %
		                      as_header_with_content(_("Map:"), gamedata.mapname, style_, true))
		                        .str());

		name_label_.set_tooltip(gamedata.gametype == GameController::GameType::REPLAY ?
		                           _("The map that this replay is based on") :
		                           _("The map that this game is based on"));

		// Show game information
		std::string description =
		   as_header_with_content(_("Gametime:"), gametimestring(gamedata.gametime), style_);

		description =
		   (boost::format("%s%s") % description %
		    as_header_with_content(
		       _("Players:"),
		       (boost::format("%u") % static_cast<unsigned int>(gamedata.nrplayers)).str(), style_))
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
		// Delete former image
		minimap_icon_.set_icon(nullptr);
		minimap_icon_.set_visible(false);
		minimap_icon_.set_no_frame();
		minimap_image_.reset();
		// Load the new one
		if (!minimap_path.empty()) {
			try {
				// Load the image
				minimap_image_ = load_image(
				   minimap_path,
				   std::unique_ptr<FileSystem>(g_fs->make_sub_file_system(gamedata.filename)).get());

				int available_width = get_w() - 4 * padding_;
				int available_height = get_h() - name_label_.get_h() - descr_.get_h() - 4 * padding_;

				// Scale it
				double scale = double(available_width) / minimap_image_->width();
				double scaleY = double(available_height) / minimap_image_->height();
				if (scaleY < scale) {
					scale = scaleY;
				}
				if (scale > 1.0)
					scale = 1.0;  // Don't make the image too big; fuzziness will result
				uint16_t w = scale * minimap_image_->width();
				uint16_t h = scale * minimap_image_->height();

				// Center the minimap in the available space
				int32_t xpos = (get_w() - w) / 2;
				int32_t ypos = name_label_.get_h() + descr_.get_h() + 2 * padding_;

				// Set small minimaps higher up for a more harmonious look
				if (h < available_height * 2 / 3) {
					ypos += (available_height - h) / 3;
				} else {
					ypos += (available_height - h) / 2;
				}

				minimap_icon_.set_size(w, h);
				minimap_icon_.set_pos(Point(xpos, ypos));
				minimap_icon_.set_frame(UI_FONT_CLR_FG);
				minimap_icon_.set_visible(true);
				minimap_icon_.set_icon(minimap_image_.get());
			} catch (const std::exception& e) {
				log("Failed to load the minimap image : %s\n", e.what());
			}
		}
	} else {
		name_label_.set_text(
		   (boost::format("<rt>%s</rt>") %
		    as_header_with_content(_("Error:"), gamedata.errormessage, style_, true, true))
		      .str());
	}
}
