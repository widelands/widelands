/*
 * Copyright (C) 2016-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/map_tags.h"

#include <map>

#include "base/i18n.h"

namespace {

const std::map<std::string, TagTexts> kMapTags = {
   {"official",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Official"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("The map is provided by the official Widelands release")}},
   {"unofficial",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Unofficial"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("Custom or add-on map")}},
   {"balanced",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Balanced"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop(
        "Player starting positions were designed to provide equal conditions for all players")}},
   {"unbalanced",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Unbalanced"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("Players face different conditions or balancing is not checked")}},
   {"seafaring",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Seafaring"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("The map allows building ports and ships")}},
   {"ferries",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Ferries"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("The map allows building ferries, ferry yards and waterways")}},
   {"artifacts",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Artifacts"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("The map contains artifacts for the Artifacts win condition")}},
   {"scenario",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Scenario"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("The map contains a story line or other custom scripting")}},
   {"ffa",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Free for all"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("Every player fights against all other players")}},
   {"1v1",
    {/** TRANSLATORS: This is a map tag. One versus one. */
     gettext_noop("1v1"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("Two players fight against each other")}},
   {"2teams",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Teams of 2"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("Players form teams of two players each")}},
   {"3teams",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Teams of 3"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("Players form teams of three players each")}},
   {"4teams",
    {/** TRANSLATORS: This is a map tag */
     gettext_noop("Teams of 4"),
     /** TRANSLATORS: This is a tool tip for a map tag */
     gettext_noop("Players form teams of four players each")}},
};

}  // namespace

bool tag_exists(const std::string& tag) {
	return kMapTags.count(tag) == 1;
}

const TagTexts localize_tag(const std::string& tag) {
	if (tag_exists(tag)) {
		TagTexts rv = (*kMapTags.find(tag)).second;
		if (!rv.displayname.empty()) {
			rv.displayname = _(rv.displayname);
		}
		if (!rv.displayname.empty()) {
			rv.tooltip = _(rv.tooltip);
		}
		return rv;
	}
	return {tag, ""};
}

void add_tag_to_dropdown(UI::Dropdown<std::string>* dropdown, const std::string& tag) {
	const TagTexts l = localize_tag(tag);
	dropdown->add(l.displayname, tag, nullptr, false, l.tooltip);
}
