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

MapDetails::MapDetails
		(Panel* parent, int32_t x, int32_t y, int32_t max_x, int32_t max_y) :
	UI::Panel(parent, x, y, max_x, max_y),

	padding_(4),
	indent_(10),
	labelh_(20),
	max_x_(max_x),
	max_y_(max_y),

	main_box_(this, 0, 0, UI::Box::Vertical,
		  max_x_, max_y_, 0),

	name_box_(&main_box_, 0, 0, UI::Box::Horizontal,
		  max_x_, 3 * labelh_ + padding_, padding_ / 2),
	name_label_(&main_box_, 0, 0, max_x_ - padding_, labelh_, ""),
	name_(&name_box_, 0, 0, max_x_ - indent_, 2 * labelh_, ""),

	author_box_(&main_box_, 0, 0, UI::Box::Horizontal,
		  max_x_, 3 * labelh_ + padding_, padding_ / 2),
	author_label_(&main_box_, 0, 0, max_x_ - padding_, labelh_, ""),
	author_(&author_box_, 0, 0, max_x_ - indent_, labelh_, ""),

	descr_box_(&main_box_, 0, 0, UI::Box::Horizontal,
		  max_x_, 6 * labelh_ + padding_, padding_ / 2),
	descr_label_(&main_box_, 0, 0, max_x_, labelh_, ""),
	descr_(&descr_box_, 0, 0, max_x_ - indent_, 5 * labelh_, "")
{
	suggested_teams_box_ = new UI::SuggestedTeamsBox(this, 0, 0, UI::Box::Vertical,
																	 padding_, indent_, labelh_, max_x_, 4 * labelh_);

	main_box_.add(&name_label_, UI::Box::AlignLeft);
	name_box_.add_space(indent_);
	name_box_.add(&name_, UI::Box::AlignLeft);
	main_box_.add(&name_box_, UI::Box::AlignLeft);
	main_box_.add_space(padding_);

	main_box_.add(&author_label_, UI::Box::AlignLeft);
	author_box_.add_space(indent_);
	author_box_.add(&author_, UI::Box::AlignLeft);
	main_box_.add(&author_box_, UI::Box::AlignLeft);
	main_box_.add_space(padding_);

	main_box_.add(&descr_label_, UI::Box::AlignLeft);
	descr_box_.add_space(indent_);
	descr_box_.add(&descr_, UI::Box::AlignLeft);
	main_box_.add(&descr_box_, UI::Box::AlignLeft);
	main_box_.add_space(padding_);
}


void MapDetails::clear() {
	name_label_.set_text("");
	author_label_.set_text("");
	descr_label_.set_text("");
	name_.set_text("");
	author_.set_text("");
	descr_.set_text("");
	suggested_teams_box_->hide();
}

void MapDetails::update(const MapData& mapdata, bool localize_mapname) {
	clear();
	if (mapdata.maptype == MapData::MapType::kDirectory) {
		// Show directory information
		name_label_.set_text(_("Directory:"));
		name_.set_text(mapdata.localized_name);
		name_.set_tooltip(_("The name of this directory"));
		main_box_.set_size(max_x_, max_y_);
	} else {
		// Show map information
		if (mapdata.maptype == MapData::MapType::kScenario) {
			name_label_.set_text(_("Scenario:"));
		} else {
			name_label_.set_text(_("Map:"));
		}
		name_.set_text(localize_mapname ? mapdata.localized_name : mapdata.name);
		if (mapdata.localized_name != mapdata.name) {
			if (localize_mapname) {
				name_.set_tooltip
				/** TRANSLATORS: Tooltip in map description when translated map names are being displayed. */
				/** TRANSLATORS: %s is the English name of the map. */
						((boost::format(_("The original name of this map: %s"))
						  % mapdata.name).str());
			} else {
				name_.set_tooltip
				/** TRANSLATORS: Tooltip in map description when map names are being displayed in English. */
				/** TRANSLATORS: %s is the localized name of the map. */
						((boost::format(_("The name of this map in your language: %s"))
						  % mapdata.localized_name).str());
			}
		} else {
			name_.set_tooltip(_("The name of this map"));
		}
		author_label_.set_text(ngettext("Author:", "Authors:", mapdata.authors.get_number()));
		author_.set_text(mapdata.authors.get_names());
		descr_label_.set_text(_("Description:"));
		descr_.set_text(mapdata.description +
										  (mapdata.hint.empty() ? "" : (std::string("\n\n") + mapdata.hint)));

		// Show / hide suggested teams
		if (mapdata.suggested_teams.empty()) {
			main_box_.set_size(max_x_, max_y_);
			descr_box_.set_size(
						descr_box_.get_w(),
						max_y_ - descr_label_.get_y() - descr_label_.get_h() - 2 * padding_);
		} else {
			suggested_teams_box_->show(mapdata.suggested_teams);
			suggested_teams_box_->set_pos(Point(0, max_y_ - suggested_teams_box_->get_h()));
			main_box_.set_size(max_x_, max_y_ - suggested_teams_box_->get_h());
			descr_box_.set_size(
						descr_box_.get_w(),
						suggested_teams_box_->get_y() - descr_label_.get_y() - descr_label_.get_h() - 4 * padding_);
		}
		descr_.set_size(descr_.get_w(), descr_box_.get_h());
		descr_.scroll_to_top();
	}
}
