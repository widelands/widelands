/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/minimap_renderer.h"
#include "graphic/text_layout.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_settings.h"
#include "map_io/map_loader.h"
#include "ui_basic/box.h"
#include "ui_basic/scrollbar.h"
#include "wui/map_tags.h"

MapDetails::MapDetails(
   Panel* parent, int32_t x, int32_t y, int32_t w, int32_t h, UI::PanelStyle style)
   : UI::Panel(parent, x, y, w, h),

     style_(style),
     padding_(4),
     main_box_(this, 0, 0, UI::Box::Vertical, 0, 0, 0),
     descr_box_(&main_box_, 0, 0, UI::Box::Vertical, 0, 0, 0),
     name_(""),
     name_label_(&main_box_,
                 0,
                 0,
                 UI::Scrollbar::kSize,
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
     minimap_icon_(&descr_box_, 0, 0, 0, 0, nullptr),
     suggested_teams_box_(new UI::SuggestedTeamsBox(this, 0, 0, UI::Box::Vertical, padding_, 0)),
     egbase_(nullptr) {

	minimap_icon_.set_frame(g_style_manager->minimap_icon_frame());
	descr_.set_handle_mouse(false);
	descr_box_.set_force_scrolling(true);

	descr_box_.add(&descr_);
	descr_box_.add_space(padding_);
	descr_box_.add(&minimap_icon_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	main_box_.add(&name_label_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(padding_);
	main_box_.add(&descr_box_);

	layout();
}

void MapDetails::clear() {
	name_label_.set_text("");
	descr_.set_text("");
	minimap_icon_.set_icon(nullptr);
	minimap_icon_.set_visible(false);
	minimap_icon_.set_size(0, 0);
	minimap_image_.reset();
	suggested_teams_box_->hide();
}

void MapDetails::layout() {
	// Adjust sizes for show / hide suggested teams
	if (suggested_teams_box_->is_visible()) {
		suggested_teams_box_->set_pos(Vector2i(0, get_h() - suggested_teams_box_->get_h()));
		main_box_.set_size(get_w(), get_h() - suggested_teams_box_->get_h() - padding_);
	} else {
		main_box_.set_size(get_w(), get_h());
	}

	if (minimap_icon_.icon() == nullptr) {
		minimap_icon_.set_desired_size(0, 0);
	} else {
		// Fit minimap to width
		const int width = std::min<int>(
		   main_box_.get_w() - UI::Scrollbar::kSize - 2 * padding_, minimap_image_->width());
		const float scale = static_cast<float>(width) / minimap_image_->width();
		const int height = scale * minimap_image_->height();

		minimap_icon_.set_desired_size(width, height);
	}

	const int descr_box_height = main_box_.get_h() - name_label_.get_h() - padding_;
	descr_.set_desired_size(main_box_.get_w() - UI::Scrollbar::kSize, descr_box_height);
	descr_box_.set_size(main_box_.get_w(), descr_box_height);
}

void MapDetails::update(const MapData& mapdata, bool localize_mapname) {
	clear();
	name_ = mapdata.name;
	// Show directory information
	if (mapdata.maptype == MapData::MapType::kDirectory) {
		name_label_.set_text(
		   (boost::format("<rt>%s%s</rt>") % as_heading(_("Directory"), style_, true) %
		    as_content(mapdata.localized_name, style_))
		      .str());
		main_box_.set_size(main_box_.get_w(), get_h());

	} else {  // Show map information
		name_label_.set_text(
		   (boost::format("<rt>%s%s</rt>") %
		    as_heading(mapdata.maptype == MapData::MapType::kScenario ? _("Scenario") : _("Map"),
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
		} else {
			name_label_.set_tooltip("");
		}

		// Show map information
		const std::string authors_heading =
		   (mapdata.authors.get_number() == 1) ?
		      /** TRANSLATORS: Label in map details if there is 1 author */
		      _("Author") :
		      /** TRANSLATORS: Label in map details if there is more than 1 author. If you need plural
		         forms here, please let us know. */
		      _("Authors");
		std::string description = as_heading(authors_heading, style_);
		description =
		   (boost::format("%s%s") % description % as_content(mapdata.authors.get_names(), style_))
		      .str();

		std::vector<std::string> tags;
		for (const auto& tag : mapdata.tags) {
			tags.push_back(localize_tag(tag));
		}
		std::sort(tags.begin(), tags.end());
		description = (boost::format("%s%s") % description % as_heading(_("Tags"), style_)).str();
		description = (boost::format("%s%s") % description %
		               as_content(i18n::localize_list(tags, i18n::ConcatenateWith::COMMA), style_))
		                 .str();

		description =
		   (boost::format("%s%s") % description % as_heading(_("Description"), style_)).str();
		description =
		   (boost::format("%s%s") % description % as_content(mapdata.description, style_)).str();

		if (!mapdata.hint.empty()) {
			/** TRANSLATORS: Map hint header when selecting a map. */
			description = (boost::format("%s%s") % description % as_heading(_("Hint"), style_)).str();
			description =
			   (boost::format("%s%s") % description % as_content(mapdata.hint, style_)).str();
		}

		descr_.set_text(as_richtext(description));

		// Render minimap
		egbase_.cleanup_for_load();
		map_loader_ = egbase_.mutable_map()->get_correct_loader(mapdata.filename);
		if (map_loader_ && !map_loader_->load_map_for_render(egbase_)) {
			minimap_image_ = draw_minimap(
			   egbase_, nullptr, Rectf(), MiniMapType::kStaticMap,
			   MiniMapLayer::Terrain | MiniMapLayer::StartingPositions | MiniMapLayer::Owner);
			minimap_icon_.set_icon(minimap_image_.get());
			minimap_icon_.set_visible(true);
		}

		// Show / hide suggested teams
		if (mapdata.suggested_teams.empty()) {
			suggested_teams_box_->hide();
		} else {
			suggested_teams_box_->set_size(get_parent()->get_w(), 0);
			suggested_teams_box_->show(mapdata.suggested_teams);
		}
	}
	layout();
}
