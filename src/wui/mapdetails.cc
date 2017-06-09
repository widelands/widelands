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
 */

#include "wui/mapdetails.h"

#include <algorithm>
#include <cstdio>
#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "map_io/widelands_map_loader.h"
#include "ui_basic/box.h"
#include "ui_basic/scrollbar.h"
#include "wui/map_tags.h"

namespace {
std::string as_header(const std::string& txt, MapDetails::Style style, bool is_first = false) {
	switch (style) {
	case MapDetails::Style::kFsMenu:
		return (boost::format("<p><font size=%i bold=1 shadow=1>%s%s</font></p>") %
		        UI_FONT_SIZE_SMALL % (is_first ? "" : "<vspace gap=9>") % richtext_escape(txt))
		   .str();
	case MapDetails::Style::kWui:
		return (boost::format("<p><font size=%i bold=1 color=D1D1D1>%s%s</font></p>") %
		        UI_FONT_SIZE_SMALL % (is_first ? "" : "<vspace gap=6>") % richtext_escape(txt))
		   .str();
	}
	NEVER_HERE();
}
std::string as_content(const std::string& txt, MapDetails::Style style) {
	switch (style) {
	case MapDetails::Style::kFsMenu:
		return (boost::format(
		           "<p><font size=%i color=D1D1D1 shadow=1><vspace gap=2>%s</font></p>") %
		        UI_FONT_SIZE_SMALL % richtext_escape(txt))
		   .str();
	case MapDetails::Style::kWui:
		return (boost::format("<p><font size=%i><vspace gap=2>%s</font></p>") %
		        (UI_FONT_SIZE_SMALL - 2) % richtext_escape(txt))
		   .str();
	}
	NEVER_HERE();
}
}  // namespace

MapDetails::MapDetails(Panel* parent, int32_t x, int32_t y, int32_t w, int32_t h, Style style)
   : UI::Panel(parent, x, y, w, h),

     style_(style),
     padding_(4),
     main_box_(this, 0, 0, UI::Box::Vertical, 0, 0, 0),
     name_label_(&main_box_,
                 0,
                 0,
                 UI::Scrollbar::kSize,
                 0,
                 "",
                 UI::Align::kLeft,
                 g_gr->images().get("images/ui_fsmenu/button_menu.png"),
                 UI::MultilineTextarea::ScrollMode::kNoScrolling),
     descr_(&main_box_, 0, 0, UI::Scrollbar::kSize, 0, ""),
     suggested_teams_box_(
        new UI::SuggestedTeamsBox(this, 0, 0, UI::Box::Vertical, padding_, 0, w)) {
	name_label_.force_new_renderer();
	descr_.force_new_renderer();

	main_box_.add(&name_label_);
	main_box_.add_space(padding_);
	main_box_.add(&descr_);
	layout();
}

void MapDetails::clear() {
	name_label_.set_text("");
	descr_.set_text("");
	suggested_teams_box_->hide();
}

void MapDetails::layout() {
	name_label_.set_size(
	   get_w() - padding_,
	   UI::g_fh1->render(as_uifont(UI::g_fh1->fontset()->representative_character()))->height() + 2);

	// Adjust sizes for show / hide suggested teams
	if (suggested_teams_box_->is_visible()) {
		suggested_teams_box_->set_pos(Vector2i(0, get_h() - suggested_teams_box_->get_h()));
		main_box_.set_size(get_w(), get_h() - suggested_teams_box_->get_h() - padding_);
	} else {
		main_box_.set_size(get_w(), get_h());
	}
	descr_.set_size(main_box_.get_w(), main_box_.get_h() - name_label_.get_h() - padding_);
	descr_.scroll_to_top();
}

void MapDetails::update(const MapData& mapdata, bool localize_mapname) {
	clear();
	// Show directory information
	if (mapdata.maptype == MapData::MapType::kDirectory) {
		name_label_.set_text((boost::format("<rt>%s%s</rt>") %
		                      as_header(_("Directory:"), style_, true) %
		                      as_content(mapdata.localized_name, style_))
		                        .str());
		main_box_.set_size(main_box_.get_w(), get_h());

	} else {  // Show map information
		name_label_.set_text(
		   (boost::format("<rt>%s%s</rt>") %
		    as_header(mapdata.maptype == MapData::MapType::kScenario ? _("Scenario:") : _("Map:"),
		              style_, true) %
		    as_content(localize_mapname ? mapdata.localized_name : mapdata.name, style_))
		      .str());

		if (mapdata.localized_name != mapdata.name) {
			if (localize_mapname) {
				name_label_.set_tooltip
				   /** TRANSLATORS: Tooltip in map description when translated map names are being
				      displayed. */
				   /** TRANSLATORS: %s is the English name of the map. */
				   ((boost::format(_("The original name of this map: %s")) % mapdata.name).str());
			} else {
				name_label_.set_tooltip
				   /** TRANSLATORS: Tooltip in map description when map names are being displayed in
				      English. */
				   /** TRANSLATORS: %s is the localized name of the map. */
				   ((boost::format(_("The name of this map in your language: %s")) %
				     mapdata.localized_name)
				       .str());
			}
		}

		// Show map information
		std::string description =
		   as_header(ngettext("Author:", "Authors:", mapdata.authors.get_number()), style_);
		description =
		   (boost::format("%s%s") % description % as_content(mapdata.authors.get_names(), style_))
		      .str();

		std::vector<std::string> tags;
		for (const auto& tag : mapdata.tags) {
			tags.push_back(localize_tag(tag));
		}
		std::sort(tags.begin(), tags.end());
		description = (boost::format("%s%s") % description % as_header(_("Tags:"), style_)).str();
		description = (boost::format("%s%s") % description %
		               as_content(i18n::localize_list(tags, i18n::ConcatenateWith::COMMA), style_))
		                 .str();

		description =
		   (boost::format("%s%s") % description % as_header(_("Description:"), style_)).str();
		description =
		   (boost::format("%s%s") % description % as_content(mapdata.description, style_)).str();

		if (!mapdata.hint.empty()) {
			/** TRANSLATORS: Map hint header when selecting a map. */
			description = (boost::format("%s%s") % description % as_header(_("Hint:"), style_)).str();
			description =
			   (boost::format("%s%s") % description % as_content(mapdata.hint, style_)).str();
		}

		description = (boost::format("<rt>%s</rt>") % description).str();
		descr_.set_text(description);

		// Show / hide suggested teams
		if (mapdata.suggested_teams.empty()) {
			suggested_teams_box_->hide();
		} else {
			suggested_teams_box_->show(mapdata.suggested_teams);
		}
	}
	layout();
}
