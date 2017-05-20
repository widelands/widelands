/*
 * Copyright (C) 2017 by the Widelands Development Team
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
#include "graphic/font_handler1.h"
#include "graphic/text_constants.h"
#include "ui_basic/scrollbar.h"

namespace {
// NOCOM Code duplication from MapDetails
std::string as_header(const std::string& txt, bool is_first = false) {
	return (boost::format("<p><font size=%i bold=1 shadow=1>%s%s</font></p>") % UI_FONT_SIZE_SMALL %
	        (is_first ? "" : "<vspace gap=9>") % richtext_escape(txt))
	   .str();
}
std::string as_content(const std::string& txt) {
	return (boost::format(
	           "<p><font size=%i bold=1 color=D1D1D1 shadow=1><vspace gap=2>%s</font></p>") %
	        UI_FONT_SIZE_SMALL % richtext_escape(txt))
	   .str();
}
}  // namespace

ScenarioDetails::ScenarioDetails(Panel* parent)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     name_label_(this,
                 0,
                 0,
                 UI::Scrollbar::kSize,
                 0,
                 "",
                 UI::Align::kLeft,
                 g_gr->images().get("images/ui_basic/but3.png"),
                 UI::MultilineTextarea::ScrollMode::kNoScrolling),
     descr_(this, 0, 0, UI::Scrollbar::kSize, 0, "") {
	name_label_.force_new_renderer();
	descr_.force_new_renderer();

	constexpr int kPadding = 4;
	add(&name_label_, UI::Box::Resizing::kFullSize);
	add_space(kPadding);
	add(&descr_, UI::Box::Resizing::kExpandBoth);
}

void ScenarioDetails::update(const ScenarioMapData& scenariodata) {
	name_label_.set_text(
	   (boost::format("<rt>%s%s</rt>") %
	    as_header(scenariodata.is_tutorial ? _("Tutorial:") : _("Scenario:"), true) %
	    as_content(scenariodata.name))
	      .str());

	std::string description =
	   (boost::format("%s%s") %
	    as_header(ngettext("Author:", "Authors:", scenariodata.authors.get_number())) %
	    as_content(scenariodata.authors.get_names()))
	      .str();

	description = (boost::format("%s%s") % description % as_header(_("Description:"))).str();
	description = (boost::format("%s%s") % description % as_content(scenariodata.description)).str();

	description = (boost::format("<rt>%s</rt>") % description).str();
	descr_.set_text(description);
	descr_.scroll_to_top();
}
