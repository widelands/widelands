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
#include "graphic/text_layout.h"
#include "ui_basic/scrollbar.h"

ScenarioDetails::ScenarioDetails(Panel* parent)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     name_label_(this,
                 0,
                 0,
                 UI::Scrollbar::kSize,
                 0,
                 UI::PanelStyle::kFsMenu,
                 "",
                 UI::Align::kLeft,
                 UI::MultilineTextarea::ScrollMode::kNoScrolling),
     descr_(this, 0, 0, UI::Scrollbar::kSize, 0, UI::PanelStyle::kFsMenu) {

	constexpr int kPadding = 4;
	add(&name_label_, UI::Box::Resizing::kFullSize);
	add_space(kPadding);
	add(&descr_, UI::Box::Resizing::kExpandBoth);
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
	} else {
		descr_.set_text("");
	}
	descr_.scroll_to_top();
}
