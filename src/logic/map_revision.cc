/*
 * Copyright (C) 2013-2022 by the Widelands Development Team
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
 *
 */

#include "logic/map_revision.h"

#include <ctime>

#include "build_info.h"

namespace Widelands {

MapVersion::MapVersion()
   : map_creator_version(build_id()),
     map_version_major(0),
     map_version_minor(0),
     map_version_timestamp(static_cast<uint32_t>(time(nullptr))) {
}
}  // namespace Widelands
