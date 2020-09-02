/*
 * Copyright (C) 2017-2020 by the Widelands Development Team
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

#include "ui_fsmenu/scenariodetails.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/minimap_renderer.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/filesystem_constants.h"
#include "ui_basic/scrollbar.h"

ScenarioDetails::ScenarioDetails(Panel* parent)
   : UI::Panel(parent, 0, 0, 0, 0),
     padding_(4),
     main_box_(this, 0, 0, UI::Box::Vertical),
     descr_box_(&main_box_, 0, 0, UI::Box::Vertical, 0, 0, 0),
     name_label_(&main_box_,
                 0,
                 0,
                 UI::Scrollbar::kSize,
                 0,
                 UI::PanelStyle::kFsMenu,
                 "",
                 UI::Align::kLeft,
                 UI::MultilineTextarea::ScrollMode::kNoScrolling),
     descr_(&descr_box_,
            0,
            0,
            UI::Scrollbar::kSize,
            0,
            UI::PanelStyle::kFsMenu,
            "",
            UI::Align::kLeft,
            UI::MultilineTextarea::ScrollMode::kNoScrolling),
     minimap_icon_(&descr_box_, 0, 0, 0, 0, nullptr),
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
}

void ScenarioDetails::layout() {
	main_box_.set_size(get_w(), get_h());

	if (minimap_icon_.icon() == nullptr) {
		minimap_icon_.set_desired_size(0, 0);
	} else {
		// Fit minimap to width
		const int width = std::min<int>(main_box_.get_w() - UI::Scrollbar::kSize - 2 * padding_, minimap_image_->width());
		const float scale = static_cast<float>(width) / minimap_image_->width();
		const int height = scale * minimap_image_->height();

		minimap_icon_.set_desired_size(width, height);
	}

	const int descr_box_height = main_box_.get_h() - name_label_.get_h() - padding_;
	descr_.set_desired_size(main_box_.get_w() - UI::Scrollbar::kSize, descr_box_height);
	descr_box_.set_size(main_box_.get_w(), descr_box_height);
}

void ScenarioDetails::update(const ScenarioData& scenariodata) {
	name_label_.set_text((boost::format("<rt>%s%s</rt>") %
	                      as_heading(scenariodata.is_tutorial ? _("Tutorial") : _("Scenario"),
	                                 UI::PanelStyle::kFsMenu, true) %
	                      as_content(scenariodata.descname, UI::PanelStyle::kFsMenu))
	                        .str());

	if (scenariodata.playable) {
		const std::string authors_heading =
		   (scenariodata.authors.get_number() == 1) ?
		      /** TRANSLATORS: Label in campaign scenario details if there is 1 author */
		      _("Author") :
		      /** TRANSLATORS: Label in campaign scenario details if there is more than 1 author. If
		         you need plural forms here, please let us know. */
		      _("Authors");
		std::string description =
		   (boost::format("%s%s") % as_heading(authors_heading, UI::PanelStyle::kFsMenu) %
		    as_content(scenariodata.authors.get_names(), UI::PanelStyle::kFsMenu))
		      .str();

		description = (boost::format("%s%s") % description %
		               as_heading(_("Description"), UI::PanelStyle::kFsMenu))
		                 .str();
		description = (boost::format("%s%s") % description %
		               as_content(scenariodata.description, UI::PanelStyle::kFsMenu))
		                 .str();

		description = (boost::format("<rt>%s</rt>") % description).str();
		descr_.set_text(description);

		// Render minimap
		egbase_.cleanup_for_load();
		std::stringstream filename;
		filename << kCampaignsDir << "/" << scenariodata.path;
		map_loader_ = egbase_.mutable_map()->get_correct_loader(filename.str());
		if (map_loader_ && !map_loader_->load_map_for_render(egbase_)) {
			minimap_image_ =
			   draw_minimap(egbase_, nullptr, Rectf(), MiniMapType::kStaticMap, MiniMapLayer::Terrain);
			minimap_icon_.set_icon(minimap_image_.get());
			minimap_icon_.set_visible(true);
		}
	} else {
		descr_.set_text("");
	}
	descr_.scroll_to_top();
}
