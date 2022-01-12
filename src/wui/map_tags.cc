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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "wui/map_tags.h"

#include <map>

#include "base/i18n.h"

namespace {

const std::map<std::string, std::string> kMapTags = {
   /** TRANSLATORS: This is a map tag */
   {"official", _("Official")},
   /** TRANSLATORS: This is a map tag */
   {"unofficial", _("Unofficial")},
   /** TRANSLATORS: This is a map tag */
   {"balanced", _("Balanced")},
   /** TRANSLATORS: This is a map tag */
   {"unbalanced", _("Unbalanced")},
   /** TRANSLATORS: This is a map tag */
   {"seafaring", _("Seafaring")},
   /** TRANSLATORS: This is a map tag */
   {"ferries", _("Ferries")},
   /** TRANSLATORS: This is a map tag */
   {"artifacts", _("Artifacts")},
   /** TRANSLATORS: This is a map tag */
   {"scenario", _("Scenario")},
   /** TRANSLATORS: This is a map tag */
   {"ffa", _("Free for all")},
   /** TRANSLATORS: This is a map tag. One versus one. */
   {"1v1", _("1v1")},
   /** TRANSLATORS: This is a map tag */
   {"2teams", _("Teams of 2")},
   /** TRANSLATORS: This is a map tag */
   {"3teams", _("Teams of 3")},
   /** TRANSLATORS: This is a map tag */
   {"4teams", _("Teams of 4")},
};

}  // namespace

bool tag_exists(const std::string& tag) {
	return kMapTags.count(tag) == 1;
}

const std::string localize_tag(const std::string& tag) {
	if (tag_exists(tag)) {
		return _((*kMapTags.find(tag)).second);
	}
	return tag;
}
