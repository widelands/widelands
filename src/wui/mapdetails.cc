/*
 * Copyright (C) 2002, 2006-2015 by the Widelands Development Team
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

#include <cstdio>
#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "map_io/widelands_map_loader.h"
#include "ui_basic/box.h"

namespace {
std::string as_header(const std::string& txt) {
	return (boost::format("<p><font size=%i bold=1 shadow=1><vspace gap=6>%s</font></p>")
			  % UI_FONT_SIZE_SMALL
			  % richtext_escape(txt)).str();
}
std::string as_content(const std::string& txt) {
	return (boost::format("<p><font size=%i shadow=1><vspace gap=2>%s</font></p>")
			  % UI_FONT_SIZE_SMALL
			  % richtext_escape(txt)).str();
}
} // namespace

MapDetails::MapDetails
		(Panel* parent, int32_t x, int32_t y, int32_t max_w, int32_t max_h) :
	UI::Panel(parent, x, y, max_w, max_h),

	padding_(4),
	main_box_(this, 0, 0, UI::Box::Vertical, max_w, max_h, 0),
	name_label_(&main_box_, 0, 0, max_w - padding_, 0, ""),
	descr_(&main_box_, 0, 0, max_w, 20, ""),
	suggested_teams_box_(new UI::SuggestedTeamsBox(this, 0, 0, UI::Box::Vertical, padding_, 0, 20, max_w, 20))
{
	name_label_.set_fontsize(UI_FONT_SIZE_SMALL + 2);
	descr_.force_new_renderer();

	main_box_.add(&name_label_, UI::Align::kLeft);
	main_box_.add_space(padding_);
	main_box_.add(&descr_, UI::Align::kLeft);
	main_box_.set_size(max_w, max_h - name_label_.get_h() - padding_);
	set_max_height(max_h);
}


void MapDetails::clear() {
	name_label_.set_text("");
	descr_.set_text("");
	suggested_teams_box_->hide();
}

void MapDetails::set_max_height(int new_height) {
	max_h_ = new_height;
	update_layout();
}

void MapDetails::update_layout() {
	// Adjust sizes for show / hide suggested teams
	if (suggested_teams_box_->is_visible()) {
		suggested_teams_box_->set_pos(Point(0, max_h_ - suggested_teams_box_->get_h()));
		main_box_.set_size(main_box_.get_w(), max_h_ - suggested_teams_box_->get_h() - padding_);
	} else {
		main_box_.set_size(main_box_.get_w(), max_h_);
	}
	descr_.set_size(descr_.get_w(), main_box_.get_h() - name_label_.get_h() - padding_);
	descr_.scroll_to_top();
}

void MapDetails::update(const MapData& mapdata, bool localize_mapname) {
	clear();
	if (mapdata.maptype == MapData::MapType::kDirectory) {
		// Show directory information
		name_label_.set_text(_("Directory"));
		descr_.set_text((boost::format("<rt>%s%s</rt>")
							  % as_header(_("Name:"))
							  % as_content(mapdata.localized_name)).str());
		main_box_.set_size(main_box_.get_w(), max_h_);
	} else {
		// Show map information
		if (mapdata.maptype == MapData::MapType::kScenario) {
			name_label_.set_text(_("Scenario"));
		} else {
			name_label_.set_text(_("Map"));
		}
		std::string description = as_header(_("Name:"));
		description = (boost::format("%s%s")
							% description
							% as_content(localize_mapname ? mapdata.localized_name : mapdata.name)).str();

		if (mapdata.localized_name != mapdata.name) {
			if (localize_mapname) {
				descr_.set_tooltip
				/** TRANSLATORS: Tooltip in map description when translated map names are being displayed. */
				/** TRANSLATORS: %s is the English name of the map. */
						((boost::format(_("The original name of this map: %s"))
						  % mapdata.name).str());
			} else {
				descr_.set_tooltip
				/** TRANSLATORS: Tooltip in map description when map names are being displayed in English. */
				/** TRANSLATORS: %s is the localized name of the map. */
						((boost::format(_("The name of this map in your language: %s"))
						  % mapdata.localized_name).str());
			}
		}
		description = (boost::format("%s%s")
							  % description
							  % as_header(ngettext("Author:", "Authors:", mapdata.authors.get_number()))).str();
		description = (boost::format("%s%s") % description % as_content(mapdata.authors.get_names())).str();
		description = (boost::format("%s%s") % description % as_header(_("Description:"))).str();
		description = (boost::format("%s%s") % description % as_content(mapdata.description)).str();
		if (!mapdata.hint.empty()) {
			/** TRANSLATORS: Map hint header when selecting a map. */
			description = (boost::format("%s%s") % description % as_header(_("Hint:"))).str();
			description = (boost::format("%s%s") % description % as_content(mapdata.hint)).str();
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
	update_layout();
}
