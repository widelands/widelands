/*
 * Copyright (C) 2007-2017 by the Widelands Development Team
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

#ifndef WL_LOGIC_CAMPAIGN_VISIBILITY_H
#define WL_LOGIC_CAMPAIGN_VISIBILITY_H

#include <string>

#include "scripting/lua_table.h"

struct CampaignVisibilitySave {
	static void ensure_campvis_file_is_current();
	static void mark_scenario_as_solved(const std::string& path);

private:
	static void update_campvis(const LuaTable& table, bool is_legacy);
};

#endif  // end of include guard: WL_LOGIC_CAMPAIGN_VISIBILITY_H
