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

#ifndef WL_WUI_MAP_TAGS_H
#define WL_WUI_MAP_TAGS_H

#include <string>

/// Functions for localizing the known map tags.

/// Returns true if this tag is known.
bool tag_exists(const std::string& tag);

/// If tag_exists, returns the localized tag.
/// Otherwise, returns 'tag'
const std::string localize_tag(const std::string& tag);

#endif  // end of include guard: WL_WUI_MAP_TAGS_H
