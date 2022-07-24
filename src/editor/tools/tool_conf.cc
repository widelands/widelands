/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "editor/tools/tool_conf.h"

#include "editor/editorinteractive.h"

ToolConf::ToolConf() : primary(nullptr), change_by(0), set_to(0), interval(0, 0), resource(0) {
}

std::string ToolConf::to_key() const {
	std::string mapobj_ids;

	for (std::list<Widelands::DescriptionIndex>::const_iterator p = map_obj_types.begin();
	     p != map_obj_types.end() && buf.size() < max_string_size; p++) {
		buf += format("%d ", static_cast<int>(*p));
	}

	return format("tool:%d, change_by:%d, interval:%d..%d, map_objs:%s, resource:%d, set_to:%d",
	              static_cast<int>(primary->get_window_id()), change_by,
	              static_cast<int>(interval.min), static_cast<int>(interval.max), buf,
	              static_cast<int>(resource), static_cast<int>(set_to));
}
